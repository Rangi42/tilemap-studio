# Install Guide

## Windows

**To install Tilemap Studio:**

1. Download **install.bat** and **tilemapstudio.exe** from [the latest release](https://github.com/Rangi42/tilemap-studio/releases).
2. Hold down Shift, right-click **install.bat**, and click "**Run as administrator**". This will install Tilemap Studio to your account and create a shortcut on your Desktop.
3. Now you can delete the two downloaded files.
4. Double-click **Tilemap Studio** on your Desktop to open it. If you get an error that "mscvrt120.dll is missing", you need to install the [Microsoft Visual C++ 2013 Redistributable](https://www.microsoft.com/en-us/download/details.aspx?id=40784). You may need the x64 version as well as the x86 version, at least for Windows 10.

If you have Microsoft Visual Studio, you can build Tilemap Studio yourself:

1. On GitHub, click the green "**Clone or download**" button and click "**Download ZIP**". This will download **tilemap-studio-master.zip**.
2. Unzip tilemap-studio-master.zip. This will create the **tilemap-studio-master** folder.
3. Navigate to the tilemap-studio-master folder in Explorer.
4. Unzip lib/fltk-1.3.5-mod.zip. This will create the lib/**fltk-1.3.5-mod** folder.
5. Open lib/fltk-1.3.5-mod/ide/VisualC2010/fltk.sln in Visual Studio 2013. (Other versions may or may not work, I haven't tried.)
6. A dialog "Upgrade VS++ Compiler and Libraries" will open, since fltk.sln was made for Visual Studio 2008. Click OK.
7. Go to **Build → Batch Build…**, check the projects **fltk**, **fltkimages**, **fltkpng**, **fltkjpeg**, and **fltkzlib** in the Release configuration, and click the **Build** button.
8. Move the .lib files from lib/fltk-1.3.5-mod/lib to lib.
9. Open ide/tilemap-studio.sln in Visual Studio 2013.
10. If the Solution Configuration dropdown on the toolbar says Debug, set it to **Release**.
11. Go to **Build → Build Solution** or press F7 to build the project. This will create bin/Release/**tilemapstudio.exe**.
12. Hold down Shift, right-click **install.bat**, and click "**Run as administrator**". This will install Tilemap Studio to your account and create a shortcut on your Desktop.


## Linux

Run the following commands:

```bash
# Install dependencies
# (you need at least g++ 4.9 for <regex> support)
sudo apt-get install make g++ git unzip
sudo apt-get install zlib1g-dev libpng-dev libxpm-dev libx11-dev libxft-dev libxinerama-dev libfontconfig1-dev x11proto-xext-dev libxrender-dev libxfixes-dev

# Clone Tilemap Studio
git clone https://github.com/Rangi42/tilemap-studio.git
cd tilemap-studio

# Build modified FLTK 1.3.5 with the latest ABI enabled
# (even if you already have libfltk1.3-dev installed)
pushd lib
unzip fltk-1.3.5-mod.zip
cd fltk-1.3.5-mod
chmod +x configure
./configure --prefix="$PWD/.." --with-abiversion=10305
make
make install
popd

# Build Tilemap Studio
# ("export PATH" is needed if fltk-config is not already in your PATH)
export PATH="$PWD/lib/bin:$PATH"
make

# Install Tilemap Studio
# (tested on Ubuntu and Ubuntu derivatives only; it just copies bin/tilemapstudio
#  and res/app.xpm to system directories)
sudo make install
```
