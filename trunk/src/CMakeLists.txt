# Define Sources of this Canorus directory

# This module is used to resolve the file dependencies
# for QT files using an UI (User Unterface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)


# Uncomment the 2nd line for M$ Windows and comment the Linux one:
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -D__LINUX_ALSASEQ__")
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g -D__WINDOWS_MM__")

# This line adds the include path of the Qt
# directory to the compiler include path
INCLUDE( ${QT_USE_FILE} )
# These two lines add additionally include pathes
# for xml and test includes
INCLUDE_DIRECTORIES( ${QT_QTXML_INCLUDE_DIR} )
INCLUDE_DIRECTORIES( ${QT_QTTEST_INCLUDE_DIR} )

# This line adds the QtXML library to the Qt libraries
# loaded when linking Canorus
set( QT_USE_QTXML true )

# Define the UI source files used by Canorus
set(Canorus_UIs  ui/mainwin.ui)

# Define the MOC source files used by Canorus
set(Canorus_MOCs ui/mainwin.h 
	widgets/lcdnumber.h 
	widgets/scrollwidget.h 
	widgets/viewport.h 
	widgets/scoreviewport.h 
	widgets/sourceviewport.h
	widgets/toolbar.h
	interface/playback.h
	)

# Add all sources to Canorus_Srcs
set(Canorus_Core_Srcs
	core/barline.cpp
	core/clef.cpp
	core/keysignature.cpp
	core/context.cpp
	core/document.cpp
	core/kdtree.cpp
	core/muselement.cpp
	core/note.cpp
	core/playable.cpp
	core/sheet.cpp
	core/staff.cpp
	core/voice.cpp
	
	core/canorusml.cpp
	)

set(Canorus_Widget_Srcs  
	widgets/lcdnumber.cpp
	widgets/scoreviewport.cpp
	widgets/sourceviewport.cpp
	widgets/scrollwidget.cpp
	widgets/toolbar.cpp
	widgets/viewport.cpp
	)

set(Canorus_Ui_Srcs ui/mainwin.cpp)

set(Canorus_Drawable_Srcs
	drawable/drawable.cpp
	drawable/drawablebarline.cpp
	drawable/drawableclef.cpp
	drawable/drawablekeysignature.cpp
	drawable/drawableaccidental.cpp
	drawable/drawablecontext.cpp
	drawable/drawablemuselement.cpp
	drawable/drawablenote.cpp
	drawable/drawablestaff.cpp
	)

set(Canorus_Interface_Srcs
	interface/engraver.cpp
	interface/playback.cpp
	interface/rtmididevice.cpp
	)

set(Canorus_RTMidi_Srcs
	rtmidi/RtMidi.cpp
	)

set(Canorus_Srcs 
	main.cpp
	${Canorus_Core_Srcs}
	${Canorus_Drawable_Srcs}
	${Canorus_Ui_Srcs}
	${Canorus_Interface_Srcs}
	${Canorus_RTMidi_Srcs}
	${Canorus_Widget_Srcs}
	)

# Add all resources to Canorus_Resrcs ((icons, html files etc.)
set(Canorus_Resrcs ui/mainwin.qrc)

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

# This line tells cmake to create the Canorus program.
# All dependent libraries like RtMidi must be added here.
# Attention: In contrast to Makefiles don't add "\" to separate lines
add_executable(canorus ${Canorus_UIC_Srcs}  ${Canorus_Srcs} 
                        ${Canorus_MOC_Srcs} ${Canorus_Resrcs_Srcs}
			)

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
# Add ${QT_QTTEST_LIBRARY} below to add the Qt Test library as well
# Uncomment the 2nd line for M$ Windows and comment the first Linux one:
target_link_libraries( canorus ${QT_LIBRARIES} ${QT_QTXML_LIBRARY} "asound" )
#target_link_libraries( canorus ${QT_LIBRARIES} "winmm.lib" )
