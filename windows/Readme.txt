This readme provides information to compile Canorus under Windows or Wine under
Linux. It assumes the installation of Qt5 under C:\Qt\Qt5.14.2 and canorus
under C:\canorus.

Installing prerequisites:
=========================
1) Install Qt 5.x open source SDK for Windows including the mingw compiler.
   Note: If the installation under Wine produces some warnings, click on Ignore
   button.
2) Install cmake 3.x from the official site and enable adding cmake to path.
3) OPTIONAL: Install swigwin and python3.
4) OPTIONAL: Install Nullsoft installer 3.x.
5) OPTIONAL: Install lilypond for windows to bundle it with canorus.

Canorus Windows builds:
=======================
1) Add gcc, g++ and make, and qmake to PATH:
   set PATH=%PATH%;C:\Qt\Tools\mingw730_32\bin;C:\Qt\5.14.2\mingw73_32\bin

2) Configure Canorus with cmake and set CMAKE_INSTALL_PREFIX to windows/canorus
   directory:
   md c:\canorus\build; cd c:\canorus\build;
   c:\cmake\bin\cmake.exe -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=C:\Qt\Tools\mingw730_32\bin\mingw32-make.exe -DCMAKE_BUILD_TYPE=Release -D QT_QMAKE_EXECUTABLE=C:\Qt\5.14.2\mingw73_32\bin\qmake.exe -D CMAKE_INSTALL_PREFIX=windows\canorus ..

3) If you want to enable scripting support, append the following to the cmake command (tested with Python 3.8.2):
   -D SWIG_DIR=C:\swigwin\Lib -D SWIG_EXECUTABLE=C:\swigwin\swig.exe -D PYTHON_LIBRARIES=C:\python\libs -D PYTHON_LIBRARY=C:\python\python38.dll -D PYTHON_INCLUDE_PATH=C:\python\include

4) Compile the project with make and install it (this will install it to windows/canorus).
   mingw32-make
   mingw32-make install

5) OPTIONAL: Bundle lilypond alongside Canorus package:
   xcopy /E "c:\Program Files (x86)\LilyPond" C:\canorus\windows\canorus\LilyPond

6) OPTIONAL: Create .exe installation package with Nullsoft installer by using windows/setup.nsi config.
   "c:\Program files (x86)\NSIS\makensis" setup.nsi



Matevž Jekovec <matevz.jekovec@gmail.com>
Canorus development team
http://www.canorus.org
