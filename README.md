# Area 51 (2005) Source Code

Welcome to the unofficial release of the Area 51 (2005) video game source code! This project aims to resurrect and preserve a piece of early 2000s video game history for enthusiasts, historians, and developers alike. Below is a brief overview of the source code details, its origin, and guidance on how the community can help bring this game into the modern era.

## How to Contribute

The main goal is to get the source code into a buildable state on modern systems. Whether you're interested in game development, historical preservation, or simply a fan of Area 51, here's how you can help:

1. **Building the Project:** Assistance is needed to compile and run the game on contemporary hardware. If you have experience with game development, legacy systems, or cross-platform development, your expertise is invaluable.
2. **Documentation and Research:** Insights into the original development environment, including compilers, libraries, and tools, would greatly assist the restoration effort.
3. **Debugging and Porting:** Once buildable, the project will require debugging and potentially porting to newer platforms to reach a wider audience.
4. **Forking and contributing:** Make changes in your forks, and submit a pull request with your updates.

## Join our Discord

[![Join our Discord](https://github.com/gabengaGamer/area51-pc/assets/54669564/bac6c8a8-2d95-4513-8943-c5c26bd09173)](https://discord.gg/7gGhFSjxsq)

## PC Code issues

1. **Model Format Compatibility (rigidgeom, skingeom):**
The most critical issue at present is the incompatibility of the PC build with the new retail model formats. The engine does not currently support updated rigidgeom and skingeom model structures used in the latest retail version.
- Update: GEOM V41 may be partially restored — further testing required to confirm stability.

2. **Audio System Malfunctions:**
The Miles Sound System 6 implementation is broken — MP3 decoding fails, and both Bink audio playback and world ambient audio systems are non-functional.
- Update: Miles6 may be partially restored — further testing required to confirm stability.

3. **Multiplayer & Network Stack Failure:**
The network layer is completely broken. Multiplayer functionality is non-operational, and several systems tied to single-player logic are also affected.
- Update: Single-player logic may be partially restored — further testing required to confirm stability.

4. **Console UI (Incomplete Interface):**
The PC version is currently using the console UI. This results in:

  - Missing PC-specific settings (graphics, keybindings, etc.)

  - Redundant console-only interface logic included in the build, which clutters the UI codebase.

5. **Save System Not Implemented:**
The save system has not been implemented for the PC version.
- Update: Save system logic may be recreated — further testing required to confirm stability.

6. **PC Renderer Not Implemented:**
There is no functional rendering backend for the PC version — rendering code is almost not implemented. The game cannot post-process, lighting and etc. 

## Building PC Code

The following prerequisites are required to build the source tree for PC:

1. **Visual Studio .Net 2003**
2. **Xbox SDK - 5849**
3. You'll need to create an environment variable called **X** and **S** that points to the important library directions of the source tree. For example, if the source tree is located at **'D:\area51-pc'** the **X** environment variable should point to **'D:\area51-pc\xCore'**. the **S** environment variable should point to **'D:\area51-pc\Support'**
4. Put game assets to **'C:\GameData\A51\Release\PC\DVD'**
5. **XtremeToolkit 4100** **(FOR UI TOOLS)** | Install it from "xCore\3rdParty\CodeJock"

## List of valid WIN32 targets.
Debug           | OptDebug           | QA                 | Release            | EDITOR-Debug        | VIEWER-Debug 
----------------|--------------------|--------------------|--------------------|---------------------|---------------------
Yes             | Yes                | Yes                | Yes                | Yes/Only for Editor!| Yes/Only for ArtistViewer!

## List of compiled apps.
Name           | Description                                                                             | Status
---------------| ----------------------------------------------------------------------------------------|---------------
AnimCompiler   |                                                                                         | Working
Art2Code       |                                                                                         | Working
ArtistViewer   |                                                                                         | Working
AudioEditor    |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
BinaryString   |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
BitmapEditor   |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
DecalCompiler  |                                                                                         | Working
DecalEditor    |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
DFSTool        |                                                                                         | Working
EDRscDesc      |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
Editor         |                                                                                         | Working
EffectsEditor  |                                                                                         | Working
ELFTool        |                                                                                         | Working
EventEditor    |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FontBuilder    |                                                                                         | Working
FontEditor     |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FxEditor       |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FXCompiler     |                                                                                         | Working
GameApp        |                                                                                         | SemiWorking
GeomCompiler   |                                                                                         | Working
LocoEditor     |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
MeshViewer     |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
PropertyEditor |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
SoundPackager  |                                                                                         | Working
StringTool     |                                                                                         | Working
Viewer         |                                                                                         | Broken [DEPRECATED] [See ArtistViewer]
WinControls    |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
WorldEditor    |                                                                                         | Working/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
XBMPTool       |                                                                                         | Working
XBMPViewer     |                                                                                         | Working
xCL            |                                                                                         | Working
XSCC           |                                                                                         | Working
xTool          |                                                                                         | Working

## List of custom tools.
Name                                                                                         | Description                                                                             
---------------------------------------------------------------------------------------------| ----------------------------------------------------------------------------------------
[Engine-51](https://github.com/bigianb/engine-51)                                            | Area-51 Asset Viewer.                                                                                         
[Inevitable-MATX-Toolkit](https://github.com/gabengaGamer/Inevitable-MATX-Toolkit)           | Blender plugin for exporting .matx models for GeomCompiler.                                                                                      
[Json-Playsurface-Processer](https://github.com/gabengaGamer/json-playsurface-processer)     | Script for importing .playsurface maps into blender.                    
[DAT-Tool](https://github.com/gabengaGamer/DAT-Tool)                                         | Command-line interface (CLI) utility for working with Tribes: Aerial Assault DAT archive format.

## Building XBOX Code

Select the **Xbox** branch and follow the instructions.

## Historical Overview of Area 51

- **Initial Release:** Area 51 was originally released on April 25, 2005, for PC, PS2, and Xbox. It has become a memorable cult classic.
- **Air Force Sponsorship:** In a unique turn of events, the game was sponsored by the United States Air Force and released as freeware for PC.
- **Abandonware Status:** Despite the game's initial success and the novel sponsorship, it eventually fell into obscurity, becoming abandonware. The game's support and distribution were discontinued, leaving it in a state where it was difficult for fans to access or play on modern systems.

## Source Code Snapshot Details

- **Snapshot Date:** The source code is a snapshot from 2005-03-31 10:40:19, just before the game's official release.
- **Discovery:** It was found at a garage sale of a former THQ developer.
- **Contents:** This release includes the source code for the Entropy engine, game logic, and targets for PC, PS2, Xbox, and an early version for GameCube. Additionally, debug symbols for various platforms are available in the release. Assets are not included, but those can be recovered from the retail game files.
