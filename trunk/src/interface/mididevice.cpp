/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/mididevice.h"
#include "core/sheet.h"
#include "core/voice.h"

/*!
	\var CAMidiDevice::GM_INSTRUMENTS
	Human names for General Midi Instruments.
*/
const QStringList CAMidiDevice::GM_INSTRUMENTS = QStringList() <<
	CAMidiDevice::tr("Acoustic Grand Piano", "instrument") <<
	CAMidiDevice::tr("Bright Acoustic Piano", "instrument") <<
	CAMidiDevice::tr("Electric Grand Piano", "instrument") <<
	CAMidiDevice::tr("Honky-tonk Piano", "instrument") <<
	CAMidiDevice::tr("Electric Piano 1", "instrument") <<
	CAMidiDevice::tr("Electric Piano 2", "instrument") <<
	CAMidiDevice::tr("Harpsichord", "instrument") <<
	CAMidiDevice::tr("Clavi", "instrument") <<
	CAMidiDevice::tr("Celesta", "instrument") <<
	CAMidiDevice::tr("Glockenspiel", "instrument") <<
	CAMidiDevice::tr("Music Box", "instrument") <<
	CAMidiDevice::tr("Vibraphone", "instrument") <<
	CAMidiDevice::tr("Marimba", "instrument") <<
	CAMidiDevice::tr("Xylophone", "instrument") <<
	CAMidiDevice::tr("Tubular Bells", "instrument") <<
	CAMidiDevice::tr("Dulcimer", "instrument") <<
	CAMidiDevice::tr("Drawbar Organ", "instrument") <<
	CAMidiDevice::tr("Percussive Organ", "instrument") <<
	CAMidiDevice::tr("Rock Organ", "instrument") <<
	CAMidiDevice::tr("Church Organ", "instrument") <<
	CAMidiDevice::tr("Reed Organ", "instrument") <<
	CAMidiDevice::tr("Accordion", "instrument") <<
	CAMidiDevice::tr("Harmonica", "instrument") <<
	CAMidiDevice::tr("Tango Accordion", "instrument") <<
	CAMidiDevice::tr("Acoustic Guitar (nylon)", "instrument") <<
	CAMidiDevice::tr("Acoustic Guitar (steel)", "instrument") <<
	CAMidiDevice::tr("Electric Guitar (jazz)", "instrument") <<
	CAMidiDevice::tr("Electric Guitar (clean)", "instrument") <<
	CAMidiDevice::tr("Electric Guitar (muted)", "instrument") <<
	CAMidiDevice::tr("Overdriven Guitar", "instrument") <<
	CAMidiDevice::tr("Distortion Guitar", "instrument") <<
	CAMidiDevice::tr("Guitar harmonics", "instrument") <<
	CAMidiDevice::tr("Acoustic Bass", "instrument") <<
	CAMidiDevice::tr("Electric Bass (finger)", "instrument") <<
	CAMidiDevice::tr("Electric Bass (pick)", "instrument") <<
	CAMidiDevice::tr("Fretless Bass", "instrument") <<
	CAMidiDevice::tr("Slap Bass 1", "instrument") <<
	CAMidiDevice::tr("Slap Bass 2", "instrument") <<
	CAMidiDevice::tr("Synth Bass 1", "instrument") <<
	CAMidiDevice::tr("Synth Bass 2", "instrument") <<
	CAMidiDevice::tr("Violin", "instrument") <<
	CAMidiDevice::tr("Viola", "instrument") <<
	CAMidiDevice::tr("Cello", "instrument") <<
	CAMidiDevice::tr("Contrabass", "instrument") <<
	CAMidiDevice::tr("Tremolo Strings", "instrument") <<
	CAMidiDevice::tr("Pizzicato Strings", "instrument") <<
	CAMidiDevice::tr("Orchestral Harp", "instrument") <<
	CAMidiDevice::tr("Timpani", "instrument") <<
	CAMidiDevice::tr("String Ensemble 1", "instrument") <<
	CAMidiDevice::tr("String Ensemble 2", "instrument") <<
	CAMidiDevice::tr("SynthStrings 1", "instrument") <<
	CAMidiDevice::tr("SynthStrings 2", "instrument") <<
	CAMidiDevice::tr("Choir Aahs", "instrument") <<
	CAMidiDevice::tr("Voice Oohs", "instrument") <<
	CAMidiDevice::tr("Synth Voice", "instrument") <<
	CAMidiDevice::tr("Orchestra Hit", "instrument") <<
	CAMidiDevice::tr("Trumpet", "instrument") <<
	CAMidiDevice::tr("Trombone", "instrument") <<
	CAMidiDevice::tr("Tuba", "instrument") <<
	CAMidiDevice::tr("Muted Trumpet", "instrument") <<
	CAMidiDevice::tr("French Horn", "instrument") <<
	CAMidiDevice::tr("Brass Section", "instrument") <<
	CAMidiDevice::tr("SynthBrass 1", "instrument") <<
	CAMidiDevice::tr("SynthBrass 2", "instrument") <<
	CAMidiDevice::tr("Soprano Sax", "instrument") <<
	CAMidiDevice::tr("Alto Sax", "instrument") <<
	CAMidiDevice::tr("Tenor Sax", "instrument") <<
	CAMidiDevice::tr("Baritone Sax", "instrument") <<
	CAMidiDevice::tr("Oboe", "instrument") <<
	CAMidiDevice::tr("English Horn", "instrument") <<
	CAMidiDevice::tr("Bassoon", "instrument") <<
	CAMidiDevice::tr("Clarinet", "instrument") <<
	CAMidiDevice::tr("Piccolo", "instrument") <<
	CAMidiDevice::tr("Flute", "instrument") <<
	CAMidiDevice::tr("Recorder", "instrument") <<
	CAMidiDevice::tr("Pan Flute", "instrument") <<
	CAMidiDevice::tr("Blown Bottle", "instrument") <<
	CAMidiDevice::tr("Shakuhachi", "instrument") <<
	CAMidiDevice::tr("Whistle", "instrument") <<
	CAMidiDevice::tr("Ocarina", "instrument") <<
	CAMidiDevice::tr("Lead 1 (square)", "instrument") <<
	CAMidiDevice::tr("Lead 2 (sawtooth)", "instrument") <<
	CAMidiDevice::tr("Lead 3 (calliope)", "instrument") <<
	CAMidiDevice::tr("Lead 4 (chiff)", "instrument") <<
	CAMidiDevice::tr("Lead 5 (charang)", "instrument") <<
	CAMidiDevice::tr("Lead 6 (voice)", "instrument") <<
	CAMidiDevice::tr("Lead 7 (fifths)", "instrument") <<
	CAMidiDevice::tr("Lead 8 (bass + lead)", "instrument") <<
	CAMidiDevice::tr("Pad 1 (new age)", "instrument") <<
	CAMidiDevice::tr("Pad 2 (warm)", "instrument") <<
	CAMidiDevice::tr("Pad 3 (polysynth)", "instrument") <<
	CAMidiDevice::tr("Pad 4 (choir)", "instrument") <<
	CAMidiDevice::tr("Pad 5 (bowed)", "instrument") <<
	CAMidiDevice::tr("Pad 6 (metallic)", "instrument") <<
	CAMidiDevice::tr("Pad 7 (halo)", "instrument") <<
	CAMidiDevice::tr("Pad 8 (sweep)", "instrument") <<
	CAMidiDevice::tr("FX 1 (rain)", "instrument") <<
	CAMidiDevice::tr("FX 2 (soundtrack)", "instrument") <<
	CAMidiDevice::tr("FX 3 (crystal)", "instrument") <<
	CAMidiDevice::tr("FX 4 (atmosphere)", "instrument") <<
	CAMidiDevice::tr("FX 5 (brightness)", "instrument") <<
	CAMidiDevice::tr("FX 6 (goblins)", "instrument") <<
	CAMidiDevice::tr("FX 7 (echoes)", "instrument") <<
	CAMidiDevice::tr("FX 8 (sci-fi)", "instrument") <<
	CAMidiDevice::tr("Sitar", "instrument") <<
	CAMidiDevice::tr("Banjo", "instrument") <<
	CAMidiDevice::tr("Shamisen", "instrument") <<
	CAMidiDevice::tr("Koto", "instrument") <<
	CAMidiDevice::tr("Kalimba", "instrument") <<
	CAMidiDevice::tr("Bag pipe", "instrument") <<
	CAMidiDevice::tr("Fiddle", "instrument") <<
	CAMidiDevice::tr("Shanai", "instrument") <<
	CAMidiDevice::tr("Tinkle Bell", "instrument") <<
	CAMidiDevice::tr("Agogo", "instrument") <<
	CAMidiDevice::tr("Steel Drums", "instrument") <<
	CAMidiDevice::tr("Woodblock", "instrument") <<
	CAMidiDevice::tr("Taiko Drum", "instrument") <<
	CAMidiDevice::tr("Melodic Tom", "instrument") <<
	CAMidiDevice::tr("Synth Drum", "instrument") <<
	CAMidiDevice::tr("Reverse Cymbal", "instrument") <<
	CAMidiDevice::tr("Guitar Fret Noise", "instrument") <<
	CAMidiDevice::tr("Breath Noise", "instrument") <<
	CAMidiDevice::tr("Seashore", "instrument") <<
	CAMidiDevice::tr("Bird Tweet", "instrument") <<
	CAMidiDevice::tr("Telephone Ring", "instrument") <<
	CAMidiDevice::tr("Helicopter", "instrument") <<
	CAMidiDevice::tr("Applause", "instrument") <<
	CAMidiDevice::tr("Gunshot", "instrument");

/*!
	\class CAMidiDevice
	\brief Canorus<->MIDI bridge.
	
	This class represents generic MIDI interface to Canorus.
	Any MIDI wrapper class should extend this class. Canorus is aware only of this class.
	
	eg. In theory RtMidi is only one of the MIDI libraries implemented.
	If, in future, we decide to implement any other MIDI libraries, CAMidiDevice should
	mask the change and the core application shouldn't change at all.
*/

/*!
	Returns the first midi channel that isn't occupied by voices in the given sheet \a s yet.
	Returns 0, if all the channels are occupied.
	
	\warning This function never returns midi channel 10 as it's reserved for percussion instruments only.
*/
unsigned char CAMidiDevice::freeMidiChannel( CASheet* s ) {
	QList<CAVoice*> voices = s->voiceList();
	for (unsigned char i=0; i<16; i++) {
		int j=0;
		while (j<voices.size() && voices[j]->midiChannel()!=i)
			j++;
		
		if (j==voices.size() && i!=9)
			return i;
	}
	
	return 0;
}
