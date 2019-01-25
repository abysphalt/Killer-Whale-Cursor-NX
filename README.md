# Killer Whale Cursor for Nintendo Switch

## What's this?
This is a recreation of the [famous Killer Whale Cursor flash app](http://img0.liveinternet.ru/images/attach/c/5//3970/3970473_sprite198.swf) for the Nintendo Switch, made using only Devkitpro and a couple of libraries.

## Where can I get the latest release from?
Right here: https://github.com/Voxel9/Killer-Whale-Cursor-NX/releases/latest

## What do I need if I want to build/contribute?
Not that much. All you'll need is [Devkitpro](https://github.com/devkitPro/installer/releases/latest) and the following libraries:

* libnx
* switch-freetype
* switch-bzip2
* switch-libpng
* switch-zlib
* switch-libdrm_nouveau
* switch-mesa
* switch-glm
* switch-glad

You can simply install these using MSYS2 which you can find in your devkitpro installation folder (devkitpro/msys2/msys2_shell.bat) and running `pacman -S <LIB-TO-INSTALL>` for each listed library to install them.

After that, you can clone or download this repository, `cd` to the root of the project folder, and run `make`. Hopefully things should continue to build fine in the future. If not, give me a shout.