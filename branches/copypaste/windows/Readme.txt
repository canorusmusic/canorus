Canorus Windows builds:
=======================
1) Configure Canorus with cmake and set CMAKE_INSTALL_PREFIX to windows/canorus directory.
2) Compile the project with make and install it (this will install it to windows/canorus).
3) Install Nullsoft installer (http://nsis.sf.net)
4) Install FontName plugin (needed for font installation): http://nsis.sourceforge.net/FontName_plug-in
5) Create an executable .exe installation with Nullsoft installer by using nsis/setup.nsi configuration script.

Adapt as needed, but something like this should compile Canorus from source and create installation package:
============================================================================================================
cd c:\canorus-X.Y
c:\cmake\bin\cmake -G "MinGW Makefiles" -D QT_QMAKE_EXECUTABLE=C:\Qt\4.3.0\bin\qmake.exe -D SWIG_DIR=C:\swigwin-1.3.31 -D SWIG_EXECUTABLE=C:\swigwin-1.3.31\swig.exe -D PYTHON_LIBRARIES=C:\python25\libs -D PYTHON_LIBRARY=C:\python25\python25.dll -D PYTHON_INCLUDE_PATH=C:\python25\include -D CMAKE_INSTALL_PREFIX=windows\canorus .
c:\mingw\bin\mingw32-make install
cd c:\canorus-X.Y\windows
c:\NSIS\makensis setup.nsi

Matevž Jekovec <matevz.jekovec@gmail.com>
Canorus development team
http://www.canorus.org