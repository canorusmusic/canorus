# Build environment for windows builds based on Windows docker images.
FROM mcr.microsoft.com/windows/nanoserver:1903

# cmake
ADD https://github.com/Kitware/CMake/releases/download/v3.17.2/cmake-3.17.2-win32-x86.zip c:/cmake.zip
RUN powershell.exe -NoP -NonI -Command "Expand-Archive 'c:\cmake.zip' 'c:\'"
RUN move c:\cmake-3.17.2-win32-x86 c:\cmake

# SWIG
ADD https://netix.dl.sourceforge.net/project/swig/swigwin/swigwin-4.0.1/swigwin-4.0.1.zip c:/temp/
RUN cd c:/temp && unzip swigwin-4.0.1.zip && mv swigwin-4.0.1 swigwin

# Python
ADD https://www.python.org/ftp/python/3.8.2/python-3.8.2.exe c:/temp/
RUN cd c:\temp\ && python-3.8.2.exe /quiet InstallAllUsers=1 TargetDir=c:\\python; killall Xvfb; sleep 2

# Qt with mingw
ADD http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe c:/temp/qt-installer.exe
COPY control_script.js c:/temp/control_script.js
COPY ["qtaccount.ini", "c:/users/root/Application Data/Qt/qtaccount.ini"]

# Qt5 packages to install and install path.
#RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; wine "C:/qt-installer.exe" --verbose --script "C:/control_script.js"; killall Xvfb; sleep 2

