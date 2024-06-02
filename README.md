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

### For BBK/BBG
Drag the BIOS file ``bbk_bios10.rom`` into FCEUX window, and then drag a disk image, e.g. ``001.img`` into FCEUX window. You might need press any key to continue if it paused for waiting a bootable BBGDOS disk image.

### For Kewang / Bang Doctor PC Jr.
Kewang SC-3000 Video-CD computer is basically a Bang Doctor PC Jr. built-in Video-CD player, it runs SMDOS and also have some compatibility to BBK, for example, BBGDOS can boot on Kewang. To emulate a Kewang SC-3000, drag the ``Kewang_SC3000.nes`` into FCEUX window, and then drag a disk image e.g. ``1.img`` into FCEUX window.

### For YuXing
Drag one of the YuXing BIOS ``Yuxing_*.nes`` into FCEUX window, and then drag a disk image,  e.g. ``YuXing_WPS.img`` into FCEUX window. load the disk content using "Fast Disk Load" ("快速调盘" in Chinese) menu option to load software on diskette.

### For Bridge sound-picture cassette system
The "Bridge" sound-picture cassette system used to provide audio synchronized slide show with cassette player and Famiclones, it implements Chinese patent [CN2096098U](https://patents.google.com/patent/CN2096098U/en) and [CN1063867C](https://patents.google.com/patent/CN1063867C/en). The left channel used for audio, and right channel used for graphic data.

"Bridge" sound-picture system was sold as dedicated cartidge with line-in cable connected to cassette player, as well as built in function for Batong BT-686.

You can download two sample MP3 files dubbed from Bridge cassettes here: https://9game.oss-us-west-1.aliyuncs.com/bridge-picture-cassette.zip

To emulate it, Drag the ``Bridge_tape.nes`` into FCEUX window, then drag one MP3 file into FCEUX window, waiting for the audio play, and the slides will load and play automatically.

**Note: the BBK and Yuxing software is incompatible to each other**

You might also check the [Chinese Famiclone Home Computer](https://archive.org/details/ChineseFamicloneHomeComputer) bundle on Internet Archive for more software.
