QSanguosha-For-Saimoe
==
> Open Source Sanguosha-Hegemony Mod

## Lisense

### Code
Our Codes are under the terms of the GPLv3 (Version 3 of the GNU General Public License)

*Please read "LICENSE" for details.*

**You are free to:**

* Use the code for any purpose

* Change the code to suit your needs

* Share the code with your friends and neighbors

* Share the changes you make

---

### Material
Our Materials are under the terms of the Creative Commons
Attribution-NonCommercial-NoDerivatives 4.0 International (CC
BY-NC-ND 4.0)

*Please read "CC BY-NC-ND 4.0" for details.*

**Materials include:**

* Musics, Dubbing, General and Skill Designs, Descriptions, Translations, Documents, Images, Icons, UI designs and others.

*Please pay attention: the materials from other sources like network is not under CC BY-NC-ND 4.0.*

*That means: You should use those materials under their original license.*

**You are free to:**

Share — copy and redistribute the material in any medium or format

**Under the following terms:**

Attribution — You must give appropriate credit, provide a link to
the license, and indicate if changes were made. You may do so in
any reasonable manner, but not in any way that suggests the licensor
endorses you or your use.

NonCommercial — You may not use the material for commercial purposes.

NoDerivatives — If you remix, transform, or build upon the material,
you may not distribute the modified material.

See the CC BY-NC-ND 4.0 file for more details.

## Introduction

Sanguosha is both a popular board game and online game,
this project try to clone the Sanguosha online version.
The whole project is written in C++,
using Qt's graphics view framework as the game engine.
I've tried many other open source game engines,
such as SDL, HGE, Clanlib and others,
but many of them lack some important features.
Although Qt is an application framework instead of a game engine,
its graphics view framework is suitable for my game developing. By Moligaloo

Saimoe MOD is an extension of original Sanguosha,
with self-designed characters from Saimoe League.
We hope you will join us with your loved one.


Contact me at [Email](mailto:acgkiller@live.com). By hmqgg

## Features

1. Framework
    * Open source with Qt graphics view framework
    * Use FMOD as sound engine
    * Use Freetype in Font Rendering
    * Use Lua as AI and extension script

2. Operation experience
    * Full package (include all yoka extension package)
    * Keyboard shortcut
    * Double-click to use cards
    * Cards sorting (by card type and card suit)
    * Multilayer display when cards are more than an upperlimit

3. Extensible
    * Some MODs are available based on this game
    * Lua Packages are supported in this game

## HOW TO BUILD
**Tips: "~" stands for the folder where the repo is in.**

VS2017 (Windows)
---

1. Download the following packages:

* QT libraries for Windows ( Visual Studio 2017, 5.6.0+ )

* Swigwin binaries ( 3.0.10 + Version )

2. Open Qsanguosha.sln right under ~/builds/vs2015, change the Configuration to Release Qt5|Win32.

3. Right click project "QSanguosha", select "lrelease all .ts files in project".

4. You are now able to build the solution. When compilation succeeded, the QSanguosha.exe is in ~/Bin folder. You should move this file to ~ folder.

5. Copy 6 files from Qt libraries to ~, they are listed below:

*   Qt5Core.dll
*   Qt5Gui.dll
*   Qt5Network.dll
*   Qt5Qml.dll
*   Qt5Widgets.dll

You can install VC redist or copy the 2 dlls from VC Redist Directory to ~.

6. Double-click the QSanguosha.exe and have fun!