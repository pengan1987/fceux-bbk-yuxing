# FCEUX for BBK, Yuxing and Kewang Video-CD
This is a FCEUX fork made by Chinese enthusiast STAR, offering a certain level of BBK, Yuxing, and KeWang VCD emulation support.

## How to build
Prerequisites:
- CMake 3.19.0 (higher versions may also work but untested)
- Visual Studio 2019

1. Using CMake-GUI, set the **source** path to where this repository downloaded, and **build** path to an empty folder.
2. Click **Configure** button, set **Generator** to Visual Studio 2019 and **Platform** to Win32, you might also set the **Optional toolset** to v141_xp if you want build for Windows XP
3. Click **Finish** and CMake-GUI will highlight some options with red background, check the ``USE_TOOLS_FCeux`` box and click **Configure** button.
4. CMake-GUI will show more options with red background, change the ``Fceux_PATH`` option to the submodule folder ``fceux230`` of this repository.
5. Click **Configure** button, when CMake-GUI says **Configuring done**, click **Generate** button.
6. When CMake-GUI says **Generating done**, click **Open Project** button and open Visual Studio 2019
7. Build and run the program with Visual Studio 2019

## How to use it?
Download the starter kit with ROM and floppy disk images for BBK,Kewang and Yuxing: 
https://9game.oss-us-west-1.aliyuncs.com/FCEUXStarterKit.zip

### For BBK
Drag the BIOS file ``bbk_bios10.rom`` into FCEUX window, and then drag a disk image, e.g. ``001.img`` into FCEUX window. You might need press any key to continue if it paused when booting without a disk image.

### For Kewang / Bang Doctor PC Jr.
Kewang SC-3000 Video-CD computer is basically a Bang Doctor PC Jr. built-in Video-CD player, it runs SMDOS and also have some compatibility to BBK, for example, BBKDOS can boot on Kewang. To emulate a Kewang SC-3000, drag the ``Kewang_SC3000.nes`` into FCEUX window, and then drag a disk image e.g. ``1.img`` into FCEUX window.

### For YuXing
Drag one of the YuXing BIOS ``Yuxing_*.nes`` into FCEUX window, and then drag a disk image,  e.g. ``YuXing_WPS.img`` into FCEUX window. load the disk content using "Fast Disk Load" ("快速调盘" in Chinese) menu option to load software on diskette.

**Note: the BBK and Yuxing software is incompatible to each other**

You might also check the [Chinese Famiclone Home Computer](https://archive.org/details/ChineseFamicloneHomeComputer) bundle on Internet Archive for more software.
