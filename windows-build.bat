netsh interface ip set dns name="Ethernet" source="static" address="8.8.8.8"
powershell -NoP -NonI -Command "Invoke-WebRequest https://github.com/canorusmusic/canorus/archive/master.zip -UseBasicParsing -OutFile c:\canorus.zip"
powershell -NoP -NonI -Command "Expand-Archive 'c:\canorus.zip' 'c:\'"
move c:\canorus-master c:\canorus

md c:\canorus\build
cd c:\canorus\build
set PATH=%PATH%;C:\Qt\Tools\mingw730_32\bin;C:\Qt\5.14.2\mingw73_32\bin
c:\cmake\bin\cmake.exe -G "MinGW Makefiles" -DCMAKE_MAKE_PROGRAM=C:\Qt\Tools\mingw730_32\bin\mingw32-make.exe -DCMAKE_BUILD_TYPE=Release -D QT_QMAKE_EXECUTABLE=C:\Qt\5.14.2\mingw73_32\bin\qmake.exe -D CMAKE_INSTALL_PREFIX=windows\canorus -D SWIG_DIR=C:\swigwin\Lib -D SWIG_EXECUTABLE=C:\swigwin\swig.exe -D PYTHON_LIBRARIES=C:\python\libs -D PYTHON_LIBRARY=C:\python\python38.dll -D PYTHON_INCLUDE_PATH=C:\python\include ..
mingw32-make VERBOSE=1
mingw32-make install VERBOSE=1

