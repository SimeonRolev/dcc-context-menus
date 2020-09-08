### Overview of implementation

- On right click, the interface for windows context actions, overloaded in `OpenWithCtxMenuExt` gets triggered. It runs some checks and then sets up the class `App` that is defined in `Application.cpp`.
- It checks for a running process of `VCS Background Service.exe`. Detects the location of the running process and parses some paths that are later used, as well as the environment of the running DCC.
- We detect the application environment - polaris/devel etc. We later use it for assembling the directory of `active_session.json` file via Application's `env_` and `label` attributes.
- The synced folders for VCS and Dropbox are being collected from a JSON file named `active_session.json` that is being generated in the DCC's Cache folder. This file is created when the workers start with a logged in user and we know the root_folder.
- When you select files and right click on them, the code checks the selection type via `setSelectionType` method. This defines the available context actions for the selection.
- When you click a context action, `executeAction` method in the `Executor.cpp` is called with the action type and the list of files, parsed to be a compatible string for a cmd command.


### Build
- Set `Solution configurations` dropdown to `Release MinDependency`
- Set `Solution platforms` dropdown to `x64`.
- In the solution explorer right click on the `OpenWithExt` *project* (not the whole solution) -> `Clean` and then `Build`
- Result .dll should be in `x64/Release MinDependency`. You can test with registering and unregistering it.


### Debug
- Set `Solution configurations` dropdown to `Debug` from the toolbar.
- In the solution explorer right click on the `OpenWithExt` *project* (not the whole solution) -> `Rebuild`
- Close all the explorer windows, add a breakpoint in `Visual Studio`.
- `Debug -> Attach to process... -> explorer.exe`. The breakpoint will be white at this point.
- Open `explorer`. The breakpoints turn red after the attach.
- Navigate to the synced folder and right click an item. You should enter the breakpoint.


### Slow loading Microsoft Symbols:
https://stackoverflow.com/questions/12567984/visual-studio-debugging-loading-very-slow

Tools > Options > Debugging > Symbols > Microsoft Symbol Servers checkbox once. Download and cache the symbols. You can remove the checbox after. There is a field in the dialogue with a path to where to cache the downloaded symbols. `Load all modules except excluded`

### Try an action from DCC directy:
You can run a context action from terminal like this:
`python app.py "C:\\Users\\Rolev\\Vectorworks Cloud Services devel\\logo1.jpg" "UPSAMPLE"`
