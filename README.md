# Area 51 (2005) Source Code

## Building PC Build

The following prerequisites are required to build the source tree for PC:

1. Visual Studio .Net 2003
2. XtremeToolkit 4100 | Install it from "xCore\3rdParty\CodeJock"
3. Xbox SDK - 5849
4. You'll need to create an environment variable called **X** and **S** that points to the important library directions of the source tree. For example, if the source tree is located at 'D:\area51-pc' the **X** environment variable should point to 'D:\area51-pc\xCore'. the **S** environment variable should point to 'D:\area51-pc\Support'

At the moment, only a few tools are functioning. Game build doesn't work. Valid build targets are: Debug Win32/Debug. Editor-Debug is half-implemented ! Retail/Relase Win32 is temporary broken.

## List of compiled tools
Name           | Compiled
---------------| ----------------------
Art2Code       | Yes
ArtistViewer   | No
AnimCompiler   | Yes
AudioEditor    | Partially/Only as lib/Check the Editor
A51SoundPKG    | Yes
BinaryString   | Partially/Only as lib/Check the Editor
BitmapEditor   | Partially/Only as lib/Check the Editor
DecalEditor    | Partially/Only as lib/Check the Editor
DecalCompiler  | Yes
DFSTool        | Yes
Editor         | Partially/Only as [relase](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor) form
EDRscDesc      | No
EffectsEditor  | Yes
ELFTool        | Yes
EventEditor    | Partially/Only as lib/Check the Editor
FontEditor     | Partially/Only as lib/Check the Editor
FontBuilder    | Yes
FxEditor       | Partially/Only as lib/Check the Editor
FXCompiler     | Yes
GameApp        | No
GeomCompiler   | Yes
LocoEditor     | No
MeshViewer     | No
PropertyEditor | No
SoundPackager  | Partially/Only as lib
StringTool     | Yes
Viewer         | No
WinControls    | No
WorldEditor    | No
XBMPTool       | Yes
XBMPViewer     | Yes
xCL            | Yes
xTool          | Yes

## Releases

[Here](https://github.com/ProjectDreamland/area51/releases/)

## Discord

Join the community Discord [here](https://discord.gg/7gGhFSjxsq)
