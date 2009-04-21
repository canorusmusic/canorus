
canorus_version:
	gcc -g -D HAVE_ALSA_ASOUNDLIB_H=1 \
	glib.c \
	elements.c \
	except.c \
	mdutil.c \
	midiread.c \
	out_seqmidi.c \
	canorus.c
	./a.out
	


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


