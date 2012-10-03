# TIFF Bond
w/ FoundationTO

### Requires
```
openFrameworks 0071 osx10.7
ofxCv -> master
ofxKinect -> master
ofxControlPanel (forked from Kyle's autoControlPanel)
ofxXmlSettings
```

### Setup
Install libusb1.0.0-devel. It's hard to get. 
Follow these instructions:
```
Platform Specifics

OS X

Note
AS OF 2010-11-16, WE HAVE UPDATED THIS PATCH. IF YOU HAVE ALREADY PATCHED, PLEASE REPATCH AND REINSTALL LIBUSB TO GET OS X WORKING AT FULL 30FPS.
You will need to pull the matching version of libusb for this patch. This is NOT v1.0.8, this is a change based off the repo head as of 2010-10-16. To get a tar.gz with the snapshot of the repo at this point, hit the link below.

http://git.libusb.org/?p=libusb.git;a=snapshot;h=7da756e09fd97efad2b35b5cee0e2b2550aac2cb;sf=tgz;js=1

Once you’ve gotten that tarball and unziped it somewhere, patch using the files in platform/osx/. Just go to the root directory of the libusb source and run

patch -p1 < [path_to_OpenKinectRepo]/platform/osx/libusb-osx-kinect.diff

You need to tell configure to include some necessary frameworks: ./configure LDFLAGS=-framework IOKit -framework CoreFoundation

Recompile libusb and put it wherever CMake will look (/usr/local/lib, /usr/lib, etc…). If you’re using a package manager like fink, macports, or homebrew, I’m going to expect you know what your doing and can deal with this. If not, see IRC channel.

OpenGL and GLUT come as prebuilt frameworks with OS X, so that should do it for requirements.
```