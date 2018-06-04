This readme provides information to compile Canorus under Windows or Wine under
Linux. It assumes the installation of Qt5.5.0 under C:\Qt\Qt5.5.0 and canorus
under C:\canorus.

Installing prerequisites:
=========================
1) Install Qt 5.x open source SDK for Windows including the mingw compiler.
   Note: If the installation under Wine produces some warnings, click on Ignore
   button.
2) Go to C:\Qt\Qt5.5.0\Tools\mingw492_32\bin and rename mingw32-make.exe to
   make.exe:
   copy mingw32-make.exe make.exe
3) Install cmake 3.x from the official site and add cmake to path.
4) Install Nullsoft installer 3.x.
5) OPTIONAL: Install swig-3.x and python.
6) OPTIONAL: Install lilypond for windows to bundle it with canorus.

Canorus Windows builds:
=======================
1) Add gcc, g++ and make, and qmake to PATH:
   set PATH=%PATH%;C:\Qt\Qt5.5.0\Tools\mingw492_32\bin;C:\Qt\Qt5.5.0\5.5\mingw492_32\bin

2) Configure Canorus with cmake and set CMAKE_INSTALL_PREFIX to windows/canorus
   directory:
   cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release -D QT_QMAKE_EXECUTABLE=C:\Qt\Qt5.5.0\5.5\mingw492_32\bin\qmake.exe -D CMAKE_INSTALL_PREFIX=windows\canorus .

3) If you want to enable scripting support, you should append the following to the cmake command:
   -D SWIG_DIR=C:\swigwin-3.0.7 -D SWIG_EXECUTABLE=C:\swigwin-3.0.7\swig.exe -D PYTHON_LIBRARIES=C:\python25\libs -D PYTHON_LIBRARY=C:\python25\python25.dll -D PYTHON_INCLUDE_PATH=C:\python25\include

4) Compile the project with make and install it (this will install it to windows/canorus).
   make
   make install
 
5) OPTIONAL: Bundle lilypond alongside Canorus package:
   xcopy /E "c:\Program Files (x86)\LilyPond" C:\canorus\windows\canorus\LilyPond
  
6) Create .exe installation package with Nullsoft installer by using windows/setup.nsi config.
   "c:\Program files (x86)\NSIS\makensis" setup.nsi



Matevž Jekovec <matevz.jekovec@gmail.com>
Canorus development team
http://www.canorus.org
