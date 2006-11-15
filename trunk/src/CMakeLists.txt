# Define Sources of this Canorus directory

# This module is used to resolve the file dependencies
# for QT files using an UI (User Unterface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

#######################
# Scripting detection #
#######################
# The following lines detect scripting environments (SWIG, Ruby, Python etc.)
FIND_PACKAGE(SWIG QUIET)
IF(SWIG_FOUND)
	SET(USE_SWIG True)
	MESSAGE("SWIG found, using scripting extensions!")
ENDIF(SWIG_FOUND)
	
FIND_PACKAGE(Ruby QUIET)	#Setup RUBY_INCLUDE_PATH and RUBY_LIBRARIES
IF(RUBY_INCLUDE_PATH)
	MESSAGE("Ruby Library and Development files found. Ruby extensions enabled.")
	MESSAGE(${RUBY_INCLUDE_PATH})
	SET(USE_RUBY True)
ELSE(RUBY_INCLUDE_PATH)
	MESSAGE("Ruby Library or Development files *not* found. Ruby extensions disabled.")
ENDIF(RUBY_INCLUDE_PATH)

FIND_PACKAGE(PythonLibs QUIET)
IF(PYTHON_INCLUDE_PATH)
	MESSAGE("Python Library and Development files found. Python extensions enabled.")
	SET(USE_PYTHON True)
ELSE(PYTHON_INCLUDE_PATH)
	MESSAGE("Python Library or Development files *not* found. Python extensions disabled.")
ENDIF(PYTHON_INCLUDE_PATH)

###################
# List of sources #
###################
# Define the UI source files used by Canorus
SET(Canorus_UIs  ui/mainwin.ui
	ui/keysig.ui
)

# Define the MOC source files used by Canorus
SET(Canorus_MOCs ui/mainwin.h	# List of classes which Qt needs to produce MOCs for them (in order for events, signals, slots and other macros to work)
	ui/keysigpsp.h
	widgets/keyoffs.h
	widgets/lcdnumber.h 
	widgets/scrollwidget.h 
	widgets/viewport.h 
	widgets/scoreviewport.h 
	widgets/sourceviewport.h
	widgets/toolbar.h
	interface/playback.h
)

SET(Canorus_Core_Srcs		# Core sources - data part of the MVC model
	core/document.cpp
	core/sheet.cpp
	core/kdtree.cpp
	
	core/context.cpp
	core/staff.cpp
	core/functionmarkingcontext.cpp
	
	core/muselement.cpp
	core/voice.cpp	
	core/barline.cpp
	core/clef.cpp
	core/keysignature.cpp
	core/timesignature.cpp
	core/playable.cpp
	core/note.cpp
	core/rest.cpp
	
	core/functionmarking.cpp
	
	core/canorusml.cpp
)

SET(Canorus_Scripting_Srcs	# Sources meant for bridging C++<->scripting language
	scripting/swigruby.cpp
	scripting/swigpython.cpp
)

SET(Canorus_Widget_Srcs  	# Sources for all the widgets present in Canorus
	widgets/keyoffs.cpp
	widgets/lcdnumber.cpp
	widgets/scoreviewport.cpp
	widgets/sourceviewport.cpp
	widgets/scrollwidget.cpp
	widgets/toolbar.cpp
	widgets/viewport.cpp
)

SET(Canorus_Ui_Srcs		# Larger UI entities like main window or different perspectives
	ui/mainwin.cpp
	ui/keysigpsp.cpp
)

SET(Canorus_Drawable_Srcs	# Drawable instances of the data
	drawable/drawable.cpp

	drawable/drawablecontext.cpp
	drawable/drawablestaff.cpp
	drawable/drawablefunctionmarkingcontext.cpp

	drawable/drawablemuselement.cpp
	drawable/drawablenote.cpp
	drawable/drawablerest.cpp
	drawable/drawableaccidental.cpp
	drawable/drawableclef.cpp
	drawable/drawablekeysignature.cpp
	drawable/drawabletimesignature.cpp
	drawable/drawablebarline.cpp
	drawable/drawablefunctionmarking.cpp
)

SET(Canorus_Interface_Srcs	# Classes which glue the UI and Data part of the MVC model. Engraver, Playback, Plugin manager and others belong here.
	interface/engraver.cpp
	interface/playback.cpp
	interface/rtmididevice.cpp
	interface/pluginmanager.cpp
	interface/plugin.cpp
)

SET(Canorus_RtMidi_Srcs		# RtMIDI library
	rtmidi/RtMidi.cpp
)

SET(Canorus_Srcs
	main.cpp
	${Canorus_Core_Srcs}
	${Canorus_Scripting_Srcs}
	${Canorus_Drawable_Srcs}
	${Canorus_Ui_Srcs}
	${Canorus_Interface_Srcs}
	${Canorus_RtMidi_Srcs}
	${Canorus_Widget_Srcs}
)

SET(Canorus_Swig_Srcs	# Sources which Swig needs to build its Ruby module.
	core/context.cpp
	core/document.cpp
	core/sheet.cpp
	core/muselement.cpp
	core/staff.cpp
	core/playable.cpp
	core/note.cpp
	core/rest.cpp
	core/timesignature.cpp
	core/keysignature.cpp
	core/clef.cpp
	core/barline.cpp
	core/voice.cpp
	core/functionmarking.cpp
	core/functionmarkingcontext.cpp
)

SET(Canorus_Scripts	# Official scripts shipped with Canorus.
	scripts/newdocument.rb
)

SET(Canorus_Images	# Images on buttons, splash etc.
	ui/images/edit_add.png
	ui/images/clogosm.png
	ui/images/tsab.png
	ui/images/exportpdf.png
	ui/images/editredo.png
	ui/images/edit_remove.png
	ui/images/player_play.png
	ui/images/bmajgshmin.png
	ui/images/amajfshmin.png
	ui/images/bmajgmin.png
	ui/images/editundo.png
	ui/images/aflmajfmin.png
	ui/images/editcut.png
	ui/images/exportlilypond.png
	ui/images/filenew.png
	ui/images/filesave.png
	ui/images/player_stop.png
	ui/images/ts24.png
	ui/images/emajcshmin.png
	ui/images/ts34.png
	ui/images/ts38.png
	ui/images/eflmajcmin.png
	ui/images/ts68.png
	ui/images/dmajbmin.png
	ui/images/n0.png
	ui/images/n1.png
	ui/images/n2.png
	ui/images/n4.png
	ui/images/n8.png
	ui/images/tsc.png
	ui/images/cleftreble.png
	ui/images/gmajemin.png
	ui/images/n16.png
	ui/images/editcopy.png
	ui/images/fileprint.png
	ui/images/n32.png
	ui/images/clefbass.png
	ui/images/doubleflat.png
	ui/images/delete.png
	ui/images/clefalto.png
	ui/images/cshmajashmin.png
	ui/images/n64.png
	ui/images/editpaste.png
	ui/images/fshmajdshmin.png
	ui/images/fileopen.png
	ui/images/cflmajaflmin.png
	ui/images/player_pause.png
	ui/images/dflmajbflmin.png
	ui/images/gflmajeflmin.png
	ui/images/exit.png
	ui/images/fmajdmin.png
)

################
# Qt4 settings #
###############
# This line adds the QtXML library to the Qt libraries
# loaded when linking Canorus
SET(QT_USE_QTXML True)

# Add all resources to Canorus_Resrcs (icons, html files etc.)
SET(Canorus_Resrcs ui/mainwin.qrc ui/keysig.qrc)

# Add all the dependencies needed for the MainWindow
# add_file_dependencies( ui/mainwin.cpp ui/ui_mainwin.h )

# This scripts runs the resource compiler rcc.
# The result of it is stored into Canorus_UIC_Resrcs_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_ADD_RESOURCES( Canorus_Resrcs_Srcs ${Canorus_Resrcs} )

# This scripts runs the user interface compiler uic.
# The result of it is stored into Canorus_UIC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_UI( Canorus_UIC_Srcs ${Canorus_UIs} )

# This script runs the meta object compiler moc.
# The result of it is stored into Canorus_MOC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_CPP( Canorus_MOC_Srcs ${Canorus_MOCs} )

#########################
# Compiler Define flags #
#########################
# RtMidi cross-platform definitions
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux") #IF(LINUX) isn't supported by CMake yet
	ADD_DEFINITIONS(-D__LINUX_ALSASEQ__)
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
IF(APPLE)
	ADD_DEFINITIONS(-D__MACOSX_CORE__)
ENDIF(APPLE)
IF(WIN32 OR WIN64)
	ADD_DEFINITIONS(-D__WINDOWS_MM__)
ENDIF(WIN32 OR WIN64)

# Define default data dir, if under POSIX systems.
IF(NOT WIN32 AND NOT WIN64)
	ADD_DEFINITIONS("-DDEFAULT_DATA_DIR=${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_DATA_DIR}")
ENDIF(NOT WIN32 AND NOT WIN64)

# Define Scripting parameters
IF(USE_SWIG)
	ADD_DEFINITIONS(-DUSE_SWIG)
ENDIF(USE_SWIG)
IF(USE_RUBY)
	ADD_DEFINITIONS(-DUSE_RUBY)
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	ADD_DEFINITIONS(-DUSE_PYTHON)
ENDIF(USE_PYTHON)

####################################
# Include paths and set up targets #
####################################
# This needs to be added in order for #include to work inside .cpp files - it adds src/ to include path
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# This line adds the include path of the Qt
# directory to the compiler include path
INCLUDE(${QT_USE_FILE})

# These two lines add additional include paths
# for xml and test includes
INCLUDE_DIRECTORIES(${QT_QTXML_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${QT_QTTEST_INCLUDE_DIR})

IF(USE_SWIG)
	INCLUDE(${SWIG_USE_FILE})
ENDIF(USE_SWIG)

IF(USE_RUBY)
	INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i PROPERTIES CPLUSPLUS ON)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i PROPERTIES SWIG_FLAGS "-includeall")
	
	SET(CANORUS_RUBY_WRAP_CXX ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusrubyRUBY_wrap.cxx)
	SET_SOURCE_FILES_PROPERTIES(${CANORUS_RUBY_WRAP_CXX} PROPERTIES GENERATED TRUE)
	
	SWIG_ADD_MODULE(CanorusRuby ruby
		${Canorus_Swig_Srcs}
		${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i
	)
	SWIG_LINK_LIBRARIES(CanorusRuby ${RUBY_LIBRARY} ${QT_LIBRARIES})
ENDIF(USE_RUBY)

IF(USE_PYTHON)
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i PROPERTIES CPLUSPLUS ON)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i PROPERTIES SWIG_FLAGS "-includeall")
	
	SET(CANORUS_PYTHON_WRAP_CXX ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspythonPYTHON_wrap.cxx)
	SET_SOURCE_FILES_PROPERTIES(${CANORUS_PYTHON_WRAP_CXX} PROPERTIES GENERATED TRUE)
	
	SWIG_ADD_MODULE(CanorusPython python
		${Canorus_Swig_Srcs}
		${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i
	)
	SWIG_LINK_LIBRARIES(CanorusPython ${PYTHON_LIBRARY} ${QT_LIBRARIES})
ENDIF(USE_PYTHON)

# This line tells cmake to create the Canorus program.
# All dependent libraries like RtMidi must be added here.
# Attention: In contrast to Makefiles don't add "\" to separate lines
ADD_EXECUTABLE(canorus ${Canorus_UIC_Srcs}  ${Canorus_Srcs} 
                       ${Canorus_MOC_Srcs} ${Canorus_Resrcs_Srcs}
                       ${CANORUS_RUBY_WRAP_CXX}
                       ${CANORUS_PYTHON_WRAP_CXX}
)
IF(USE_RUBY)
	ADD_DEPENDENCIES(canorus CanorusRuby) # Build scripting modules first and then Canorus exe
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	ADD_DEPENDENCIES(canorus CanorusPython) # Build scripting modules first and then Canorus exe
ENDIF(USE_PYTHON)

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
# Add ${QT_QTTEST_LIBRARY} below to add the Qt Test library as well
TARGET_LINK_LIBRARIES(canorus ${QT_LIBRARIES} ${QT_QTXML_LIBRARY} ${RUBY_LIBRARY} ${PYTHON_LIBRARY})

############################
# RtMIDI Library additions #
############################
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	TARGET_LINK_LIBRARIES(canorus "asound")
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
IF(APPLE)
	TARGET_LINK_LIBRARIES(canorus "-framework CoreMidi")
	TARGET_LINK_LIBRARIES(canorus "-framework CoreAudio")
	TARGET_LINK_LIBRARIES(canorus "-framework CoreFoundation")
ENDIF(APPLE)
IF(WIN32 OR WIN64)
	TARGET_LINK_LIBRARIES(canorus "winmm.lib")
ENDIF(WIN32 OR WIN64)

################
# Installation #
################
# Install canorus exe and CanorusRuby lib
INSTALL(TARGETS canorus
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_BIN_DIR}
)
IF(USE_RUBY)
	INSTALL(TARGETS CanorusRuby LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_LIB_DIR})
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	INSTALL(TARGETS CanorusPython LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_LIB_DIR})
ENDIF(USE_PYTHON)

# Install scripts
INSTALL(FILES ${Canorus_Scripts} DESTINATION "${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_DATA_DIR}scripts")
# Install icons, images etc.
INSTALL(FILES ${Canorus_Images} DESTINATION "${CMAKE_INSTALL_PREFIX}${CANORUS_INSTALL_DATA_DIR}images")
