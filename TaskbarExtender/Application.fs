module TaskbarExtender.Application
    open System
    open System.Windows
    open System.Windows.Controls
    open System.Runtime.InteropServices

    open Models
    open Views
    open Controllers

    //Implementation specific types
    type WindowModelImpl = Models.WindowModel<FrameworkElement>
    type TaskbarModelImpl= Models.TaskbarModel<WindowModelImpl, UInt64>                    
    type TaskbarWindowImpl = Views.TaskbarWindow<WindowModelImpl>

    let wrapAsControllableObject (taskbarModel: TaskbarModelImpl) =
        { new Controllers.IHookServerControlledObject with
            member this.PointIsInRange x =                  taskbarModel.ContainsPoint x
            member this.TaskbarContainsWindow handle =      taskbarModel.ContainsWindow handle
            member this.AddWindowToTaskbar handle =         taskbarModel.AddWindow handle handle
            member this.RemoveWindowFromTaskbar handle =    taskbarModel.RemoveWindow handle
            member this.SendWindowClosedSignal handle =     taskbarModel.CloseWindow handle
            member this.SendWindowOpenedSignal handle =     taskbarModel.OpenWindow handle handle
            }

    //TODO - factor magical strings out to seperate file
    let serverName = "\\\\.\\mailslot\\TaskbarExtenderMailslot"
    let windowMessageIDString = "TaskbarExtenderAppbarMessage"
    let windowhookordinal = 3
    let WH_CALLWNDPROC = 4  //from WinUser.h

    let shellordinal = 1
    let WH_SHELL = 10 //from WinUser.h    

    let defaultButtonCtor (windowHandle: UInt64) =
        let img = new System.Windows.Controls.Image()
        img.BeginInit()
        img.Source <- 
            WindowModelImpl.GetIconHandle (nativeint <| UIntPtr(windowHandle))
            |> WindowModelImpl.MakeBitmapSource
        img.EndInit()
        img :> FrameworkElement

    let defaultWindowCtor (windowHandle: UInt64) = {
        Handle=                     windowHandle 
        button=                     None
        ButtonCtor=                 defaultButtonCtor 
        }

    let taskbarParams = { 
        Models.TaskbarParameters.X=1912.0
        Y = 0.0
        Height = 0.0
        Width=1920.0
        Edge= Models.Edge.BottomEdge
        IconHeightPercentage= 0.95 
        IconMargin=8.0 
        IconLeftOffset=10.0 
        }

    
    let main () =
        let app = new Application()
        
        let taskbarModel = new TaskbarModelImpl(taskbarParams.X, taskbarParams.Width, defaultWindowCtor)

        use hookServerController = new Controllers.HookServerController(serverName, (taskbarModel |> wrapAsControllableObject)); 
        use processStubController = new Controllers.ProcessStubController(windowhookordinal, WH_CALLWNDPROC)
        use shellProcStubController = new Controllers.ProcessStubController(shellordinal, WH_SHELL)
        use systemTaskbarModel = new Models.SystemTaskbarModel();

        let windowAccessorFunctions = {
            GetButton=      (fun (window: WindowModelImpl) -> window.Button)
            GetHandle=      (fun (window: WindowModelImpl) -> window.Handle)
        }
        let taskbarWindow = new TaskbarWindowImpl(taskbarParams, windowAccessorFunctions, windowMessageIDString)

        taskbarWindow.ButtonClicked
            |> Event.add (fun windowHandle -> 
                let activated = WindowController.ActivateWindow windowHandle
                if not activated then System.Diagnostics.Debugger.Log(1, "PInvoke", "window activation failed in WindowController"))
        
        taskbarModel.WindowAdded
            |> Event.add (fun windowModel ->
                taskbarWindow.AddTab windowModel
                systemTaskbarModel.RemoveTabFromSystemBar windowModel.Handle)
        taskbarModel.WindowRemoved
            |> Event.add (fun windowModel ->
                taskbarWindow.RemoveTab windowModel
                systemTaskbarModel.AddTabToSystemBar windowModel.Handle)
        taskbarModel.WindowClosed
            |> Event.add (fun windowModel ->
                taskbarWindow.RemoveTab windowModel)
            
        taskbarWindow.Show()
         
        app.Run() 
           

    [<STAThread>]
    do main() |> ignore