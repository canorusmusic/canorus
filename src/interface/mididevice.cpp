/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "interface/mididevice.h"
#include "core/sheet.h"
#include "core/voice.h"

CAMidiDevice::CAMidiDevice()
 : QObject()
{
	GM_INSTRUMENTS = QStringList() <<
	QObject::tr("Acoustic Grand Piano", "instrument") <<
	QObject::tr("Bright Acoustic Piano", "instrument") <<
	QObject::tr("Electric Grand Piano", "instrument") <<
	QObject::tr("Honky-tonk Piano", "instrument") <<
	QObject::tr("Electric Piano 1", "instrument") <<
	QObject::tr("Electric Piano 2", "instrument") <<
	QObject::tr("Harpsichord", "instrument") <<
	QObject::tr("Clavi", "instrument") <<
	QObject::tr("Celesta", "instrument") <<
	QObject::tr("Glockenspiel", "instrument") <<
	QObject::tr("Music Box", "instrument") <<
	QObject::tr("Vibraphone", "instrument") <<
	QObject::tr("Marimba", "instrument") <<
	QObject::tr("Xylophone", "instrument") <<
	QObject::tr("Tubular Bells", "instrument") <<
	QObject::tr("Dulcimer", "instrument") <<
	QObject::tr("Drawbar Organ", "instrument") <<
	QObject::tr("Percussive Organ", "instrument") <<
	QObject::tr("Rock Organ", "instrument") <<
	QObject::tr("Church Organ", "instrument") <<
	QObject::tr("Reed Organ", "instrument") <<
	QObject::tr("Accordion", "instrument") <<
	QObject::tr("Harmonica", "instrument") <<
	QObject::tr("Tango Accordion", "instrument") <<
	QObject::tr("Acoustic Guitar (nylon)", "instrument") <<
	QObject::tr("Acoustic Guitar (steel)", "instrument") <<
	QObject::tr("Electric Guitar (jazz)", "instrument") <<
	QObject::tr("Electric Guitar (clean)", "instrument") <<
	QObject::tr("Electric Guitar (muted)", "instrument") <<
	QObject::tr("Overdriven Guitar", "instrument") <<
	QObject::tr("Distortion Guitar", "instrument") <<
	QObject::tr("Guitar harmonics", "instrument") <<
	QObject::tr("Acoustic Bass", "instrument") <<
	QObject::tr("Electric Bass (finger)", "instrument") <<
	QObject::tr("Electric Bass (pick)", "instrument") <<
	QObject::tr("Fretless Bass", "instrument") <<
	QObject::tr("Slap Bass 1", "instrument") <<
	QObject::tr("Slap Bass 2", "instrument") <<
	QObject::tr("Synth Bass 1", "instrument") <<
	QObject::tr("Synth Bass 2", "instrument") <<
	QObject::tr("Violin", "instrument") <<
	QObject::tr("Viola", "instrument") <<
	QObject::tr("Cello", "instrument") <<
	QObject::tr("Contrabass", "instrument") <<
	QObject::tr("Tremolo Strings", "instrument") <<
	QObject::tr("Pizzicato Strings", "instrument") <<
	QObject::tr("Orchestral Harp", "instrument") <<
	QObject::tr("Timpani", "instrument") <<
	QObject::tr("String Ensemble 1", "instrument") <<
	QObject::tr("String Ensemble 2", "instrument") <<
	QObject::tr("SynthStrings 1", "instrument") <<
	QObject::tr("SynthStrings 2", "instrument") <<
	QObject::tr("Choir Aahs", "instrument") <<
	QObject::tr("Voice Oohs", "instrument") <<
	QObject::tr("Synth Voice", "instrument") <<
	QObject::tr("Orchestra Hit", "instrument") <<
	QObject::tr("Trumpet", "instrument") <<
	QObject::tr("Trombone", "instrument") <<
	QObject::tr("Tuba", "instrument") <<
	QObject::tr("Muted Trumpet", "instrument") <<
	QObject::tr("French Horn", "instrument") <<
	QObject::tr("Brass Section", "instrument") <<
	QObject::tr("SynthBrass 1", "instrument") <<
	QObject::tr("SynthBrass 2", "instrument") <<
	QObject::tr("Soprano Sax", "instrument") <<
	QObject::tr("Alto Sax", "instrument") <<
	QObject::tr("Tenor Sax", "instrument") <<
	QObject::tr("Baritone Sax", "instrument") <<
	QObject::tr("Oboe", "instrument") <<
	QObject::tr("English Horn", "instrument") <<
	QObject::tr("Bassoon", "instrument") <<
	QObject::tr("Clarinet", "instrument") <<
	QObject::tr("Piccolo", "instrument") <<
	QObject::tr("Flute", "instrument") <<
	QObject::tr("Recorder", "instrument") <<
	QObject::tr("Pan Flute", "instrument") <<
	QObject::tr("Blown Bottle", "instrument") <<
	QObject::tr("Shakuhachi", "instrument") <<
	QObject::tr("Whistle", "instrument") <<
	QObject::tr("Ocarina", "instrument") <<
	QObject::tr("Lead 1 (square)", "instrument") <<
	QObject::tr("Lead 2 (sawtooth)", "instrument") <<
	QObject::tr("Lead 3 (calliope)", "instrument") <<
	QObject::tr("Lead 4 (chiff)", "instrument") <<
	QObject::tr("Lead 5 (charang)", "instrument") <<
	QObject::tr("Lead 6 (voice)", "instrument") <<
	QObject::tr("Lead 7 (fifths)", "instrument") <<
	QObject::tr("Lead 8 (bass + lead)", "instrument") <<
	QObject::tr("Pad 1 (new age)", "instrument") <<
	QObject::tr("Pad 2 (warm)", "instrument") <<
	QObject::tr("Pad 3 (polysynth)", "instrument") <<
	QObject::tr("Pad 4 (choir)", "instrument") <<
	QObject::tr("Pad 5 (bowed)", "instrument") <<
	QObject::tr("Pad 6 (metallic)", "instrument") <<
	QObject::tr("Pad 7 (halo)", "instrument") <<
	QObject::tr("Pad 8 (sweep)", "instrument") <<
	QObject::tr("FX 1 (rain)", "instrument") <<
	QObject::tr("FX 2 (soundtrack)", "instrument") <<
	QObject::tr("FX 3 (crystal)", "instrument") <<
	QObject::tr("FX 4 (atmosphere)", "instrument") <<
	QObject::tr("FX 5 (brightness)", "instrument") <<
	QObject::tr("FX 6 (goblins)", "instrument") <<
	QObject::tr("FX 7 (echoes)", "instrument") <<
	QObject::tr("FX 8 (sci-fi)", "instrument") <<
	QObject::tr("Sitar", "instrument") <<
	QObject::tr("Banjo", "instrument") <<
	QObject::tr("Shamisen", "instrument") <<
	QObject::tr("Koto", "instrument") <<
	QObject::tr("Kalimba", "instrument") <<
	QObject::tr("Bag pipe", "instrument") <<
	QObject::tr("Fiddle", "instrument") <<
	QObject::tr("Shanai", "instrument") <<
	QObject::tr("Tinkle Bell", "instrument") <<
	QObject::tr("Agogo", "instrument") <<
	QObject::tr("Steel Drums", "instrument") <<
	QObject::tr("Woodblock", "instrument") <<
	QObject::tr("Taiko Drum", "instrument") <<
	QObject::tr("Melodic Tom", "instrument") <<
	QObject::tr("Synth Drum", "instrument") <<
	QObject::tr("Reverse Cymbal", "instrument") <<
	QObject::tr("Guitar Fret Noise", "instrument") <<
	QObject::tr("Breath Noise", "instrument") <<
	QObject::tr("Seashore", "instrument") <<
	QObject::tr("Bird Tweet", "instrument") <<
	QObject::tr("Telephone Ring", "instrument") <<
	QObject::tr("Helicopter", "instrument") <<
	QObject::tr("Applause", "instrument") <<
	QObject::tr("Gunshot", "instrument");
}

/*!
	\class CAMidiDevice
	\brief Canorus<->MIDI bridge.
	
	This class represents generic MIDI interface to Canorus.
	Any MIDI wrapper class should extend this class. Canorus is aware only of this class.
	
	Currently only RtMidi is one of the MIDI libraries implemented. This class could be
	extended to implement any MIDI library.
	
	\warning MIDI INPUT is not available for Swig and therefore scripting languages yet.
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

/*!
	\var CAMidiDevice::GM_INSTRUMENTS
	Human names for General Midi Instruments.
*/
