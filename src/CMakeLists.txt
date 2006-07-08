# Define Sources of this Canorus directory

# This module is used to resolve the file dependencies
# for QT files using an UI (User Unterface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -g")

# This line adds the include path of the Qt
# directory to the compiler include path
INCLUDE( ${QT_USE_FILE} )

# Define the UI source files used by Canorus
set(Canorus_UIs  mainwin.ui)

# Define the MOC source files used by Canorus
set(Canorus_MOCs mainwin.h scrollwidget.h viewport.h)

# Add all sources to Canorus_Srcs
set(Canorus_Srcs mainwin.cpp
	main.cpp
	kdtree.cpp
	drawable.cpp
	note.cpp
	scrollwidget.cpp
	viewport.cpp)

# Add all resources to Canorus_Resrcs ((icons, html files etc.)
set(Canorus_Resrcs mainwin.qrc)

# Add all the dependencies needed for the MainWindow
# add_file_dependencies( mainwin.cpp ui_mainwin.h )

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
                        ${Canorus_MOC_Srcs} ${Canorus_Resrcs_Srcs})

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
target_link_libraries( canorus ${QT_LIBRARIES} )
