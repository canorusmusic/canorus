#/*!
#   Copyright (c) 2007, Matevž Jekovec, Georg Rudolph, Canorus development team
#   All Rights Reserved. See AUTHORS for a complete list of authors.
#
#   Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
#*/
#
#
# For parsing midi files canorus uses pmidi.
#
# With this makefile you can build on Linux the original pmidi program,
# for your reference. 
#
# The source is version 1.6.0, taken from an openSuse 11.0 package, with the two
# patches applied, also from there, which you find here in this directory too.
#
# The sequencer controlling part is not needed here, and due to the clean design
# of pmidi it was relatively easy not to compile it and leave it out.
# The parsed midi events are passed through the big switch statement in
# play() in pmidi.c. This pass through is now done accordingly in wrapper.c.
#
#	These file make the brigde to canorus:
#
#	wrapper.h
#	wrapper.c
#
#	These files of pmidi are compiled in unmodified:
#
#	elements.c
#	except.c
#	glib.c
#	mdutil.c
#	midiread.c
#
# Thanks to Steve Ratcliffe!
#


# Target to build the complete, original pmidi:
all:
	gcc -g -D VERSION=1.6 -D HAVE_ALSA_ASOUNDLIB_H=1 -D USE_DRAIN=1 -Wl,-lasound -o pmidi \
	elements.c \
	except.c \
	glib.c \
	mdutil.c \
	midiread.c \
	pmidi.c \
	seqlib.c \
	seqmidi.c

#
