# ChillEngine

## Introduction

The engine contains four basic modules.

- Editor — Game Engine Editor.
- Engine — Carrying the core logic of rendering and game architecture.
- EngineDLL — Exposed Engine API to Editor.
- EngineTest — Test Engine features like rendering, game architecture, etc.
- GameCode/GameProject — Gameplay logic.

<img src="https://raw.githubusercontent.com/Chillstepp/MyPicBed/master/master/image-20240331144724543.png" alt="image-20240331144724543" style="zoom:67%;" />

## Editor

- XML Serialization
- Undo/Redo Pattern
- Logger & Filtered log viewer
- Generate game Solution/Project
  - After the engine creates the project, generates the game code directory framework, generates .sln/.vcxproj, and sets the engine path environment variable.

- 

## Gameplay Arch

- Data-oriented
- Transform/Script component.


## Rendering Feature

RHI: DX12
