# Area 51 (2005) Source Code Release

Welcome to the unofficial release of the Area 51 (2005) video game source code! This project aims to resurrect and preserve a piece of early 2000s video game history for enthusiasts, historians, and developers alike. Below is a brief overview of the source code details, its origin, and guidance on how the community can help bring this game into the modern era.

## Historical Overview of Area 51

- **Initial Release**: Area 51 was originally released on April 25, 2005, for PC, PS2, and Xbox. It has become a memorable cult classic. 
- **Air Force Sponsorship**: In a unique turn of events, the game was sponsored by the United States Air Force and released as freeware for PC.
- **Abandonware Status**: Despite the game's initial success and the novel sponsorship, it eventually fell into obscurity, becoming abandonware. The game's support and distribution were discontinued, leaving it in a state where it was difficult for fans to access or play on modern systems.

## Source Code Snapshot Details

- **Snapshot Date**: The source code is a snapshot from 2005-03-31 10:40:19, just before the game's official release.
- **Discovery**: It was found at a garage sale of a former THQ developer.
- **Contents**: This release includes the source code for the Entropy engine, game logic, and targets for PC, PS2, Xbox, and an early version for GameCube. Additionally, debug symbols for various platforms are available in the release. Assets are not included, but those can be recovered from the retail game files.

## How to Contribute

The main goal is to get the source code into a buildable state on modern systems. Whether you're interested in game development, historical preservation, or simply a fan of Area 51, here's how you can help:

1. **Building the Project**: Assistance is needed to compile and run the game on contemporary hardware. If you have experience with game development, legacy systems, or cross-platform development, your expertise is invaluable.
2. **Documentation and Research**: Insights into the original development environment, including compilers, libraries, and tools, would greatly assist the restoration effort.
3. **Debugging and Porting**: Once buildable, the project will require debugging and potentially porting to newer platforms to reach a wider audience.

## Getting Started

To contribute, please fork the repository, make your changes, and submit a pull request with your updates. For discussion, collaboration, and support, join our community on platforms like Discord or participate in GitHub Discussions for this project.


## Building

### Xbox

When building for Xbox it's recommended to use Windows XP (VM or otherwise) as some applications/tools may have compatibility issues with newer versions of Windows.

The following prerequisites are required to build the source tree for Xbox:

1. Visual Studio .Net 2003
2. Xbox SDK - any version should work but only tested with 5849
3. You'll need to create an environment variable called **X** that points to the xCore directory of the source tree. For example, if the source tree is located at 'D:\area51' the **X** environment variable should point to 'D:\area51\xCore'.

Once the prerequisites are completed you should be able to open the Xbox VS solution located at 'Apps\GameApp\A51.sln'. Valid build targets are: XBOX-Debug, XBOX-OptDebug, XBOX-QA, and XBOX-Retail.

## Releases

[Here](https://github.com/ProjectDreamland/area51/releases/)

## Discord

Join the community Discord [here](https://discord.gg/7gGhFSjxsq)
