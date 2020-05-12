# Build environment for windows builds based on Windows docker images.
FROM mcr.microsoft.com/windows/servercore:ltsc2019

# Qt with mingw
ADD http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe c:/qt-installer.exe
COPY control_script.js c:/control_script.js
COPY ["qtaccount.ini", "c:/users/ContainerAdministrator/AppData/Roaming/Qt/qtaccount.ini"]
# For some reason, no DNS configuration is done by default.
RUN netsh interface ip set dns name="Ethernet" source="static" address="8.8.8.8" && c:/qt-installer.exe --verbose --script c:/control_script.js

# cmake
ADD https://github.com/Kitware/CMake/releases/download/v3.17.2/cmake-3.17.2-win32-x86.zip c:/cmake.zip
RUN powershell -NoP -NonI -Command "Expand-Archive 'c:\cmake.zip' 'c:\'"
RUN move c:\cmake-3.17.2-win32-x86 c:\cmake

# SWIG
ADD https://netix.dl.sourceforge.net/project/swig/swigwin/swigwin-4.0.1/swigwin-4.0.1.zip c:/swigwin.zip
RUN powershell -NoP -NonI -Command "Expand-Archive 'c:\swigwin.zip' 'c:\'"
RUN move c:\swigwin-4.0.1 c:\swigwin

# Python
ADD https://www.python.org/ftp/python/3.8.2/python-3.8.2.exe c:/temp/python-install.exe
RUN c:\temp\python-install.exe /quiet InstallAllUsers=1 TargetDir=c:\python
