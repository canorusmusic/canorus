# Define Sources of this Canorus directory

# This module is used to resolve the file dependencies
# for QT files using an UI (User Unterface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

# Build universal binaries on Mac
IF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch ppc -arch i386")
ENDIF(APPLE)

#######################
# Scripting detection #
#######################
# The following lines detect scripting environments (SWIG, Ruby, Python etc.)
FIND_PACKAGE(SWIG QUIET)
IF(SWIG_FOUND AND NOT NO_SWIG)
	SET(USE_SWIG True)
	MESSAGE("SWIG found, using scripting extensions.")
ELSE(SWIG_FOUND AND NOT NO_SWIG)
	MESSAGE("SWIG *not* found, disabling SWIG!")
ENDIF(SWIG_FOUND AND NOT NO_SWIG)

FIND_PACKAGE(Ruby)	#Setup RUBY_INCLUDE_PATH and RUBY_LIBRARIES
IF(RUBY_INCLUDE_PATH AND NOT NO_RUBY)
	MESSAGE("Ruby Library and Development files found. Ruby extensions enabled.")
	SET(USE_RUBY True)
ELSE(RUBY_INCLUDE_PATH AND NOT NO_RUBY)
	MESSAGE("Ruby Library and Development files *not* found. Ruby extensions disabled!")
	SET(RUBY_LIBRARY "")
ENDIF(RUBY_INCLUDE_PATH AND NOT NO_RUBY)

FIND_PACKAGE(PythonLibs)
IF(PYTHON_INCLUDE_PATH AND NOT NO_PYTHON)
	MESSAGE("Python Library and Development files found. Python extensions enabled.")
	SET(USE_PYTHON True)
ELSE(PYTHON_INCLUDE_PATH AND NOT NO_PYTHON)
	MESSAGE("Python Library and Development files *not* found. Python extensions disabled!")
	SET(PYTHON_LIBRARY "")
ENDIF(PYTHON_INCLUDE_PATH AND NOT NO_PYTHON)

###################
# List of sources #
###################
# Define the UI source files used by Canorus
SET(Canorus_UIs		# List of .ui forms drawn by qt-designer that need to be compiled
	ui/mainwin.ui
#	ui/keysig.ui
	ui/timesig.ui
	ui/settingsdialog.ui
)

# Define the MOC source files used by Canorus
SET(Canorus_MOCs
	ui/mainwin.h	# List of classes which Qt needs to produce MOCs for them (in order for events, signals, slots and other macros to work)
#	ui/keysigpsp.h  # obsolete
#	ui/timesigpsp.h # obsolete
	ui/settingsdialog.h
	ui/pluginaction.h
	widgets/keyoffs.h
	widgets/lcdnumber.h
	widgets/viewport.h
	widgets/viewportcontainer.h
	widgets/scoreviewport.h
	widgets/sourceviewport.h
	widgets/menutoolbutton.h
	interface/playback.h
	interface/mididevice.h
)

SET(Canorus_Core_Srcs		# Core sources - data part of the MVC model
	core/canorus.cpp
	core/settings.cpp
	core/canorusml.cpp
	core/undocommand.cpp

	core/document.cpp
	core/sheet.cpp
	core/context.cpp
	core/staff.cpp
	core/functionmarkingcontext.cpp
	core/lyricscontext.cpp

	core/muselement.cpp
	core/muselementfactory.cpp
	core/voice.cpp
	core/barline.cpp
	core/clef.cpp
	core/keysignature.cpp
	core/timesignature.cpp
	core/playable.cpp
	core/note.cpp
	core/slur.cpp
	core/rest.cpp

	core/syllable.cpp
	
	core/functionmarking.cpp
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
	widgets/menutoolbutton.cpp
	widgets/viewport.cpp
	widgets/viewportcontainer.cpp
)

SET(Canorus_Ui_Srcs		# Implementations of widgets that have their own .ui qt-designer files
	ui/mainwin.cpp
#	ui/keysigpsp.cpp  # obsolete
#	ui/timesigpsp.cpp # obsolete
	ui/settingsdialog.cpp
	ui/pluginaction.cpp
)

SET(Canorus_Drawable_Srcs	# Drawable instances of the data
	drawable/drawable.cpp

	drawable/drawablecontext.cpp
	drawable/drawablestaff.cpp
	drawable/drawablelyricscontext.cpp
	drawable/drawablefunctionmarkingcontext.cpp

	drawable/drawablemuselement.cpp
	drawable/drawablenote.cpp
	drawable/drawableslur.cpp
	drawable/drawablerest.cpp
	drawable/drawableaccidental.cpp
	drawable/drawableclef.cpp
	drawable/drawablekeysignature.cpp
	drawable/drawabletimesignature.cpp
	drawable/drawablebarline.cpp
	drawable/drawablesyllable.cpp
	drawable/drawablefunctionmarking.cpp
)

SET(Canorus_Interface_Srcs	# Classes which glue the UI and Data part of the MVC model. Engraver, Playback, Plugin manager and others belong here.
	interface/engraver.cpp
	interface/playback.cpp
	interface/rtmididevice.cpp
	interface/pluginmanager.cpp
	interface/plugin.cpp
)

SET(Canorus_Export_Srcs     # Classes for exporting Canorus data to various file formats
	export/lilypondexport.cpp
)

SET(Canorus_Import_Srcs     # Classes for import various file formats to Canorus data
	import/lilypondimport.cpp
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
	${Canorus_Export_Srcs}
	${Canorus_Import_Srcs}
	${Canorus_RtMidi_Srcs}
	${Canorus_Widget_Srcs}
)

SET(Canorus_Swig_Srcs	# Sources which Swig needs to build its Python/Ruby module.
	core/document.cpp
	core/sheet.cpp
	core/context.cpp
	core/staff.cpp
	core/lyricscontext.cpp
	core/functionmarkingcontext.cpp
	core/canorusml.cpp

	core/muselement.cpp
	core/voice.cpp
	core/barline.cpp
	core/clef.cpp
	core/keysignature.cpp
	core/timesignature.cpp
	core/playable.cpp
	core/note.cpp
	core/slur.cpp
	core/rest.cpp

	core/syllable.cpp
	
	core/functionmarking.cpp
)

SET(Canorus_Scripts	# Official scripts shipped with Canorus.
	scripts/newdocument.py
)

FILE(GLOB Canorus_Images "ui/images/*.png" "ui/images/*.svg")

# Set translation files
FILE(GLOB Canorus_Translation_Srcs "lang/*.ts") # lang/*.ts -> Canorus_Translation_Srcs
STRING(REGEX REPLACE ".ts" ".qm" Canorus_Translation_Bins "${Canorus_Translation_Srcs}") # Set Canorus_Translation_Bins to same values as Canorus_Translation_Srcs with different extensions

SET(Canorus_Fonts	# Music fonts needed by Canorus
	fonts/CenturySchL-BoldItal.ttf	# Century family needed for numbers and signs
	fonts/CenturySchL-Bold.ttf
	fonts/CenturySchL-Ital.ttf
	fonts/CenturySchL-Roma.ttf
	fonts/Emmentaler-14.ttf		# emmentaler needed for notes, rests, clefs etc.
	fonts/FreeSans.ttf		# FreeSans needed for function markings
)

IF(APPLE)
	SET(Canorus_OSX_Icon ${CANORUS_OSX_DIR}/files/canorus.icns)
	SET(Canorus_OSX_Info ${CANORUS_OSX_DIR}/files/Info.plist)

	SET(Canorus_OSX_Qt_libs
		${QT_QTCORE_LIBRARY}/Versions/4/QtCore
		${QT_QTGUI_LIBRARY}/Versions/4/QtGui
		${QT_QTXML_LIBRARY}/Versions/4/QtXml
		${QT_QTSVG_LIBRARY}/Versions/4/QtSvg
		)
ENDIF(APPLE)


################
# Qt4 settings #
###############
# This line adds the QtXML and QtSVG libraries to the Qt libraries
# loaded when linking Canorus
SET(QT_USE_QTXML True)
SET(QT_USE_QTSVG True)

# Add all resources to Canorus_Resrcs (icons, html files etc.)
SET(Canorus_Resrcs ui/keysig.qrc)

# Add all the dependencies needed for the MainWindow
# add_file_dependencies( ui/mainwin.cpp ui/ui_mainwin.h )

# This scripts runs the resource compiler rcc.
# The result of it is stored into Canorus_Resrcs_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_ADD_RESOURCES(Canorus_Resrcs_Srcs ${Canorus_Resrcs})

# This scripts runs the user interface compiler uic.
# The result of it is stored into Canorus_UIC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_UI(Canorus_UIC_Srcs ${Canorus_UIs})

# This script runs the meta object compiler moc.
# The result of it is stored into Canorus_MOC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_CPP(Canorus_MOC_Srcs ${Canorus_MOCs})

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
IF(MINGW)
	ADD_DEFINITIONS(-D__WINDOWS_MM__)
ENDIF(MINGW)

# Define default data dir, if under POSIX systems.
IF(NOT MINGW)
	IF (NOT DEFAULT_DATA_DIR)
		ADD_DEFINITIONS("-DDEFAULT_DATA_DIR=\\\"${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}\\\"")
	ELSE (NOT DEFAULT_DATA_DIR)
		ADD_DEFINITIONS("-DDEFAULT_DATA_DIR=\\\"${DEFAULT_DATA_DIR}\\\"")
	ENDIF (NOT DEFAULT_DATA_DIR)
ENDIF(NOT MINGW)

IF(APPLE)
	SET(MACOSX_BUNDLE_INFO_STRING "Free music score editor")
	SET(MACOSX_BUNDLE_ICON_FILE canorus.icns) # \todo Copy this file to Applications/canorus.app/Resources when the package is created
ENDIF(APPLE)

# Define Canorus version
ADD_DEFINITIONS("-DCANORUS_VERSION=\\\"${CANORUS_VERSION}\\\"")

# Define Scripting parameters
IF(USE_SWIG)
	ADD_DEFINITIONS(-DUSE_SWIG)
ENDIF(USE_SWIG)
IF(USE_RUBY)
	ADD_DEFINITIONS(-DUSE_RUBY)
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	ADD_DEFINITIONS(-DUSE_PYTHON)
	ADD_DEFINITIONS(-DSWIG_PYTHON_SILENT_MEMLEAK) # don't output memory leak warnings when creating new elements - we still use them in Canorus later and destroy them there!
ENDIF(USE_PYTHON)

####################################
# Include paths and set up targets #
####################################
# This needs to be added in order for #include to work inside .cpp files - it adds src/ to include path
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# This line adds the include path of the Qt
# directory to the compiler include path
INCLUDE(${QT_USE_FILE})

IF(USE_SWIG)
	INCLUDE(${SWIG_USE_FILE})
	SET(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_SOURCE_DIR})
ENDIF(USE_SWIG)

IF(USE_RUBY)
	INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i PROPERTIES CPLUSPLUS ON)
#	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i PROPERTIES SWIG_FLAGS "-includeall")

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
#	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i PROPERTIES SWIG_FLAGS "-includeall")

	SET(CANORUS_PYTHON_WRAP_CXX ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspythonPYTHON_wrap.cxx)
	SET_SOURCE_FILES_PROPERTIES(${CANORUS_PYTHON_WRAP_CXX} PROPERTIES GENERATED TRUE)

	SWIG_ADD_MODULE(CanorusPython python 		# Warning! CMake renames CanorusPython automatically to _CanorusPython
		${Canorus_Swig_Srcs}
		${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i
	)
	SWIG_LINK_LIBRARIES(CanorusPython ${PYTHON_LIBRARY} ${QT_LIBRARIES})
ENDIF(USE_PYTHON)

# Adds Canorus resource file under Windows (Canorus icon etc.).
IF(MINGW)
	# CMake currently doesn't support windres yet. Add command manually
	FIND_PROGRAM(
		WINDRES_PROGRAM windres.exe PATHS
		"[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\MinGW;InstallLocation]/bin"
		c:/MinGW/bin /MinGW/bin
	)
	ADD_CUSTOM_COMMAND(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/canorusrc.obj
		COMMAND ${WINDRES_PROGRAM}
		-i canorusrc.rc
		-o canorusrc.obj
	)
	SET(Canorus_Srcs ${Canorus_Srcs} canorusrc.obj)
ENDIF(MINGW)
	
# This line tells cmake to create the Canorus program.
# All dependent libraries like RtMidi must be added here.
# Attention: In contrast to Makefiles don't add "\" to separate lines
ADD_EXECUTABLE(canorus ${Canorus_UIC_Srcs}  ${Canorus_Srcs}
                       ${Canorus_MOC_Srcs} ${Canorus_Resrcs_Srcs}
                       ${CANORUS_RUBY_WRAP_CXX}
                       ${CANORUS_PYTHON_WRAP_CXX}
                       ${MACOSX_BUNDLE}	# Works only under Apple - adds the application description, icon etc.
)
IF(USE_RUBY)
	ADD_DEPENDENCIES(canorus CanorusRuby) # Build scripting modules first and then Canorus exe
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	ADD_DEPENDENCIES(canorus _CanorusPython) # Build scripting modules first and then Canorus exe
ENDIF(USE_PYTHON)

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
# Add ${QT_QTTEST_LIBRARY} below to add the Qt Test library as well
TARGET_LINK_LIBRARIES(canorus ${QT_LIBRARIES} ${RUBY_LIBRARY} ${PYTHON_LIBRARY})

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
IF(MINGW)
	TARGET_LINK_LIBRARIES(canorus "winmm.lib")
ENDIF(MINGW)

###############
# Translation #
###############
ADD_CUSTOM_COMMAND(
	TARGET canorus
	PRE_BUILD
	COMMAND ${QT_BINARY_DIR}/lupdate
	ARGS -noobsolete ${Canorus_UIC_Srcs} ${Canorus_Srcs} -ts ${Canorus_Translation_Srcs} lang/template.ts
)
ADD_CUSTOM_COMMAND(
	TARGET canorus
	PRE_BUILD
	COMMAND ${QT_BINARY_DIR}/lrelease
	ARGS ${Canorus_Translation_Srcs}
)

################
# Installation #
################
# Install canorus exe and CanorusRuby lib
INSTALL(TARGETS canorus
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}
)
IF(USE_RUBY)
	INSTALL(TARGETS CanorusRuby LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	INSTALL(TARGETS CanorusRuby LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	INSTALL(TARGETS _CanorusPython LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CanorusPython.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	INSTALL(TARGETS _CanorusPython LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
	INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CanorusPython.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
ENDIF(USE_PYTHON)

# Install scripts
INSTALL(FILES ${Canorus_Scripts} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/scripts")
# Install data - icons, images, sounds etc.
INSTALL(FILES ${Canorus_Images} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/images") # all images are currently included in exe
# Install translations
INSTALL(FILES ${Canorus_Translation_Bins} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/lang")
# Install fonts
INSTALL(FILES ${Canorus_Fonts} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_FONTS_DIR}")

IF(MINGW)
	# Install Qt libs
	INSTALL( FILES ${QT_BIN_DIR}/QtCore4.dll ${QT_BIN_DIR}/QtGui4.dll ${QT_BIN_DIR}/QtSvg4.dll ${QT_BIN_DIR}/QtXml4.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	
	# Install qsvg1.dll plugin for rendering SVG icons
	INSTALL( FILES ${QT_PLUGINS_DIR}/iconengines/qsvg1.dll DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}/iconengines" )
	
	# Install mingwm10.dll
	IF(MINGW)
		INSTALL( FILES ${MINGW_BIN_DIR}/mingwm10.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	ENDIF(MINGW)
	
	# Install Python libs
	IF(USE_PYTHON)
		INSTALL( FILES ${PYTHON_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	ENDIF(USE_PYTHON)
	
	# Install Ruby libs
	IF(USE_RUBY)
		INSTALL( FILES ${RUBY_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	ENDIF(USE_RUBY)
ENDIF(MINGW)

IF(APPLE)

	# Install icon & Info.plist
	INSTALL(FILES ${Canorus_OSX_Icon} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
	INSTALL(FILES ${Canorus_OSX_Info} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_OSX_APP_DIR})

	# FIXME: library installations disabled for now since linking bundled libs doesn't work properly.
	# -- Sam 2007.06.09

	# Install Qt libs
	#INSTALL(PROGRAMS ${Canorus_OSX_Qt_libs} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})

	# Install Python libs
	IF(USE_PYTHON)
		# OSX build requires a "framework" build of Python
		# in CMakeCache.txt PYTHON_LIBRARY is not a file but '-framework Python'
		#SET(OSX_PYTHON_LIB /Library/Frameworks/Python.framework/Versions/Current/Python)		
		#INSTALL(FILES ${OSX_PYTHON_LIB} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	ENDIF(USE_PYTHON)

	# Make a list of source libraries so the packaging script knows which symbols to change
	# Bundle Qt libs and Python with the .app, OSX comes with the correct version of Ruby already installed
	#SET(OSX_installed_libs_file ${CANORUS_OSX_DIR}/OSX_installed_libs.txt)
	#SET(OSX_installed_libs ${Canorus_OSX_Qt_libs} ${OSX_PYTHON_LIB})
	#FILE(WRITE ${OSX_installed_libs_file} "")
	#FOREACH(file ${OSX_installed_libs})
	#	FILE(APPEND ${OSX_installed_libs_file} "${file}\n")
	#ENDFOREACH(file)
ENDIF(APPLE)

