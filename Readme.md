# ASAP - Automated Slide Analysis Platform

[![Build status](https://ci.appveyor.com/api/projects/status/gy0rv4vos88aiq53?svg=true)](https://ci.appveyor.com/project/GeertLitjens/asap)

ASAP is an open source platform for visualizing, annotating and automatically analyzing whole-slide histopathology images. It consists of several key-components (slide input/output, image processing, viewer) which can be used seperately. It is built on top of several well-developed open source packages like OpenSlide, Qt and OpenCV but also tries to extend them in several meaningful ways.

#### Features

- Reading of scanned whole-slide images from several different vendors (Aperio, Ventana, Hamamatsu, Olympus, support for fluorescence images in Leica LIF format)
- Writing of generic multi-resolution tiled TIFF files for ARGB, RGB, Indexed and monochrome images (including support for different data types like float)
- Python wrapping of the IO library for access to multi-resolution images through Numpy array's
- Basic image primitives (Patch) which can be fed to image processing filters, connection to OpenCV
- Qt-based viewer to visualize whole-slide images in a fast, fluid manner
- Point, polygonal and spline annotation tools to allow annotation of whole slide images.
- Annotation storage in simple, human-readable XML for easy use in other software
- Viewer and reading library can easily be extended by implementing plugins using one of the four interface (tools, filters, extensions, fileformats)
- Integration of on-the-fly image processing while viewing (current examples include color deconvolution and nuclei detection)

#### Installation

Currently ASAP is only supported under 64-bit Windows and Linux machines. Compilation on other architectures should be relatively straightforward as no OS-specific libraries or headers are used. The easiest way to install the software is to download the binary installer or .DEB package from the release page. **If you install the DEB package, you can find the ASAP executable under /opt/ASAP/bin**.

#### Compilation

To compile the code yourself, some prerequesites are required. First, we use CMake (version >= 3.5) as our build system and Microsoft Visual Studio or GCC as the compiler. The software depends on numerous third-party libraries:

- Boost (http://www.boost.org/)
- OpenCV (http://www.opencv.org/)
- Qt (http://www.qt.io/)
- libtiff (http://www.libtiff.org/)
- libjpeg (http://libjpeg.sourceforge.net/)
- JasPer (http://www.ece.uvic.ca/~frodo/jasper/)
- DCMTK (http://dicom.offis.de/dcmtk.php.en)
- SWIG (http://www.swig.org/) (only for Python wrapping of the IO library)
- OpenSlide (http://openslide.org/)
- PugiXML (http://pugixml.org/)
- zlib (http://www.zlib.net/)
- unittest++ (https://github.com/unittest-cpp/unittest-cpp)

To help developers compile this software themselves we provide the necesarry binaries (Visual Studio 2013, 64-bit) for all third party libraries on Windows except Boost, OpenCV and Qt (due to size constraints). See the Release page for binaries. If you want to provide the packages yourself, there are no are no strict version requirements, except for libtiff (4.0.1 and higher), Boost (1.55 or higher), Qt (5.1 or higher) and OpenCV (3.1). On Linux all packages can be installed through the package manager on Ubuntu-derived systems (tested on Ubuntu and Kubuntu 18.04 LTS). You can also use the provided Dockerfile for Linux builds (under buildtools).

Subsequently, fire up CMake, point it to a source and build directory and hit Configure. Select your compiler of preference and hit ok. This will start the iterative process of CMake trying to find a third party dependency and you specifiying its location. The first one to provide will be Boost. To allow CMake to find Boost add a BOOST\_ROOT variable pointing to for example C:/libs/boost\_1\_57\_0. Then press Configure again and CMake will ask for the next library. These should be pretty straightforward to fill in (e.g. TIFF\_LIBRRARY should point to tiff.lib, TIFF\_INCLUDE\_DIRECTORY to |folder to libtiff|\include. If more steps are unclear, please open a ticket on the Github issue-tracker.

During configuration you will notice that several parts of ASAP can be built seperately (e.g. the viewer). To build this part, simply check the component and hit Configure again. The 'Package on install'-option will allow you to build a binary setup-package like the one provided on the Github-release page. On Windows this requires NSIS to be installed.

After all the dependencies are resolved, hit Generate and CMake will create a Visual Studio Solution or makefile file which can be used to compile the source code.


## Changelog for networking branch

The frontend client of AAPI is adapted from ASAP 1.9. Major revision of code is listed below,

* HTTP Client: [HttpClient.cpp](ASAP/HttpClient.cpp) and [HttpClient.h](ASAP/HttpClient.h) are two newly
added files for creating the HTTP client;

* UI: [AnnotationWorkstationExtensionPlugin.cpp](ASAP/annotation/AnnotationWorkstationExtensionPlugin.cpp) and 
[AnnotationWorkstationExtensionPlugin.h](ASAP/annotation/AnnotationWorkstationExtensionPlugin.h) are related to the frontend
UI rendering and SIGNAL/SLOT connection. The Qt-Designer compatible ui file can be found at [AnnotationDockWidget.ui](ASAP/annotation/resources/AnnotationDockWidget.ui).

* Build tools and CMake files: In order to support local compilation of the code, several CMakeList.txt files are modified, 
    1. [CMakeLists.txt](CMakeLists.txt): change the default compilation options to enable GUI compilation;
    2. [executables/CMakeLists.txt](executables/CMakeLists.txt): Use UnitTest++ instead of UNITTEST
    3. [ASAP/annotation/CMakeLists.txt](ASAP/annotation/CMakeLists.txt): turn on CMAKE_AUTOUIC to compile the ui design file;
    4. [ASAP/CMakeLists.txt](ASAP/CMakeLists.txt): add requirements for Qt Network and Xml; link the HttpClient class;
    5. [buildtools](buildtools): modify the docker file and the script to package the release as a Ubuntu deb file.

For details, it's recommended to use the "compare" functionality on GitHub to view changes between the networking branch 
and the forked develop branch.