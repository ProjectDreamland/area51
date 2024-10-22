# Area 51 (2005) Source Code

## Join our Discord

[![Join our Discord](https://github.com/gabengaGamer/area51-pc/assets/54669564/bac6c8a8-2d95-4513-8943-c5c26bd09173)](https://discord.gg/7gGhFSjxsq)

## Building PC Code

The following prerequisites are required to build the source tree for PC:

1. Visual Studio .Net 2003
2. XtremeToolkit 4100 | Install it from "xCore\3rdParty\CodeJock"
3. Xbox SDK - 5849
4. You'll need to create an environment variable called **X** and **S** that points to the important library directions of the source tree. For example, if the source tree is located at 'D:\area51-pc' the **X** environment variable should point to 'D:\area51-pc\xCore'. the **S** environment variable should point to 'D:\area51-pc\Support'
5. Put game assets to 'C:\GameData\A51\Release\PC\DVD'

At the moment, all tools are fixed. Game build is half loads. Valid build targets are: Debug Win32. EDITOR-Debug is for Editor only ! Release is temporary broken.

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
