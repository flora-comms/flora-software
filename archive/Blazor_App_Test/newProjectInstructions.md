# Instructions for creating a Blazor Test application

### Download and install .net sdk from microsoft website
- run dotnet --version in terminal to confirm installation

### Install C# extension for VS Code
- Down C# devkit from VScode extension marketplace

### create a new blazor webassembly project
- Open a terminal in vscode
- Navigate to the directory you want to create a project
- Run the following command, replace testApp with desired name\
dotnet new blazorwasm -o testApp

### open the project in vscode 
- open a terminal and run the following command\
dotnet run

### Open app in browser 
visit http://localhost:5087 in your browser to view the app

or run the following command which will launch the app in your browser\
dotnet watch

### Generate static files 
- run\
dotnet publish -c Release
- This will generate the static files in bin/Release/netX.X/publish/wwwroot







