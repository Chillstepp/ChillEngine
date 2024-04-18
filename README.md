# ChillEngine

## Introduction

The engine contains four basic modules.

- Editor — Game Engine Editor.
- Engine — Carrying the core logic of rendering and game architecture.
- EngineDLL — Exposed Engine API to Editor.
- EngineTest — Test Engine features like rendering, game architecture, etc.
- GameCode/GameProject(Generate by editor) — Gameplay logic.

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240331144724543.png" alt="image-20240331144724543" style="zoom:67%;" />

## Build Related

- Only support Windows now. (Cross platform is in planning).
- Engine/GameProject has four Configuration: Debug, DebugEditor, Release, ReleaseEditor:
  - DebugEditor/ReleaseEditor ouputs DLL,which will be used for editor
  - Debug/Release output exe.

# Engine Features

## Editor

- XML Serialization
- Undo/Redo Pattern
- Logger & Filtered log viewer
- Generate game Solution/Project by [EnvDTE(Visual Studio automation)](https://learn.microsoft.com/en-us/dotnet/api/envdte.dte?view=visualstudiosdk-2022) ,and Rider automation is WIP.
  - After the engine creates the project, generates the game code directory framework, generates .sln/.vcxproj, and sets the engine path environment variable.
  - Generate script code after create Script in editor.
  - Build Game Code in editor.

## Gameplay Arch

- Data-oriented
- Transform/Script component.

## Rendering Feature

### DX12 Wrapper

- `d3dx12.h` like helper function/struct.

- Use **Split Barrier** help to improve performance, especially in multi-engine scenarios or where resources are read/write transitioned sparsely throughout one or more command lists.  [Reference: Split Barriers](https://learn.microsoft.com/en-us/windows/win32/direct3d12/using-resource-barriers-to-synchronize-resource-states-in-direct3d-12#split-barriers)

- 

  
