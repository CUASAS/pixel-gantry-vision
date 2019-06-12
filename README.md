Pixel Gantry Vision
===================

This is the C++ code that utilizes OpenCV for calculating focus and identifying fiducial marks in pixel module construction. This code is compiled to a dll and then access through the included LabVIEW adapter VIs.


Install OpenCV
==============

OpenCV is a required dependency of the gVision software. It can be installed by following these steps:

  - Download The OpenCV installer from the following url: https://sourceforge.net/projects/opencvlibrary/files/4.1.0/opencv-4.1.0-vc14_vc15.exe/download
  - Use the installer to unpack OpenCV into pixel-gantry-vision\gVisionSource.


Compiling the Code
==================

Compiling the code requires Visual Studio 2019 (or later) so install that first. Community edition is fine, but the the more full featured releases will work as well. After getting Visual Studio open up the project (`gVision.sln`) and build. If there are errors, check that the project paths are correct.


After Compilation
=================

After sucessfully compiling the code, you should have the file `gVisionSource\x64\Release\gVision.dll`. There is a LabVIEW script called `Update Binaries.vi` which is responsible for copying the necessary dlls over to the `gVisionBin` directory inside the LabVIEW part of the project. Run this VI and run the tests. If the tests work, then the `gVisionLV` library can be copied over to the `pixel-gantry-control` project for use.
