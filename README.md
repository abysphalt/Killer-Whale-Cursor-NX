# Killer Whale Cursor for Nintendo Switch

## What's this?
This is a recreation of the [famous Killer Whale Cursor flash app](http://img0.liveinternet.ru/images/attach/c/5//3970/3970473_sprite198.swf) for the Nintendo Switch, made using only Devkitpro and a couple of libraries.

### Features
* Right joy-con motion tracking + touch input to make the Killer Whale follow the cursor
* Change between front and back view of the whale
* Customize motion sensitivity, whale movement speed, and "flexibility" (how fast the individual segments follow eachother)
* Customizable whale color
* Customizable background color

### Planned features:
* Loading custom backdrop images from the microSD card
* Slightly more accurate motion tracking
* Faster load times

## Download
https://github.com/Voxel9/Killer-Whale-Cursor-NX/releases/latest

## What do I need if I want to build/contribute?
Install [Devkitpro](https://github.com/devkitPro/installer/releases/latest) and the following libraries (via pacman):

* switch-dev
* switch-freetype
* switch-bzip2
* switch-libpng
* switch-zlib
* switch-libdrm_nouveau
* switch-mesa
* switch-glm
* switch-glad

Then, clone this repository and run `make` in the root of the folder.