# Build environment for windows builds based on Ubuntu and wine.
#FROM ubuntu:20.04
FROM ubuntu:18.04

ENV DEBIAN_FRONTEND=noninteractive

# ubuntu 20.04
# issues with mount manager - drive c: is not visible, might require full desktop installation?
#RUN dpkg --add-architecture i386 && apt update && apt install -y ca-certificates psmisc wget wine wine32 xvfb

# ubuntu 18.04
# complains when 1vcredist_msvc2017_x86 is installed, connect with VNC inside container and click Ignore, otherwise works fine
RUN dpkg --add-architecture i386 && apt update && apt install -y ca-certificates psmisc wget wine-development wine32-development xvfb unzip --no-install-recommends

# winehq procedure
# hangs when quitting (any) wine command
#RUN dpkg --add-architecture i386 && apt update && apt install -y gnupg ca-certificates psmisc wget xvfb ssh x11vnc --no-install-recommends
#RUN wget -O - https://dl.winehq.org/wine-builds/winehq.key | apt-key add -
#RUN echo 'deb https://dl.winehq.org/wine-builds/ubuntu/ focal main' >> /etc/apt/sources.list && apt update
#RUN apt install -y --install-recommends winehq-devel

# Setup virtual graphic environment for wine.
#RUN Xvfb :1 -screen 0 1280x960x24 &
ENV DISPLAY :1

# Run virtual desktop and some Windows app to initialize wine ~/.wine directory.
RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; wine cmd /c "exit"; killall Xvfb; sleep 2

# cmake
ADD https://github.com/Kitware/CMake/releases/download/v3.17.2/cmake-3.17.2-win32-x86.zip /root/.wine/drive_c
RUN cd /root/.wine/drive_c && unzip cmake-3.17.2-win32-x86.zip && mv cmake-3.17.2-win32-x86 cmake

# SWIG
ADD https://netix.dl.sourceforge.net/project/swig/swigwin/swigwin-4.0.1/swigwin-4.0.1.zip /root/.wine/drive_c
RUN cd /root/.wine/drive_c && unzip swigwin-4.0.1.zip && mv swigwin-4.0.1 swigwin

# Python
ADD https://www.python.org/ftp/python/3.8.2/python-3.8.2.exe /root/.wine/drive_c
RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; cd /root/.wine/drive_c && wine python-3.8.2.exe /quiet InstallAllUsers=1 TargetDir=c:\\python; killall Xvfb; sleep 2

# Qt with mingw
ADD http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe /root/.wine/drive_c/qt-installer.exe
COPY control_script.js /root/.wine/drive_c/control_script.js
COPY ["qtaccount.ini", "/root/.wine/drive_c/users/root/Application Data/Qt/qtaccount.ini"]

# Qt5 packages to install and install path.
RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; wine "C:/qt-installer.exe" --verbose --script "C:/control_script.js"; killall Xvfb; sleep 2

