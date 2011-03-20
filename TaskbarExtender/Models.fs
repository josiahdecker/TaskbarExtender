module TaskbarExtender.Models
    open System
    open System.Linq;
    open System.Text;
    open System.IO;
    open System.IO.Pipes;
    open System.Collections.Generic;
    open System.Windows
    open System.Windows.Media.Imaging
    open System.Runtime.InteropServices
    open System.Windows.Interop


    type GenericDelegate<'a, 'b> = delegate of 'a -> 'b

    type Edge =
        |TopEdge
        |BottomEdge
        |LeftEdge
        |RightEdge

        member this.NativeValue 
            with get() =
                match this with
                    |TopEdge -> NativeMethods.AppbarMessenger.TOP
                    |BottomEdge -> NativeMethods.AppbarMessenger.BOTTOM
                    |LeftEdge -> NativeMethods.AppbarMessenger.LEFT
                    |RightEdge -> NativeMethods.AppbarMessenger.RIGHT


    ///<summary>
    ///     A Model for a system window.  Lazily initializes the button that represents it on the taskbar.  Requires a function for button construction to be passed
    ///     in (alows the model to be generic).
    ///</summary>
    ///<remarks>possibly unnecessarily generic</remarks>
    type WindowModel<'buttonType> = {
        Handle: UInt64 
        mutable button: 'buttonType option 
        ButtonCtor: UInt64 -> 'buttonType
        }
        with
            ///<remarks>Members on record types are evaluated every time they are called, so this will always return the current value</remarks>
            member this.Button = 
                match this.button with
                    | Some(x) -> x
                    | None ->
                        let button = this.ButtonCtor this.Handle
                        this.button <- Some(button)
                        button
            ///<summary>Queries the system for the HICON to represent the window, if an icon cannot be found it returns a (System defined) default icon</summary>
            static member GetIconHandle windowHandle = TaskbarExtender.NativeMethods.NativeWindowUtilities.getIconHandle(windowHandle)
            static member MakeBitmapSource iconHandle = System.Windows.Interop.Imaging.CreateBitmapSourceFromHIcon(
                                                            iconHandle, 
                                                            Int32Rect.Empty, 
                                                            BitmapSizeOptions.FromEmptyOptions())
            static member FromUInt64 winPtr ctorFxn = { Handle = winPtr; button = None; ButtonCtor = ctorFxn }


    type Collections.Generic.Dictionary<'key, 'value> with
        member this.RunIfContains (key: 'key) (fxn: 'value -> 'a) (defaultValue: 'a) : 'a =
            let (success, value) = this.TryGetValue(key)
            if success then (fxn value) else defaultValue

    ///<summary>
    ///     The model for the extended taskbar.  Holds window objects and fires events when windows are added or removed.
    ///     Window objects are cached when the window is moved offscreen, and removed from the cache when the window is closed.
    ///     First generic is the type of the window model, second is the type of the object passed to the default window model ctor (can be unit or a tuple type) which
    ///     is used when the window can not be found in the cache.
    ///<param name="screenLeft">The left edge of the screen</param>
    ///<param name="screenWidth">The width of the screen</param>
    ///<param name="defObjCtor">A function used to construct the window when it is not found in the cache</param> 
    type TaskbarModel<'a, 'b>(screenLeft: double, screenWidth: double, defObjCtor: ('b -> 'a) ) =
        let screenRight = screenLeft + screenWidth

        let windows = new Dictionary<UInt64, 'a>()
        let windowCache = new Dictionary<UInt64, 'a>()
        
        let mutable focusedWindow: UInt64 = 0UL
        
        let windowAddEvent = new Event<'a>() 
        let windowRemoveEvent = new Event<'a>()
        let windowCloseEvent = new Event<'a>()
        
        [<CLIEvent>]
        member this.WindowAdded = windowAddEvent.Publish
        [<CLIEvent>]
        member this.WindowRemoved = windowRemoveEvent.Publish
        [<CLIEvent>]
        member this.WindowClosed = windowCloseEvent.Publish    

        member public this.ContainsWindow (windowHandle: UInt64): bool=
            windows.ContainsKey(windowHandle)

        member public this.ContainsPoint (x: double) =
            x >= screenLeft && x <= screenRight

        ///<remarks>
        ///     If the window representation is not found int the cache the defObjCtor member will be used to
        ///     create the representation and will be added to the cache.
        ///</remarks>
        member public this.AddWindow (paramForCtor: 'b) (windowHandle: UInt64) =
            let (success, window) = windowCache.TryGetValue(windowHandle)
            let win =
                if success then 
                    window
                else 
                    let newWin = defObjCtor paramForCtor
                    windowCache.Add(windowHandle, newWin)
                    newWin
            windows.Add(windowHandle, win)
            windowAddEvent.Trigger win

        ///<remarks>
        ///     Removes the window from the list of displayed windows, but leaves the window in the cache.  See CloseWindow to remove the
        ///     window from the cache.
        ///</remarks>
        member public this.RemoveWindow (windowHandle: UInt64) =
            let handle = windowHandle
            let (success, win) = windows.TryGetValue(handle)
            if success then
                windows.Remove(handle) |> ignore
                windowRemoveEvent.Trigger win

        ///<remarks>
        ///     Removes the window from the window cache. Calls RemoveWindow to remove the window from the display
        ///</remarks>
        member public this.CloseWindow (windowHandle: UInt64) =
            windowCache.Remove(windowHandle) |> ignore
            let (success, win) = windows.TryGetValue(windowHandle)
            if success then
                windows.Remove(windowHandle) |> ignore
                windowCloseEvent.Trigger win 

        ///<remarks>
        ///     Only meant to be called when the window is opened on the secondary screen.  If this is called AddWindow will
        ///     not also be called.
        ///</remarks>
        member public this.OpenWindow (paramForCTor: 'b) (windowHandle: UInt64) =
            this.AddWindow paramForCTor windowHandle

        member public this.ActivateWindow (windowHandle: UInt64) =
            focusedWindow <- windowHandle

        member public this.DeactivateWindow (windowHandle: UInt64) =
            if focusedWindow = windowHandle then 
                focusedWindow <- 0UL

    type SystemTaskbarModel() =
        let systemTaskbar = new TaskbarExtender.NativeMethods.SystemTaskbarCommunicator()

        member this.RemoveTabFromSystemBar (window: UInt64) = systemTaskbar.RemoveTabFromBar(nativeint <| UIntPtr(window))
        member this.AddTabToSystemBar (window: UInt64) = systemTaskbar.AddTabToBar(nativeint <| UIntPtr(window))

        interface IDisposable with
            member this.Dispose () = systemTaskbar.Dispose() 
      
    //if either Y or Height is zero then Y and Height will be copied from the system taskbar  
    type TaskbarParameters = {
        X: double
        Y: double
        Width: double
        Height: double
        IconHeightPercentage: double
        IconLeftOffset: double
        IconMargin: double
        Edge: Edge
        }
        
    let CreateTaskbarParameters x y w h ihp ilo im eg = {X = x; Y=y; Width=w;  Height = h; IconHeightPercentage=ihp; IconLeftOffset=ilo; IconMargin=im; Edge = eg }
    
    [<Interface>]
    type UIThreadFxnCalls =
        abstract member RunOnUIThread<'a> : (unit -> 'a) -> 'a
        abstract member Refresh: unit -> unit 

    //registers an apbar with the operating system, which reserves space at the bottom of the window, inherits IDisposable to unregister the appbar
    //when the program exits
    type AppbarMessengerModel(window: System.Windows.Window, uiFxns: UIThreadFxnCalls, messageString : string) =
        let windowInteropHelper = new WindowInteropHelper(window)
        let appbarMessenger = new NativeMethods.AppbarMessenger(windowInteropHelper.Handle, messageString)

        //when the AppbarMessenger is created it registers a unique ID that the system uses to send fullscreen toggle messages
        let messageID = appbarMessenger.GetMessageID() |> int32

        //we want to stay visible even when another app is fullscreen, so we ignore the signal
        let fullscreenAppClosing = fun () -> ()
        let fullscreenAppOpening = fun () -> ()

        let fullScreenToggle (lParam: IntPtr) =
            match lParam with
                |_ when lParam = IntPtr.Zero -> fullscreenAppClosing ()
                |_ -> fullscreenAppOpening ()

        let windowMessagingHook = 
            new HwndSourceHook(
                fun hwnd msg wParam lParam handled ->
                    match msg with
                        | _ when msg = messageID -> fullScreenToggle lParam
                        | _ -> () //printf "got message with value: %d" msg
                    handled <- false
                    IntPtr.Zero
                    )

        let hwndSource = HwndSource.FromHwnd windowInteropHelper.Handle

        let setWindowPosition (rect: Rect) (edge: Edge) =
            let adjusted = appbarMessenger.QueryPosition(rect, edge.NativeValue)
            let resulted = appbarMessenger.SetPosition(adjusted, edge.NativeValue)
            window.Top <- resulted.Top
            window.Left <- resulted.Left
            window.Width <- resulted.Width
            window.Height <- resulted.Height

        let registerTaskbar (positionParam: Rect option) (edge: Edge) =
            let position = 
                match positionParam with
                    |Some(x) -> x
                    |None -> 
                        let systemTaskbarPosition = appbarMessenger.GetSystemTaskbarPosition()
                        Rect(Point(window.Left, systemTaskbarPosition.Top), Size(window.Width, systemTaskbarPosition.Height))
            //hwndSource.AddHook windowMessagingHook
            window.Activated.AddHandler (fun _ _ -> appbarMessenger.Activate())
            window.SizeChanged.AddHandler (fun _ _ -> setWindowPosition position edge)
            setWindowPosition position edge

        member this.GetSystemTaskbarPosition () = appbarMessenger.GetSystemTaskbarPosition()
        member this.RegisterAppbar(?positionParam: Rect, ?edge: Edge) = registerTaskbar positionParam (defaultArg edge BottomEdge)

        member this.SetPosition rect (edge: Edge) = setWindowPosition rect edge

        member this.ForceWindowPosition (rect: Rect) =
            window.Top <- rect.Top
            window.Left <- rect.Left
            window.Width <- rect.Width
            window.Height <- rect.Height
            uiFxns.RunOnUIThread 
                (fun () -> 
                    appbarMessenger.MoveWindow(int rect.Left, int rect.Top, int rect.Width, int rect.Height, true) 
                    |> ignore)
        

        interface IDisposable with
            member this.Dispose () = 
                hwndSource.Dispose()
                appbarMessenger.Remove()   
                    