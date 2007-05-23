# The name of our project is "Canorus".  CMakeLists files in this project can
# refer to the root source directory of the project as ${CANORUS_SOURCE_DIR}
# and to the root binary directory of the project as ${CANORUS_BINARY_DIR}.
PROJECT(Canorus)

# Don't build anything unless the version of CMake is high enough.
# The matching ELSE/ENDIF should be the last lines in the file.
# QT4 Support: Requires a very recent version (2.3.4 from March/April 2006)
IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)

# Enable make uninstall rule
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Set MingW bin dir, if MingW present
IF(MINGW)
	STRING(REGEX REPLACE "([^ ]+)[/\\].*" "\\1" MINGW_BIN_DIR_TMP "${CMAKE_CXX_COMPILER}")
	STRING(REGEX REPLACE "\\\\" "/" MINGW_BIN_DIR "${MINGW_BIN_DIR_TMP}") # Replace back slashes to slashes
ENDIF(MINGW)

# Set Qt4 bin dir to find QtCore4.dll and other libs to install
STRING(REGEX REPLACE "([^ ]+)[/\\].*" "\\1" QT_BIN_DIR_TMP "${QT_QMAKE_EXECUTABLE}")
STRING(REGEX REPLACE "\\\\" "/" QT_BIN_DIR "${QT_BIN_DIR_TMP}")  # Replace back slashes to slashes

#-----------------------------------------------------------------------------
# Canorus version number.
FILE(READ VERSION CANORUS_VERSION)
STRING(REGEX REPLACE "\n" "" CANORUS_VERSION "${CANORUS_VERSION}") # get rid of the newline at the end

# Set default install path:
# In *nix environments this is /usr/local 
# Under M$ Windows this is C:\Program files\Canorus 
# On Mac OS X it is /Applications for the .app and /Library/Fonts for the fonts, but see note below
# WARNING: Leading slash should be added - this is for easier concatenation of the strings due to slash/backslash issues
IF(MINGW)
	IF (NOT CMAKE_INSTALL_PREFIX)
		SET(CMAKE_INSTALL_PREFIX "C:/Program files/Canorus")
	ENDIF (NOT CMAKE_INSTALL_PREFIX)
	SET(CANORUS_INSTALL_DATA_DIR "")
	SET(CANORUS_INSTALL_BIN_DIR "")
	SET(CANORUS_INSTALL_LIB_DIR "")
	SET(CANORUS_INSTALL_FONTS_DIR "")

ELSE(MINGW)
IF(APPLE)
	# This is where OSX-specific resources and scripts live.
	SET(CANORUS_OSX_DIR ${CMAKE_CURRENT_BINARY_DIR}/macosx)

	# "make install" will always install to macosx/package-contents,
	# regardless of any -DCMAKE_INSTALL_PREFIX=foo
	# To install locally, build a package and use the OSX installer
	# TODO: integrate packaging script into CMake so we can just do "make install"
	# or similar and have it install and link the libraries properly
	SET(CMAKE_INSTALL_PREFIX "${CANORUS_OSX_DIR}/package-contents")

	SET(CANORUS_INSTALL_FONTS_DIR "Library/Fonts") 
	SET(CANORUS_OSX_APP_DIR	"Applications/Canorus.app/Contents")
	
	SET(CANORUS_INSTALL_BIN_DIR ${CANORUS_OSX_APP_DIR}/MacOS)
	SET(CANORUS_INSTALL_DATA_DIR ${CANORUS_OSX_APP_DIR}/Resources)
	SET(CANORUS_INSTALL_LIB_DIR ${CANORUS_OSX_APP_DIR}/Resources/lib)
ELSE(APPLE)
	IF (NOT CMAKE_INSTALL_PREFIX)
		SET(CMAKE_INSTALL_PREFIX "/usr/local")
	ENDIF (NOT CMAKE_INSTALL_PREFIX)
	SET(CANORUS_INSTALL_DATA_DIR "share/canorus")
	SET(CANORUS_INSTALL_BIN_DIR "bin")
	SET(CANORUS_INSTALL_LIB_DIR "lib")
	SET(CANORUS_INSTALL_FONTS_DIR "share/fonts/truetype/canorus")
ENDIF(APPLE)
ENDIF(MINGW)

#-----------------------------------------------------------------------------
# Search for Qt4. We need the latest version of Qt
SET(QT_MIN_VERSION "4.2.0")

# Either remove the below line and set it to the path of
# your qmake executable or modify the FindQt4.cmake program
# and add "NO_DEFAULT_PATH" to the FIND_PROGRAM(QT_QMAKE_EXECUTABLE
# instruction as last option, so the default path are not searched
# for the qmake binary (like /usr/bin/qmake on Mandriva)
#set(QT_QMAKE_EXECUTABLE "/usr/lib/qt4/bin/qmake")

# this line includes FindQt4.cmake, which searches the Qt library and headers
FIND_PACKAGE(Qt4 REQUIRED)

# in the following lines all the requires include directories are added
INCLUDE_DIRECTORIES(src)

# Recurse into the "src" and "doc" subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)

ELSE("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)
  MESSAGE(SEND_ERROR
          "This version of CMake is too old to build Canorus.  "
          "Please upgrade to CMake to at least 2.3.4")
ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)

# Install README, VERSION, LICENSE.GPL and other raw txt files
INSTALL(FILES AUTHORS DEVELOPERS LICENSE.GPL NEWS README VERSION DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}")
