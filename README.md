## Building 
### Prerequisites
- [Download and install Visual Studio 2022](https://visualstudio.microsoft.com/vs/)
- [Download and install Vulkan SDK](https://vulkan.lunarg.com/)
- [Download and install CMake](https://cmake.org/download/)
  - Make sure it's added to the system path.
- For Windows Users, install [Powershell](https://learn.microsoft.com/en-us/powershell/scripting/install/installing-powershell-on-windows?view=powershell-7.3) or [Git Bash](https://gitforwindows.org/)



### Build Steps
- Create a new directory where you will work called HW3
- Open Powershell or Git Bash and navigate to your HW3 directory
- Clone the Repo ``git clone https://github.com/TheMcKillaGorilla/381_F23_HW3 .`` - this will put the project in your current directory, i.e. HW3
- Switch to the HW branch ``git switch HW``
- Pull all the dependencies into the project ``git submodule update --init --recursive``
  - If you get the following errors:
  ``fetch-pack: unexpected disconnect while reading sideband packet``
  ``fatal: early EOF``
    - Set the following flags:
        - Windows:
          - set GIT_TRACE_PACKET=1
          - set GIT_TRACE=1
          - set GIT_CURL_VERBOSE=1
        - Mac/Linux:
          - export GIT_TRACE_PACKET=1
          - export GIT_TRACE=1
          - export GIT_CURL_VERBOSE=1
      - Run the command
        - ``git config --global core.compression 0``
- Create a folder called build in your project's root directory if it isn't already there. In Terminal, or PowerShell, navigate to that directory by typing ``cd build``
- With the build folder as your present working directory (pwd) run the following commands:
  - If your team is using Visual Studio Code 2022
    - ``cmake -S .. -G "Visual Studio 17 2022"``
  - If your team is using Visual Studio Code 2019
    - ``cmake -S .. -G "Visual Studio 16 2019"``
- After this has run, you should see a file in the build folder called **Wolfie3D.sln**
- Open this file with Visual Studio. First you will need to make sure all necessary projects in the solution get built (shaders and Wolfie3D). To do this, in the VS Solution Explorer, right-click Solution Wolfie3D at the top and click Build Solution
  - If you get any errors during the build, make sure you have the latest version of the Vulkan SDK installed
- Once the build succeeds, right-click the Wolfie3D Project file within the solution (should be three project files, shaders, uninstall, and Wolfie3D), and click **Set As Startup Project** 
- At the top of Visual Studio you should see a green arrow and the text **Local Windows Debugger**, click that and the HW application should run