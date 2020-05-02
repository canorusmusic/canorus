# Build environment for windows builds based on Ubuntu and wine.
FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN dpkg --add-architecture i386 && apt update && apt install -y ca-certificates psmisc wget wine wine32 xvfb --no-install-recommends

# Setup virtual graphic environment for wine.
#RUN Xvfb :1 -screen 0 1280x960x24 &
ENV DISPLAY :1

# Run virtual desktop and some Windows app to initialize wine ~/.wine directory.
RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; wine cmd /c "exit"; killall Xvfb; sleep 2

RUN wget http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe -O ~/.wine/drive_c/qt-installer.exe

COPY control_script.js /root/.wine/drive_c/control_script.js

COPY ["qtaccount.ini", "/root/.wine/drive_c/users/root/Application Data/Qt/qtaccount.ini"]

# Qt5 packages to install and install path.
RUN Xvfb $DISPLAY -screen 0 1280x960x24 & sleep 2; wine "C:/qt-installer.exe" --verbose --script "C:/control_script.js"; killall Xvfb; sleep 2
