# Define Sources of this Canorus directory

INCLUDE(CheckLibraryExists)

#FIND_PACKAGE(Qt5)

# This module is used to resolve the file dependencies
# for QT files using an UI (User Interface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)

IF(NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE Debug)
ENDIF(NOT CMAKE_BUILD_TYPE)

SET(CMAKE_C_FLAGS_DEBUG "-O0 -g -fstack-protector-all -Wall -Wuninitialized -Wtype-limits -Wstack-protector")
SET(CMAKE_C_FLAGS_RELEASE "-O2")

SET(CMAKE_CXX_STANDARD 14)

SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE}")

# Build universal binaries on Mac
IF(APPLE)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -arch ppc -arch i386")
	SET(DEFAULT_DATA_DIR "Canorus.app/Contents/Resources")
ENDIF(APPLE)

#######################
# Linux: Detect Alsa  #
#######################
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
        FIND_PACKAGE(ALSA REQUIRED)
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")


#######################
# Scripting detection #
#######################
# The following lines detect scripting environments (SWIG, Ruby, Python etc.)
FIND_PACKAGE(SWIG 3.0.8 QUIET)
IF(SWIG_FOUND AND NOT NO_SWIG)
	SET(USE_SWIG True)
	IF(${CMAKE_VERSION} VERSION_GREATER "3.13")
		cmake_policy(SET CMP0078 NEW)
		cmake_policy(SET CMP0086 NEW)
	ENDIF(${CMAKE_VERSION} VERSION_GREATER "3.13")
	MESSAGE("SWIG version ${SWIG_VERSION} found, using scripting extensions.")
ELSE(SWIG_FOUND AND NOT NO_SWIG)
	MESSAGE("SWIG version >= 3.0.8 *not* found, version is ${SWIG_VERSION}, disabling SWIG!")
	SET(NO_RUBY True)
	SET(NO_PYTHON True)
ENDIF(SWIG_FOUND AND NOT NO_SWIG)

FIND_PACKAGE(Ruby)	#Setup RUBY_INCLUDE_PATH and RUBY_LIBRARIES
IF(RUBY_INCLUDE_PATH AND NOT NO_RUBY)
	MESSAGE("Ruby Library and Development files found. Ruby extensions enabled.")
	SET(USE_RUBY True)
    # Create empty file as needed so build does not stop here		
	FILE(APPEND ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusrubyRUBY_wrap.cxx "")
ELSE(RUBY_INCLUDE_PATH AND NOT NO_RUBY)
	MESSAGE("Ruby Library and Development files *not* found. Ruby extensions disabled!")
	SET(RUBY_LIBRARY "")
ENDIF(RUBY_INCLUDE_PATH AND NOT NO_RUBY)

set(Python_ADDITIONAL_VERSIONS 3.4) # Required for Ubuntu LTS 14.04

FIND_PACKAGE(PythonLibs 3)

IF(PYTHON_INCLUDE_DIRS AND NOT NO_PYTHON)
	MESSAGE("Python Library and Development files found. Python extensions enabled.")
	# PYTHON_LIBRARY points to C:\PythonXY\pythonXY.dll
	# PYTHON_LIB should point to C:\PythonXY\Lib
	STRING(REGEX REPLACE "([^ ]+)[/\\].*" "\\1" PYTHON_LIB "${PYTHON_LIBRARY}") # remove pythonXY.dll
	STRING(REGEX REPLACE "\\\\" "/" PYTHON_LIB "${PYTHON_LIB}")  # Replace back slashes to slashes
	SET( PYTHON_LIB "${PYTHON_LIB}/Lib/" ) # add /Lib/

	# Needed on some Linux distros (SuSE, Gentoo) to prevent linking issues (openpty, forkpty) 
	CHECK_LIBRARY_EXISTS(util openpty "" HAVE_OPENPTY)
	IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
		IF (HAVE_OPENPTY)
			SET(PYTHON_LIBRARY ${PYTHON_LIBRARY} util)
		ENDIF (HAVE_OPENPTY)
	ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	
	SET(USE_PYTHON True)
ELSE(PYTHON_INCLUDE_DIRS AND NOT NO_PYTHON)
	MESSAGE("Python Library and Development files *not* found. Python extensions disabled!")
	SET(PYTHON_LIBRARY "")
ENDIF(PYTHON_INCLUDE_DIRS AND NOT NO_PYTHON)

##################################
# Poppler detection for printing #
##################################
#FIND_PATH(POPPLER_INCLUDE_PATH 
#	NAMES	poppler-config.h
#	PATH_SUFFIXES poppler
# )
#FIND_LIBRARY(POPPLER_LIBRARY NAMES poppler )
#FIND_LIBRARY(POPPLERQT4_LIBRARY NAMES poppler-qt4)
#IF(POPPLER_INCLUDE_PATH AND POPPLER_LIBRARY AND POPPLERQT4_LIBRARY)
#	MESSAGE("Poppler Libraries found. Printing enabled.")
#	MESSAGE("Include path: ${POPPLER_INCLUDE_PATH}" )
#	SET(USE_POPPLER True)
#	SET(POPPLER_SOURCE control/printctl.cpp)
#	SET(POPPLER_HEADER control/printctl.h)
#ELSE(POPPLER_INCLUDE_PATH AND POPPLER_LIBRARY AND POPPLERQT4_LIBRARY)
#	MESSAGE("Poppler Library and Development files *not* found. Printing disabled!")
#	MESSAGE("Path ${POPPLER_INCLUDE_PATH}, Lib ${POPPLER_LIBRARIES}")
#	SET(POPPLER_LIBRARY "")
#	SET(POPPLER_INCLUDE_PATH "")
#	SET(POPPLERQT4_LIBRARY "")
#	SET(POPPLER_SOURCE "")
#	SET(POPPLER_HEADER "")
#ENDIF(POPPLER_INCLUDE_PATH AND POPPLER_LIBRARY AND POPPLERQT4_LIBRARY)

# Set ZLib sources on windows
FILE(GLOB ZLIB_Srcs "../lib/zlib-1.2.3/*.c")

##############################
# List of Qt4-parsed sources #
##############################
# Define the UI source files used by Canorus
SET(Canorus_UIs		# List of .ui forms drawn by qt-designer that need to be compiled
	ui/mainwin.ui
	ui/settingsdialog.ui
	
	ui/propertiesdialog.ui
	ui/documentproperties.ui
	ui/sheetproperties.ui
	ui/staffproperties.ui
	ui/voiceproperties.ui
	ui/lyricscontextproperties.ui
	ui/functionmarkcontextproperties.ui
	ui/chordnamecontextproperties.ui

	ui/midirecorder.ui
	ui/transposeview.ui
	ui/jumptoview.ui
)

# Define the MOC source files used by Canorus
SET(Canorus_Gui_MOCs	# List of classes which Qt needs to produce MOCs for them (in order for events, signals, slots and other macros to work)
	core/autorecovery.h

	scoreui/keysignatureui.h
	
	ui/mainwin.h
	ui/settingsdialog.h
	ui/propertiesdialog.h
	ui/transposeview.h
	ui/jumptoview.h

	widgets/lcdnumber.h
	widgets/view.h
	widgets/viewcontainer.h
	widgets/scoreview.h
	widgets/sourceview.h
	widgets/toolbutton.h
	widgets/toolbuttonpopup.h
	widgets/menutoolbutton.h
	widgets/undotoolbutton.h
	widgets/pyconsole.h
	widgets/midirecorderview.h
	widgets/resourceview.h
	widgets/actionseditor.h
	widgets/progressstatusbar.h
	widgets/tabwidget.h
	
	control/previewctl.h
	control/printctl.h
	control/mainwinprogressctl.h

	scorectl/keysignaturectl.h
)

IF(Qt5WebEngineWidgets_LIBRARIES)
	SET(Canorus_Gui_MOCs ${Canorus_Gui_MOCs} widgets/helpbrowser.h)
ENDIF(Qt5WebEngineWidgets_LIBRARIES)

SET(Canorus_Core_MOCs # MOCs compiled into scripting library as well
	import/import.h
	import/lilypondimport.h
	import/midiimport.h
	import/musicxmlimport.h
	export/export.h
	export/lilypondexport.h
	export/pdfexport.h
	export/svgexport.h

	control/externprogram.h
	control/typesetctl.h
	
	interface/mididevice.h
	interface/playback.h
	core/midirecorder.h
	core/settings.h

	interface/pluginaction.h
)

################
# Qt5 settings #
################
# Enable needed Qt modules for Canorus
SET(QT_USE_QTXML True) # Save/Load
SET(QT_USE_QTSVG True) # Icons, Images etc.

# Add all resources to Canorus_Resrcs (icons, html files etc.)
#SET(Canorus_Resrcs ui/mainwin.qrc) # not needed, all resources are loaded on-the-fly

# Add all the dependencies needed for the MainWindow
# add_file_dependencies( ui/mainwin.cpp ui/ui_mainwin.h )

# This scripts runs the resource compiler rcc.
# The result of it is stored into Canorus_Resrcs_Srcs
# variable. This has to be included later in the
# add_executable line.
QT5_ADD_RESOURCES(Canorus_Resrcs_Srcs ${Canorus_Resrcs})

# This scripts runs the user interface compiler uic.
# The result of it is stored into Canorus_UIC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT5_WRAP_UI(Canorus_UIC_Srcs ${Canorus_UIs})

# This script runs the meta object compiler moc.
# The result of it is stored into Canorus_Gui_MOC_Srcs and Canorus_Core_MOC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT5_WRAP_CPP(Canorus_Gui_MOC_Srcs ${Canorus_Gui_MOCs})
QT5_WRAP_CPP(Canorus_Core_MOC_Srcs ${Canorus_Core_MOCs})

#########################
# List of other sources #
#########################
SET(Canorus_Core_Srcs		# Core sources
	core/settings.cpp
	core/undocommand.cpp
	core/undo.cpp
	core/autorecovery.cpp
	core/mimedata.cpp
	core/file.cpp
	core/fileformats.cpp
	core/typesetter.cpp
	core/tar.cpp
	core/archive.cpp
	core/midirecorder.cpp
	core/muselementfactory.cpp
	core/transpose.cpp
	core/notechecker.cpp
	core/actiondelegate.cpp
)

SET(Canorus_Score_Srcs		# Score representation
	score/playablelength.cpp
	score/diatonicpitch.cpp
	score/interval.cpp
	score/diatonickey.cpp
	
	score/document.cpp
	score/resource.cpp
	score/sheet.cpp
	score/notecheckererror.cpp
	score/context.cpp
	score/staff.cpp
	score/functionmarkcontext.cpp
	score/figuredbasscontext.cpp
	score/lyricscontext.cpp
	score/chordnamecontext.cpp
	
	score/muselement.cpp
	score/voice.cpp
	score/barline.cpp
	score/clef.cpp
	score/keysignature.cpp
	score/timesignature.cpp
	score/playable.cpp
	score/note.cpp
	score/slur.cpp
	score/tuplet.cpp
	score/rest.cpp
	score/midinote.cpp
	
	score/mark.cpp
	score/articulation.cpp
	score/fermata.cpp
	score/repeatmark.cpp
	score/tempo.cpp
	score/ritardando.cpp
	score/text.cpp
	score/bookmark.cpp
	score/fingering.cpp
	score/dynamic.cpp
	score/crescendo.cpp
	score/instrumentchange.cpp
	
	score/syllable.cpp
	
	score/functionmark.cpp
	
	score/figuredbassmark.cpp

	score/chordname.cpp
)

SET(Canorus_Scripting_Srcs	# Sources meant for bridging C++<->scripting language
	scripting/swigruby.cpp
	scripting/swigpython.cpp
)

SET(Canorus_Widget_Srcs  	# Sources for all custom widgets present in Canorus
	widgets/lcdnumber.cpp
	widgets/scoreview.cpp
	widgets/sourceview.cpp
	widgets/toolbutton.cpp
	widgets/toolbuttonpopup.cpp
	widgets/menutoolbutton.cpp
	widgets/undotoolbutton.cpp
	widgets/view.cpp
	widgets/viewcontainer.cpp
	widgets/pyconsole.cpp
	widgets/midirecorderview.cpp
	widgets/resourceview.cpp
	widgets/actionseditor.cpp
	widgets/progressstatusbar.cpp
	widgets/tabwidget.cpp
)

IF(Qt5WebEngineWidgets_LIBRARIES)
	SET(Canorus_Widget_Srcs ${Canorus_Widget_Srcs} widgets/helpbrowser.cpp)
ENDIF(Qt5WebEngineWidgets_LIBRARIES)

SET(Canorus_Ui_Srcs		# Implementations of widgets that have their own .ui qt-designer files
	ui/mainwin.cpp
	ui/settingsdialog.cpp
	ui/propertiesdialog.cpp
	ui/transposeview.cpp
	ui/jumptoview.cpp
	ui/singleaction.cpp
	ui/actionstorage.cpp

	scoreui/keysignatureui.cpp
)

SET(Canorus_Gui_Ctl_Srcs       # Control instances for user interface or views 
	control/previewctl.cpp
	control/printctl.cpp
	control/mainwinprogressctl.cpp
	control/helpctl.cpp

	scorectl/keysignaturectl.cpp
)

SET(Canorus_Ctl_Srcs            # Control instances for user interface or views and core
	control/externprogram.cpp
	control/typesetctl.cpp
	control/resourcectl.cpp
)

SET(Canorus_Layout_Srcs	# Drawable instances of the data
	layout/layoutengine.cpp
	
	layout/drawable.cpp

	layout/drawablecontext.cpp
	layout/drawablenotecheckererror.cpp
	layout/drawablestaff.cpp
	layout/drawablelyricscontext.cpp
	layout/drawablefiguredbasscontext.cpp
	layout/drawablefunctionmarkcontext.cpp
	layout/drawablechordnamecontext.cpp

	layout/drawablemuselement.cpp
	layout/drawablenote.cpp
	layout/drawableslur.cpp
	layout/drawabletuplet.cpp
	layout/drawablerest.cpp
	layout/drawablemidinote.cpp
	layout/drawableaccidental.cpp
	layout/drawableclef.cpp
	layout/drawablekeysignature.cpp
	layout/drawabletimesignature.cpp
	layout/drawablebarline.cpp
	layout/drawablemark.cpp
	layout/drawablesyllable.cpp
	layout/drawablefiguredbassnumber.cpp
	layout/drawablefunctionmark.cpp
	layout/drawablechordname.cpp
)

SET(Canorus_Interface_Srcs	# Other interfaces like Engraver, Playback, Plugin manager and others belong here.
	interface/playback.cpp
	interface/rtmididevice.cpp
	interface/mididevice.cpp
	interface/pluginmanager.cpp
	interface/pluginaction.cpp
	interface/plugin.cpp
	interface/keybdinput.cpp

	interface/pyconsoleinterface.cpp
)

SET(Canorus_Export_Srcs     # Classes for exporting Canorus data to various file formats
	export/export.cpp
	export/midiexport.cpp
	export/lilypondexport.cpp
	export/canorusmlexport.cpp
	export/canexport.cpp
	export/musicxmlexport.cpp
	export/pdfexport.cpp
	export/svgexport.cpp
)

SET(Canorus_Import_Srcs     # Classes for import various file formats to Canorus data
	import/import.cpp
	import/lilypondimport.cpp
	import/midiimport.cpp
	import/canorusmlimport.cpp
	import/canimport.cpp
	import/musicxmlimport.cpp
	import/mxlimport.cpp
)

SET(Canorus_RtMidi_Srcs		# RtMIDI library
	../lib/rtmidi-4.0.0/RtMidi.cpp
)

SET(Canorus_ZIP_Srcs
    zip/zip.c
)

SET(Canorus_PMIDI_Srcs		# PMIDI excerpts
	import/pmidi/glib.c
	import/pmidi/elements.c
	import/pmidi/except.c
	import/pmidi/mdutil.c
	import/pmidi/midiread.c
	import/pmidi/wrapper.c
)

SET(Canorus_Srcs
	main.cpp
	canorus.cpp
	
	${Canorus_Core_Srcs}
	${Canorus_Score_Srcs}
	${Canorus_Ctl_Srcs}
	${Canorus_Gui_Ctl_Srcs}
	${Canorus_Scripting_Srcs}
	${Canorus_Layout_Srcs}
	${Canorus_Ui_Srcs}
	${Canorus_Interface_Srcs}
	${Canorus_Export_Srcs}
	${Canorus_Import_Srcs}
	${Canorus_RtMidi_Srcs}
	${Canorus_ZIP_Srcs}
	${Canorus_Widget_Srcs}
	${Canorus_PMIDI_Srcs}
)

SET(Canorus_Swig_Srcs	# Sources which Swig needs to build its Python/Ruby module.
	${Canorus_Score_Srcs}
	core/transpose.cpp
	
	core/settings.cpp
	core/file.cpp
	core/tar.cpp
	core/archive.cpp
	core/midirecorder.cpp
	core/typesetter.cpp
	${Canorus_Import_Srcs}
	${Canorus_Export_Srcs}
	${Canorus_Ctl_Srcs}
	${Canorus_RtMidi_Srcs}
	${Canorus_ZIP_Srcs}
	${Canorus_PMIDI_Srcs}
	interface/rtmididevice.cpp
	interface/mididevice.cpp
	interface/playback.cpp

	interface/pyconsoleinterface.cpp
	interface/plugin.cpp
	interface/pluginaction.cpp
	interface/pluginmanager.cpp
	interface/plugins_swig.cpp
	scripting/swigpython.cpp
	scripting/swigruby.cpp
	${Canorus_Core_MOC_Srcs}
)

SET(Canorus_Fmt_Srcs    # All Canorus sources that need code style formatting.
	main.cpp
	canorus.cpp

	${Canorus_Core_Srcs}
	${Canorus_Score_Srcs}
	${Canorus_Ctl_Srcs}
	${Canorus_Gui_Ctl_Srcs}
	${Canorus_Scripting_Srcs}
	${Canorus_Layout_Srcs}
	${Canorus_Ui_Srcs}
	${Canorus_Interface_Srcs}
	${Canorus_Export_Srcs}
	${Canorus_Import_Srcs}
	${Canorus_Widget_Srcs}
)

IF(MINGW) # Append ZLIB srcs to Swig srcs on Windows
	SET( Canorus_Swig_Srcs ${Canorus_Swig_Srcs} ${ZLIB_Srcs} )
ENDIF(MINGW)

SET( Canorus_Scripts "scripts" ) # Official scripts shipped with Canorus.

SET( Canorus_Plugins "plugins" ) # Official plug-ins shipped with Canorus.

SET( Canorus_Images "ui/images" )

SET( Canorus_Examples "../examples" )

set_source_files_properties(${Canorus_RtMidi_Srcs} PROPERTIES COMPILE_FLAGS -Wno-stack-protector)

# Set translation files
FILE(GLOB Canorus_Translation_Srcs "lang/*.ts") # lang/*.ts -> Canorus_Translation_Srcs

SET(Canorus_Fonts	# Music fonts needed by Canorus
	fonts/CenturySchL-BoldItal.ttf	# Century family needed for numbers and signs
	fonts/CenturySchL-Bold.ttf
	fonts/CenturySchL-Ital.ttf
	fonts/CenturySchL-Roma.ttf
	fonts/Emmentaler-14.ttf		# LilyPond's emmentaler needed for notes, rests, clefs etc.
	fonts/FreeSans.ttf		# FreeSans needed for function markings
)

IF(APPLE)
	SET(Canorus_OSX_Icon ${CANORUS_OSX_DIR}/files/canorus.icns)
	SET(Canorus_OSX_Info ${CANORUS_OSX_DIR}/files/Info.plist)

	SET(Canorus_OSX_Qt_libs
		${QT_QTCORE_LIBRARY}/Versions/5/QtCore
		${QT_QTGUI_LIBRARY}/Versions/5/QtGui
		${QT_QTWIDGETS_LIBRARY}/Versions/5/QtWidgets
		${QT_QTXML_LIBRARY}/Versions/5/QtXml
		${QT_QTSVG_LIBRARY}/Versions/5/QtSvg
		)
ENDIF(APPLE)

#########################
# Compiler Define flags #
#########################
# RtMidi cross-platform definitions
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux") #IF(LINUX) isn't supported by CMake yet
	ADD_DEFINITIONS(-D__LINUX_ALSA__)
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
		ADD_DEFINITIONS(-DDEFAULT_DATA_DIR=\"${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}\")
	ELSE (NOT DEFAULT_DATA_DIR)
		ADD_DEFINITIONS(-DDEFAULT_DATA_DIR=\"${DEFAULT_DATA_DIR}\")
	ENDIF (NOT DEFAULT_DATA_DIR)
ENDIF(NOT MINGW)

IF(APPLE)
	SET(MACOSX_BUNDLE_INFO_STRING "Free music score editor")
	SET(MACOSX_BUNDLE_ICON_FILE canorus.icns) # \todo Copy this file to Applications/canorus.app/Resources when the package is created
ENDIF(APPLE)

# Define Canorus version
ADD_DEFINITIONS(-DCANORUS_VERSION=\"${CANORUS_VERSION}\")

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
#IF(USE_POPPLER)
#	ADD_DEFINITIONS(-DUSE_POPPLER)
#ENDIF(USE_POPPLER)

####################################
# Include paths and set up targets #
####################################
# This needs to be added in order for #include to work inside .cpp files - it adds src/ to include path
#TARGET_INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/core")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/ui")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/widgets")
INCLUDE_DIRECTORIES("${CMAKE_CURRENT_SOURCE_DIR}/import")
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

# These lines add the include paths of the Qt
# directory to the compiler include paths
INCLUDE_DIRECTORIES(${Qt5Core_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Gui_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Svg_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Widgets_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5Xml_INCLUDE_DIRS})
INCLUDE_DIRECTORIES(${Qt5PrintSupport_INCLUDE_DIRS})

# FIX: Qt was built with -reduce-relocations
if (Qt5_POSITION_INDEPENDENT_CODE)
	SET(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

IF(USE_SWIG)
	INCLUDE(${SWIG_USE_FILE})
	SET(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})
ENDIF(USE_SWIG)

IF(USE_RUBY)
	INCLUDE_DIRECTORIES(${RUBY_INCLUDE_PATH})
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i PROPERTIES CPLUSPLUS ON)
			
	SET(CANORUS_RUBY_WRAP_CXX ${CMAKE_CURRENT_BINARY_DIR}/canorusrubyRUBY_wrap.cxx)
	SET_SOURCE_FILES_PROPERTIES(${CANORUS_RUBY_WRAP_CXX} PROPERTIES GENERATED TRUE)

	SWIG_ADD_LIBRARY(CanorusRuby
		LANGUAGE ruby
		OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
		SOURCES ${Canorus_Swig_Srcs} ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canorusruby.i
	)
	SWIG_LINK_LIBRARIES(CanorusRuby ${RUBY_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Svg Qt5::Xml Qt5::PrintSupport)
	SET_TARGET_PROPERTIES( ${SWIG_MODULE_CanorusRuby_REAL_NAME} PROPERTIES COMPILE_FLAGS "-DSWIGCPP" )
ENDIF(USE_RUBY)

IF(USE_PYTHON)
	INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_DIRS})
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i PROPERTIES CPLUSPLUS ON)
	SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i PROPERTIES SWIG_FLAGS "-threads")
	
	SET(CANORUS_PYTHON_WRAP_CXX ${CMAKE_CURRENT_BINARY_DIR}/canoruspythonPYTHON_wrap.cxx)
	SET_SOURCE_FILES_PROPERTIES(${CANORUS_PYTHON_WRAP_CXX} PROPERTIES GENERATED TRUE)
	
	SWIG_ADD_LIBRARY(CanorusPython
		LANGUAGE python
        OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}
		SOURCES ${Canorus_Swig_Srcs} ${CMAKE_CURRENT_SOURCE_DIR}/scripting/canoruspython.i
	)
	SWIG_LINK_LIBRARIES(CanorusPython ${PYTHON_LIBRARY} Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Svg Qt5::Xml Qt5::PrintSupport)
	SET_TARGET_PROPERTIES( ${SWIG_MODULE_CanorusPython_REAL_NAME} PROPERTIES COMPILE_FLAGS "-DSWIGCPP" )
	set_source_files_properties(${CANORUS_PYTHON_WRAP_CXX} PROPERTIES COMPILE_FLAGS -Wno-error)
ENDIF(USE_PYTHON)

#IF(USE_POPPLER)
#	INCLUDE_DIRECTORIES(${POPPLER_INCLUDE_PATH})
#ENDIF(USE_POPPLER)

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
	SET(Canorus_Srcs ${Canorus_Srcs} ${ZLIB_Srcs} canorusrc.obj)
ENDIF(MINGW)
	
# This line tells cmake to create the Canorus program.
# All dependent libraries like RtMidi must be added here.
# Attention: In contrast to Makefiles don't add "\" to separate lines
ADD_EXECUTABLE(canorus ${Canorus_UIC_Srcs}  ${Canorus_Srcs}
                       ${Canorus_Core_MOC_Srcs} ${Canorus_Gui_MOC_Srcs} ${Canorus_Resrcs_Srcs}
                       ${CANORUS_RUBY_WRAP_CXX}
                       ${CANORUS_PYTHON_WRAP_CXX}
                       ${MACOSX_BUNDLE}	# Works only under Apple - adds the application description, icon etc.
)
IF(USE_RUBY)
	ADD_DEPENDENCIES(canorus ${SWIG_MODULE_CanorusRuby_REAL_NAME}) # Build scripting modules first and then Canorus exe
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	ADD_DEPENDENCIES(canorus ${SWIG_MODULE_CanorusPython_REAL_NAME}) # Build scripting modules first and then Canorus exe
ENDIF(USE_PYTHON)

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
# Add ${QT_QTTEST_LIBRARY} below to add the Qt Test library as well
# Add ${POPPLERQT4_LIBRARY} ${POPPLER_LIBRARY} to reactivate poppler libraries
TARGET_LINK_LIBRARIES(canorus Qt5::Widgets Qt5::Core Qt5::Gui Qt5::Svg Qt5::Xml Qt5::PrintSupport ${Qt5WebEngineWidgets_LIBRARIES} ${RUBY_LIBRARY} ${PYTHON_LIBRARY} z pthread )
# Duma leads to a crash on libfontconfig with Ubuntu (10.04/12.04)
# duma )

############################
# RtMIDI Library additions #
############################
IF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
	TARGET_LINK_LIBRARIES(canorus "asound")
	IF(USE_PYTHON)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusPython_REAL_NAME} "asound")
	ENDIF(USE_PYTHON)
	IF(USE_RUBY)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusRuby_REAL_NAME} "asound")
	ENDIF(USE_RUBY)
ENDIF("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
IF(APPLE)
	TARGET_LINK_LIBRARIES(canorus "-framework CoreMidi")
	TARGET_LINK_LIBRARIES(canorus "-framework CoreAudio")
	TARGET_LINK_LIBRARIES(canorus "-framework CoreFoundation")
	IF(USE_PYTHON)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusPython_REAL_NAME} "-framework CoreMidi")
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusPython_REAL_NAME} "-framework CoreAudio")
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusPython_REAL_NAME} "-framework CoreFoundation")
	ENDIF(USE_PYTHON)
	IF(USE_RUBY)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusRuby_REAL_NAME} "-framework CoreMidi")
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusRuby_REAL_NAME} "-framework CoreAudio")
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusRuby_REAL_NAME} "-framework CoreFoundation")
	ENDIF(USE_RUBY)
ENDIF(APPLE)
IF(MINGW)
	TARGET_LINK_LIBRARIES(canorus "winmm.lib")
	TARGET_LINK_LIBRARIES(canorus "-mwindows") # Disable console output on Windows
	IF(USE_PYTHON)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusPython_REAL_NAME} "winmm.lib")
	ENDIF(USE_PYTHON)
	IF(USE_RUBY)
		TARGET_LINK_LIBRARIES(${SWIG_MODULE_CanorusRuby_REAL_NAME} "winmm.lib")
	ENDIF(USE_RUBY)
ENDIF(MINGW)

###############
# Translation #
###############
# Do not use qt5_create_translation since make clean will delete all previous
# .ts files. Use custom command instead:
#qt5_create_translation(Canorus_Translation_Bins ${Canorus_UIC_Srcs} ${Canorus_Srcs} lang/template.ts)
ADD_CUSTOM_TARGET(tr
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS ${Canorus_UIC_Srcs}
	COMMAND ${Qt5_LUPDATE_EXECUTABLE} -noobsolete ${Canorus_UIC_Srcs} ${Canorus_Srcs} -ts ${Canorus_Translation_Srcs} lang/template.ts
)
set_source_files_properties(${Canorus_Translation_Srcs} PROPERTIES OUTPUT_LOCATION ${CMAKE_CURRENT_BINARY_DIR}/lang)
qt5_add_translation(Canorus_Translation_Bins ${Canorus_Translation_Srcs})

ADD_CUSTOM_TARGET(canorus_qm DEPENDS ${Canorus_Translation_Bins})
ADD_DEPENDENCIES(canorus canorus_qm)

###############
# Dev Install #
###############
IF(${CANORUS_DEV_INSTALL} MATCHES True)
    ADD_CUSTOM_COMMAND(
        TARGET canorus
        POST_BUILD
        COMMAND ${CMAKE_MAKE_PROGRAM} install
    )
ENDIF(${CANORUS_DEV_INSTALL} MATCHES True)

################
# Installation #
################
# Install canorus exe and CanorusRuby lib
INSTALL(TARGETS canorus
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}
)
IF(USE_RUBY)
        INSTALL(TARGETS ${SWIG_MODULE_CanorusRuby_REAL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
        INSTALL(TARGETS ${SWIG_MODULE_CanorusRuby_REAL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
ENDIF(USE_RUBY)
IF(USE_PYTHON)
	INSTALL(TARGETS ${SWIG_MODULE_CanorusPython_REAL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/CanorusPython.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_LIB_DIR})
	INSTALL(TARGETS ${SWIG_MODULE_CanorusPython_REAL_NAME} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
	INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/CanorusPython.py DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR})
ENDIF(USE_PYTHON)

# Install scripts
INSTALL( DIRECTORY ${Canorus_Scripts} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}" )
# Install plugins
INSTALL( DIRECTORY ${Canorus_Plugins} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}" )
# Install data - icons, images, sounds etc.
INSTALL( DIRECTORY ${Canorus_Images} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}" )
# Install translations
INSTALL(FILES ${Canorus_Translation_Bins} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/lang")
# Install fonts
INSTALL(FILES ${Canorus_Fonts} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/fonts")
# Install users manual, if it exists
INSTALL(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/doc)")
INSTALL(CODE "FILE(MAKE_DIRECTORY ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/doc/usersguide)")
FILE(GLOB Canorus_Usersguide RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ../doc/usersguide/*.qhc ../doc/usersguide/*.qch)
INSTALL(FILES ${Canorus_Usersguide} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}/doc/usersguide")
# Install examples
INSTALL( DIRECTORY ${Canorus_Examples} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_DATA_DIR}" )

IF(MINGW)
	# Install Qt libs
        INSTALL( FILES ${QT_BIN_DIR}/Qt5Core.dll ${QT_BIN_DIR}/Qt5Gui.dll ${QT_BIN_DIR}/Qt5Widgets.dll ${QT_BIN_DIR}/Qt5Svg.dll ${QT_BIN_DIR}/Qt5Xml.dll ${QT_BIN_DIR}/Qt5Sql.dll ${QT_BIN_DIR}/Qt5Network.dll ${QT_BIN_DIR}/Qt5PrintSupport.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	
	# Install qsvg plugin for rendering SVG icons
	get_target_property(QSvgIcon_loc Qt5::QSvgIconPlugin LOCATION)
	INSTALL( FILES ${QSvgIcon_loc} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}/iconengines" )
	
	# Install qwindows plugin for Microsoft Windows integration
	get_target_property(QWindows_loc Qt5::QWindowsIntegrationPlugin LOCATION)
	INSTALL( FILES ${QWindows_loc} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}/platforms" )
	
	# Install dlls
	INSTALL( FILES ${MINGW_BIN_DIR}/libgcc_s_dw2-1.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	INSTALL( FILES ${MINGW_BIN_DIR}/libwinpthread-1.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	INSTALL( FILES ${MINGW_BIN_DIR}/libstdc++-6.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	IF("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
		INSTALL( FILES ${MINGW_BIN_DIR}/libssp-0.dll DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )
	ENDIF("${CMAKE_BUILD_TYPE}" MATCHES "Debug")
	
	# Install Python libs
	IF(USE_PYTHON)
		INSTALL( FILES ${PYTHON_LIBRARY} DESTINATION ${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR} )             # PythonXY.dll
		INSTALL( DIRECTORY ${PYTHON_LIB} DESTINATION "${CMAKE_INSTALL_PREFIX}/${CANORUS_INSTALL_BIN_DIR}/pythonLib" ) # Python .py files
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

##########################
# Automatic code styling #
##########################
# Applies code style formatting.
ADD_CUSTOM_TARGET(fmt
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	COMMAND clang-format -i -style=Webkit ${Canorus_Fmt_Srcs}
)

