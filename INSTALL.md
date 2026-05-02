# Install Guide

## Windows

### Install the Tilemap Studio release

1. Download **install.bat** and **tilemapstudio.exe** from [the latest release](https://github.com/Rangi42/tilemap-studio/releases).
2. Hold down Shift, right-click **install.bat**, and click "**Run as administrator**". This will install Tilemap Studio to your account and create a shortcut on your Desktop.
3. Now you can delete the two downloaded files.
4. Double-click **Tilemap Studio** on your Desktop to open it. If you get an error that "msvcrt140.dll is missing", you need to install the [Microsoft Visual C++ Redistributable for Visual Studio 2019](https://www.visualstudio.com/downloads/).

### Build Tilemap Studio from source

You will need [Microsoft Visual Studio](https://visualstudio.microsoft.com/vs/); the Community edition is free.

If the pre-built release already works for you, you don't have to build it yourself.

1. On GitHub, click the green "**Code**" button and click "**Download ZIP**". This will download **tilemap-studio-master.zip**.
2. Unzip tilemap-studio-master.zip. This will create the **tilemap-studio-master** folder.
3. Navigate to the tilemap-studio-master folder in Explorer.
4. Download fltk-1.4.5-source.tar.bz2 or fltk-1.4.5-source.tar.gz from [**fltk.org**](https://www.fltk.org/software.php) to a new **tilemap-studio-master\lib** subfolder.
5. Extract fltk-1.4.5-source.tar (you may need a program such as [7-Zip](https://www.7-zip.org/)). This will create the lib\\**fltk-1.4.5** folder.
6. Open Visual Studio, select **Open a local folder**, and open the lib\fltk-1.4.5 folder. This will automatically generate the CMake project with a configuration named **x64-Debug** by default.
7. From the Configuration dropdown, select **Manage Configurations...**, click the green plus sign to add a new configuration, and select **x86-Release** from the list. Set the **Configuration type** to **Release** and set the **Toolset** to **msvc_x86_x64**. In the list of CMake variables, uncheck the **FLTK_GRAPHICS_GDIPLUS** option and set the **FLTK_ABI_VERSION** option to **10405**.
8. Set the active Configuration to **x86-Release**.
9. In the **Solution Explorer**, switch to the **CMake Targets View**, right-click on **fltk_images**, and select **Build fltk_images**. This will also build the other required libraries: fltk, fltk_png, and fltk_z.
10. Move all the .lib files from lib\fltk-1.4.5\out\build\x86-Release\lib\\\*.lib up to lib\\\*.lib.
11. Copy the lib\fltk-1.4.5\\**FL** folder to a new include\\**FL** folder. Also copy lib\fltk-1.4.5\out\build\x86-Release\FL\fl_config.h into include\FL.
12. Open ide\tilemap-studio.sln in Visual Studio 2019.
13. If the Solution Configuration dropdown on the toolbar says Debug, set it to **Release**.
14. Go to **Build → Build Solution** or press F7 to build the project. This will create bin\Release\\**tilemapstudio.exe**.
15. Hold down Shift, right-click **install.bat**, and click "**Run as administrator**". This will install Tilemap Studio to your account and create a shortcut on your Desktop.


## Linux

### Install dependencies

You need at least g++ 7 for C++17 support.

CMake (version 3.15 or later) is required for building FLTK 1.4.

#### Ubuntu/Debian

Run the following commands:

```bash
sudo apt install make g++ git autoconf cmake
sudo apt install zlib1g-dev libpng-dev libxpm-dev libx11-dev libxft-dev libxinerama-dev libfontconfig1-dev x11proto-xext-dev libxrender-dev libxfixes-dev libcairo2-dev libpango1.0-dev
```

#### Fedora

Run the following commands:

```bash
sudo dnf install make g++ git autoconf cmake
sudo dnf install zlib-devel libpng-devel libXpm-devel libX11-devel libXft-devel libXinerama-devel fontconfig-devel libXext-devel libXrender-devel libXfixes-devel libcairo2-devel libpango1.0-devel
```

### Install and build Tilemap Studio

Run the following commands:

```bash
# Clone Tilemap Studio
git clone https://github.com/Rangi42/tilemap-studio.git
cd tilemap-studio

# Build FLTK 1.4.5
git clone --branch release-1.4.5 --depth 1 https://github.com/fltk/fltk.git lib/fltk
pushd lib/fltk
cmake -B build \
	-D CMAKE_INSTALL_PREFIX="$(realpath "$PWD/../..")" \
	-D CMAKE_BUILD_TYPE=Release \
	-D FLTK_ABI_VERSION=10405 \
	-D FLTK_BUILD_GL=0 \
	-D FLTK_BUILD_TEST=0 \
	-D FLTK_GRAPHICS_CAIRO=1 \
	-D FLTK_BACKEND_WAYLAND=0 \
	-D FLTK_USE_SYSTEM_LIBPNG=0 \
	-D FLTK_USE_SYSTEM_ZLIB=0
cmake --build build
cmake --install build
popd

# Build Tilemap Studio
make

# Install Tilemap Studio
# (tested on Ubuntu and Ubuntu derivatives only; it just copies bin/tilemapstudio
#  and res/app.xpm to system directories)
sudo make install
```
