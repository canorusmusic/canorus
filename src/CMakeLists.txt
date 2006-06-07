# Define Sources of this Canorus directory

# This module is used to resolve the file dependencies
# for QT files using an UI (User Unterface) Qt file
# created with the Qt4 designer
# INCLUDE(AddFileDependencies)

# This line adds the include path of the Qt
# directory to the compiler include path
INCLUDE( ${QT_USE_FILE} )

# Define the UI source files used by Canorus
set(Canorus_UIs  CAMainWin.ui)

# Define the MOC source files used by Canorus
set(Canorus_MOCs CAMainWin.h)

# Add all sources to Canorus_Srcs
set(Canorus_Srcs CAMainWin.cpp main.cpp)

# Add all the dependencies needed for the MainWindow
# add_file_dependencies( CAMainWin.cpp ui_CAMainWin.h )

# This scripts runs the user interface compiler.
# The result of it is stored into Canorus_UIC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_UI( Canorus_UIC_Srcs ${Canorus_UIs} )

# This script runs the meta object compiler.
# The result of it is stored into Canorus_MOC_Srcs
# variable. This has to be included later in the
# add_executable line.
QT4_WRAP_CPP( Canorus_MOC_Srcs ${Canorus_MOCs} )

# This line tells cmake to create the Canorus program.
# All dependent libraries like RtMidi must be added here.
add_executable(canorus ${Canorus_UIC_Srcs} ${Canorus_Srcs} ${Canorus_MOC_Srcs})

# Here the needed Qt Libraries are added to the link
# command. Never remove that line :-)
target_link_libraries( canorus ${QT_LIBRARIES} )
