# The name of our project is "Canorus".  CMakeLists files in this project can
# refer to the root source directory of the project as ${CANORUS_SOURCE_DIR}
# and to the root binary directory of the project as ${CANORUS_BINARY_DIR}.
PROJECT(Canorus)

# Assume that Debug build is only done by developers -> install in build folder
# However be aware that this only works in a separate build folder!
#SET(CMAKE_BUILD_TYPE "Debug")
IF("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
  SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
  SET(CANORUS_DEV_INSTALL True)
ELSE("${CMAKE_BUILD_TYPE}" MATCHES "Debug") # Release
  IF(${CANORUS_DEV_INSTALL} MATCHES True)
    SET(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install)
    SET(QT_NO_DEBUG_OUTPUT)
  ENDIF(${CANORUS_DEV_INSTALL} MATCHES True)
ENDIF("${CMAKE_BUILD_TYPE}" MATCHES "Debug")

# Set build directory, if none set.
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "Build directory and source directory must not be the same.\nDo something like this:\nmkdir build; cd build; cmake ..")
endif (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)

# Don't build anything unless the version of CMake is high enough.
cmake_minimum_required(VERSION 2.6)

# Disable ruby support by default
SET(NO_RUBY 1)

# Enable make uninstall rule
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

# Set MingW bin dir, if MingW present
IF(MINGW)
	STRING(REGEX REPLACE "([^ ]+)[/\\].*" "\\1" MINGW_BIN_DIR "${CMAKE_CXX_COMPILER}")
	STRING(REGEX REPLACE "\\\\" "/" MINGW_BIN_DIR "${MINGW_BIN_DIR}") # Replace back slashes to slashes
ENDIF(MINGW)

# Set Qt5 bin dir to find QtCore5.dll and other libs to install
STRING(REGEX REPLACE "([^ ]+)[/\\].*" "\\1" QT_BIN_DIR "${QT_QMAKE_EXECUTABLE}")
STRING(REGEX REPLACE "\\\\" "/" QT_BIN_DIR "${QT_BIN_DIR}")  # Replace back slashes to slashes

#-----------------------------------------------------------------------------
# Canorus version number.
FILE(READ VERSION CANORUS_VERSION)
STRING(REGEX REPLACE "\n" "" CANORUS_VERSION "${CANORUS_VERSION}") # get rid of the newline at the end

# Set default install path:
# In *nix environments this is /usr/local 
# Under M$ Windows this is C:\Program files\Canorus 
# On Mac OS X it is /Applications for the .app and /Library/Fonts for the fonts, but see note below
IF(MINGW)
	cmake_policy(SET CMP0020 NEW)
	
	IF (NOT CMAKE_INSTALL_PREFIX)
		SET(CMAKE_INSTALL_PREFIX "C:/Program files/Canorus")
	ENDIF (NOT CMAKE_INSTALL_PREFIX)
	SET(CANORUS_INSTALL_DATA_DIR "")
	SET(CANORUS_INSTALL_BIN_DIR "")
	SET(CANORUS_INSTALL_LIB_DIR "")
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

	SET(CANORUS_OSX_APP_DIR "Canorus.app/Contents")
	
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
ENDIF(APPLE)
ENDIF(MINGW)

#-----------------------------------------------------------------------------
# Search for Qt5. We need the latest version of Qt
SET(QT_MIN_VERSION "5.2.1")

# this line includes FindQt.cmake, which searches the Qt library and headers
FIND_PACKAGE(Qt5Core REQUIRED)
FIND_PACKAGE(Qt5Gui REQUIRED)
FIND_PACKAGE(Qt5Svg REQUIRED)
FIND_PACKAGE(Qt5Widgets REQUIRED)
FIND_PACKAGE(Qt5LinguistTools REQUIRED)
FIND_PACKAGE(Qt5Xml REQUIRED)
FIND_PACKAGE(Qt5Help REQUIRED)
FIND_PACKAGE(Qt5PrintSupport REQUIRED)
FIND_PACKAGE(Qt5WebEngineWidgets)

# in the following lines all the requires include directories are added
INCLUDE_DIRECTORIES(src)
INCLUDE_DIRECTORIES(src/zlib)

# Recurse into the "src" and "doc" subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(doc)

# Install README, VERSION, COPYING and other raw txt files
INSTALL(FILES AUTHORS DEVELOPERS COPYING NEWS README VERSION DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}")

