module TaskbarExtender.Views
    open System
    open System.Linq
    open System.Text
    open System.IO
    open System.IO.Pipes
    open System.Collections.Generic
    open System.Windows
    open System.Windows.Input
    open System.Windows.Media.Imaging
    open System.Windows.Controls


    type WindowAccessorFunctions<'a> = {
        GetButton: 'a -> FrameworkElement
        GetHandle: 'a -> UInt64
    }

    ///<summary>
    ///View model for the main taskbar window.  Generic parameter is the type of the window model.  The window model members are accessed by functions
    ///passed in through the fxns parameter, keeping this class generic Haskell style.
    ///</summary>
    ///<param name="windowParams">A Models.TaskbarParameters object that describes relevant system parameters</param>
    ///<param name="fxns">A record containing the functions used to access the window model (used in the AddTab and RemoveTab member functions)</param> 
    ///<remarks>It is important that the WindowAccessorFunctions.GetButton function, when applied to the object passed to AddTab, returns the same object when applied to the object passed to RemoveTab
    /// (i.e. for a given input to 'GetButton' the ouputs of seperate calls should have referential equality)</remarks>
    type TaskbarWindow<'a>(windowParams: Models.TaskbarParameters, fxns: WindowAccessorFunctions<'a>, appbarMsg: String) as this =
        inherit Window()

        let invokeOnUIThreadWithPriority (priority: System.Windows.Threading.DispatcherPriority) (fxn: ('c -> 'd))  : 'd =
            this.Dispatcher.Invoke(priority, new Models.GenericDelegate<'c, 'd>(fxn), ()) 
                :?> 'd

        let invokeOnUIThread = invokeOnUIThreadWithPriority System.Windows.Threading.DispatcherPriority.Normal

        let panel = new StackPanel()

        let loadHandler = new RoutedEventHandler(fun _ _ ->
            let appbarModel = new Models.AppbarMessengerModel(this, this, appbarMsg)
            let systemBarPos = appbarModel.GetSystemTaskbarPosition()
            let top, height = 
                if windowParams.Y = 0.0 || windowParams.Height = 0.0 then
                    systemBarPos.Top, systemBarPos.Height
                else
                    windowParams.Y, windowParams.Height
            let pos = Rect( Point(windowParams.X, systemBarPos.Top), 
                            Size(windowParams.Width, systemBarPos.Height))
            this.Top <- pos.Top
            this.Left <- pos.X
            this.Width <- pos.Width
            this.Height <- pos.Height
            
            panel.Orientation <- Orientation.Horizontal
            panel.Width <- pos.Width
            panel.Height <- pos.Height

            appbarModel.RegisterAppbar( positionParam = pos,
                                        edge = windowParams.Edge)
            this.LocationChanged.AddHandler(fun _ _ ->
                                                (this :> Models.UIThreadFxnCalls).Refresh()
                                                appbarModel.ForceWindowPosition pos )
            this.Closed.AddHandler(fun _ _ ->
                                    (appbarModel :> IDisposable).Dispose())
            )

        do
            //must be done on load because otherwise its difficult to get the window to go into the registered appbar space
            this.Loaded.AddHandler(loadHandler)
        
        let iconHeight() = windowParams.IconHeightPercentage * panel.Height

        do
            this.AddChild(panel)
            this.WindowStyle <- System.Windows.WindowStyle.None
            this.BorderThickness <- new Thickness(0.0)
            this.Margin <- new Thickness(0.0)
            this.AllowsTransparency <- true
            this.Background <- System.Windows.Media.Brushes.Transparent     

        let buttonClicked = new Event<_>()

        let add windowModel = 
            let button = fxns.GetButton windowModel
            let height = iconHeight()
            button.SetValue(FrameworkElement.WidthProperty, height) //same as height
            button.SetValue(FrameworkElement.HeightProperty, height)
            button.Margin <- Thickness(windowParams.IconMargin, 0.0, 0.0, 0.0)
            button.MouseDown.Add(fun eventArgs -> buttonClicked.Trigger (fxns.GetHandle windowModel) )
            panel.Children.Add(button) |> ignore

        let remove windowModel =
            let button = fxns.GetButton windowModel
            panel.Children.Remove(button)


        [<CLIEvent>]
        member this.ButtonClicked = buttonClicked.Publish

        ///<summary>
        ///calls the 'add' member function on UI thread
        ///</summary>
        member this.AddTab (window: 'a) =
            invokeOnUIThread (fun () -> add window)

        ///<summary>
        ///calls the 'remove' member function on UI thread
        ///</summary>
        member this.RemoveTab (window: 'a) =
            invokeOnUIThread (fun () -> remove window )

        member this.PositionRect with get() = Rect(Point(this.Left, this.Top), Size(this.Width, this.Height))

        interface Models.UIThreadFxnCalls with
            member this.Refresh () =
                this.InvalidateVisual()
                invokeOnUIThreadWithPriority System.Windows.Threading.DispatcherPriority.Render (fun () -> ())

            member this.RunOnUIThread<'x> (fxn: unit -> 'x) =
                invokeOnUIThreadWithPriority System.Windows.Threading.DispatcherPriority.Render fxn
