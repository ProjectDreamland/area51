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

## Building PC Code

The following prerequisites are required to build the source tree for PC:

1. **Visual Studio .Net 2003**
2. **XtremeToolkit 4100** | Install it from "xCore\3rdParty\CodeJock"
3. **Xbox SDK - 5849**
4. You'll need to create an environment variable called **X** and **S** that points to the important library directions of the source tree. For example, if the source tree is located at **'D:\area51-pc'** the **X** environment variable should point to **'D:\area51-pc\xCore'**. the **S** environment variable should point to **'D:\area51-pc\Support'**
5. Put game assets to **'C:\GameData\A51\Release\PC\DVD'**

## List of valid win32 targets.
Debug           | OptDebug      | QA            | Release      | EDITOR-Debug  
----------------|---------------|---------------|--------------|-------------
Yes             | No            | No            | No           | Yes/Only for Editor!

## List of compiled tools
Name           | Compiled
---------------| ----------------------
Art2Code       | Yes
ArtistViewer   | No
AnimCompiler   | Yes
AudioEditor    | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
A51SoundPKG    | Yes
BinaryString   | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
BitmapEditor   | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
DecalEditor    | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
DecalCompiler  | Yes
DFSTool        | Yes
Editor         | Yes
EDRscDesc      | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
EffectsEditor  | Yes
ELFTool        | Yes
EventEditor    | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FontEditor     | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FontBuilder    | Yes
FxEditor       | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
FXCompiler     | Yes
GameApp        | Yes/Retail PC models doesn't loads
GeomCompiler   | Yes
LocoEditor     | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
MeshViewer     | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
PropertyEditor | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
SoundPackager  | Yes
StringTool     | Yes
Viewer         | No
WinControls    | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
WorldEditor    | Yes/Lib for: [Editor](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor-1.0)
XBMPTool       | Yes
XBMPViewer     | Yes
xCL            | Yes
xTool          | Yes

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
