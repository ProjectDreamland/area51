# Area 51 (2005) Source Code

## Building PC Code

The following prerequisites are required to build the source tree for PC:

1. Visual Studio .Net 2003
2. XtremeToolkit 4100 | Install it from "xCore\3rdParty\CodeJock"
3. Xbox SDK - 5849
4. You'll need to create an environment variable called **X** and **S** that points to the important library directions of the source tree. For example, if the source tree is located at 'D:\area51-pc' the **X** environment variable should point to 'D:\area51-pc\xCore'. the **S** environment variable should point to 'D:\area51-pc\Support'
5. Put game assets to 'C:\GameData\A51\Release\PC\DVD'

At the moment, only a few tools are functioning. Game build is half loads. Valid build targets are: Debug Win32/Debug. Editor-Debug is half-implemented ! Retail/Relase Win32 is temporary broken.

## List of compiled tools
Name           | Compiled
---------------| ----------------------
Art2Code       | Yes
ArtistViewer   | No
AnimCompiler   | Yes
AudioEditor    | Yes/Only as lib/Check the Editor
A51SoundPKG    | Yes
BinaryString   | Yes/Only as lib/Check the Editor
BitmapEditor   | Yes/Only as lib/Check the Editor
DecalEditor    | Yes/Only as lib/Check the Editor
DecalCompiler  | Yes
DFSTool        | Yes
Editor         | Partially/Only as [relase](https://github.com/gabengaGamer/area51-pc/releases/tag/Editor) form
EDRscDesc      | Yes/Only as lib/Check the Editor
EffectsEditor  | Yes
ELFTool        | Yes
EventEditor    | Yes/Only as lib/Check the Editor
FontEditor     | Yes/Only as lib/Check the Editor
FontBuilder    | Yes
FxEditor       | Yes/Only as lib/Check the Editor
FXCompiler     | Yes
GameApp        | Yes/Game doesn't boot
GeomCompiler   | Yes
LocoEditor     | Yes/Only as lib/Check the Editor
MeshViewer     | Yes/Only as lib/Check the Editor
PropertyEditor | Yes/Only as lib/Check the Editor
SoundPackager  | Yes
StringTool     | Yes
Viewer         | No
WinControls    | Yes/Only as lib/Check the Editor
WorldEditor    | Yes/Only as lib/Check the Editor
XBMPTool       | Yes
XBMPViewer     | Yes
xCL            | Yes
xTool          | Yes

## Discord

Join the community Discord [here](https://discord.gg/7gGhFSjxsq)
