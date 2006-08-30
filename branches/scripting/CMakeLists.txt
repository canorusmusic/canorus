# The name of our project is "Canorus".  CMakeLists files in this project can
# refer to the root source directory of the project as ${CANORUS_SOURCE_DIR}
# and to the root binary directory of the project as ${CANORUS_BINARY_DIR}.
project (Canorus)

# Don't build anything unless the version of CMake is high enough.
# The matching ELSE/ENDIF should be the last lines in the file.
# QT4 Support: Requires a very recent version (2.3.4 from March/April 2006)
IF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)

#-----------------------------------------------------------------------------
# Canorus version number.  An even minor number corresponds to releases.
SET(CANORUS_MAJOR_VERSION 0)
SET(CANORUS_MINOR_VERSION 1)
SET(CANORUS_BUILD_VERSION 0)
SET(CANORUS_VERSION
	"${CANORUS_MAJOR_VERSION}.${CANORUS_MINOR_VERSION}.${CANORUS_BUILD_VERSION}")

#-----------------------------------------------------------------------------
# Search for Qt4. We need the latest version of Qt
set(QT_MIN_VERSION "4.1.0")

# Either remove the below line and set it to the path of
# your qmake executable or modify the FindQt4.cmake program
# and add "NO_DEFAULT_PATH" to the FIND_PROGRAM(QT_QMAKE_EXECUTABLE
# instruction as last option, so the default path are not searched
# for the qmake binary (like /usr/bin/qmake on Mandriva)
#set(QT_QMAKE_EXECUTABLE "/usr/lib/qt4/bin/qmake")

# this line includes FindQt4.cmake, which searches the Qt library and headers
find_package(Qt4 REQUIRED)

# in the following lines all the requires include directories are added
INCLUDE_DIRECTORIES(${QT_INCLUDE_DIR})
INCLUDE_DIRECTORIES( src )

# Recurse into the "src" and "doc" subdirectories.  This does not actually
# cause another cmake executable to run.  The same process will walk through
# the project's entire directory structure.
add_subdirectory (src)
add_subdirectory (doc)

ELSE("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)
  MESSAGE(SEND_ERROR
          "This version of CMake is too old to build Canorus.  "
          "Please upgrade to CMake to at least 2.3.4")
ENDIF("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" GREATER 2.2)
