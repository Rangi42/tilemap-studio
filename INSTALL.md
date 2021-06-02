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
4. Download fltk-1.3.6-source.tar.bz2 or fltk-1.3.6-source.tar.gz from [**fltk.org**](https://www.fltk.org/software.php) to a new **tilemap-studio-master\lib** subfolder.
5. Extract fltk-1.3.6-source.tar (you may need a program such as [7-Zip](https://www.7-zip.org/)). This will create the lib\**fltk-1.3.6** folder.
6. Open lib\fltk-1.3.6\abi-version.ide in a text editor such as Notepad and replace "`#undef FL_ABI_VERSION`" with "`#define FL_ABI_VERSION 10306`". Save it.
7. Open lib\fltk-1.3.6\ide\VisualC2010\fltk.sln in Visual Studio 2019. (Other versions may or may not work, I haven't tried.)
8. A "Retarget Projects" dialog will open, since fltk.sln was made for Visual Studio 2010. Click OK to upgrade the Windows SDK version and platform toolset.
9. Go to **Build → Batch Build…**, check the projects **fltk**, **fltkimages**, **fltkpng**, **fltkjpeg**, and **fltkzlib** in the Release configuration, and click the **Build** button.
10. Move all the .lib files from lib\fltk-1.3.6\lib\\\*.lib up to lib\\\*.lib.
11. Copy the lib\FL folder up to include\FL.
12. Open ide\tilemap-studio.sln in Visual Studio 2019.
13. If the Solution Configuration dropdown on the toolbar says Debug, set it to **Release**.
14. Go to **Build → Build Solution** or press F7 to build the project. This will create bin\Release\**tilemapstudio.exe**.
15. Hold down Shift, right-click **install.bat**, and click "**Run as administrator**". This will install Tilemap Studio to your account and create a shortcut on your Desktop.


## Linux

### Install dependencies

You need at least g++ 7 for C++17 `<string_view>` support.

#### Ubuntu/Debian

Run the following commands:

```bash
sudo apt install make g++ git autoconf
sudo apt install zlib1g-dev libpng-dev libxpm-dev libx11-dev libxft-dev libxinerama-dev libfontconfig1-dev x11proto-xext-dev libxrender-dev libxfixes-dev
```

#### Fedora

Run the following commands:

```bash
sudo dnf install make g++ git autoconf
sudo dnf install zlib-devel libpng-devel libXpm-devel libX11-devel libXft-devel libXinerama-devel fontconfig-devel libXext-devel libXrender-devel libXfixes-devel
```

### Install and build Tilemap Studio

Run the following commands:

```bash
# Clone Tilemap Studio
git clone https://github.com/Rangi42/tilemap-studio.git
cd tilemap-studio

# Build FLTK 1.3.6 with the latest ABI enabled
# (even if you already have libfltk1.3-dev installed)
git clone --branch release-1.3.6 --depth 1 https://github.com/fltk/fltk.git
pushd fltk
./autogen.sh --prefix="$PWD/.." --with-abiversion=10306
make
make install
popd

# Build Tilemap Studio
# ("export PATH" is needed if bin/fltk-config is not already in your PATH)
export PATH="$PWD/bin:$PATH"
make

# Install Tilemap Studio
# (tested on Ubuntu and Ubuntu derivatives only; it just copies bin/tilemapstudio
#  and res/app.xpm to system directories)
sudo make install
```
