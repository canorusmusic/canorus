/*!
	Copyright (c) 2008-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/diatonicpitch.h"
#include "score/diatonickey.h"

/*!
	\class CADiatonicPitch
	\brief Musical note pitch

	This is a typical music presentation of the note pitch.

	It consists of two properties:
	- note name (C-0, D-1, E-2 etc.)
	- accidentals (0-neutral, 1-one sharp, -1-one flat etc.)

	Note name begins with sub-contra C.

	Diatonic pitches can be compared with each other or only note names
	and summed with intervals.

	\sa CAInterval, operator+(), CAMidiDevice::diatonicPitchToMidi()
*/

CADiatonicPitch::CADiatonicPitch()
{
    setNoteName(Undefined);
    setAccs(0);
}

CADiatonicPitch::CADiatonicPitch(const QString& pitch)
{
    setNoteName(Undefined);
    setAccs(0);

    QString noteName = pitch.toLower();
    if (noteName[0].toLatin1() - 'a' > 6) {
        // syntax error
        return;
    }

    int curPitch = (noteName[0].toLatin1() - 'a' + 5) % 7;
    if (!noteName.startsWith("as") && !noteName.startsWith("es"))
        noteName.remove(0, 1); // remove one-letter note name

    // determine accidentals
    int curAccs = 0;
    while (noteName.indexOf("is") != -1) {
        curAccs++;
        noteName.remove(0, noteName.indexOf("is") + 2);
    }
    while ((noteName.indexOf("es") != -1) || (noteName.indexOf("as") != -1)) {
        curAccs--;
        noteName.remove(0, ((noteName.indexOf("es") == -1) ? (noteName.indexOf("as") + 2) : (noteName.indexOf("es") + 2)));
    }

    if (noteName.size() > 0) {
        // syntax error
        return;
    }

    setNoteName(curPitch);
    setAccs(curAccs);
}

CADiatonicPitch::CADiatonicPitch(const int& noteName, const int& accs)
{
    setNoteName(noteName);
    setAccs(accs);
}

bool CADiatonicPitch::operator==(CADiatonicPitch p)
{
    if (noteName() == p.noteName() && accs() == p.accs())
        return true;
    else
        return false;
}

bool CADiatonicPitch::operator==(int p)
{
    if (noteName() == p)
        return true;
    else
        return false;
}

/*!
	Converts the music pitch to string.
*/
const QString CADiatonicPitch::diatonicPitchToString(CADiatonicPitch pitch)
{
    if (pitch == Undefined) {
        return "";
    }

    QString name;

    name = static_cast<char>((pitch.noteName() + 2) % 7 + 'a');

    for (int i = 0; i < pitch.accs(); i++)
        name += "is"; // append as many -is-es as necessary

    for (int i = 0; i > pitch.accs(); i--) {
        if ((name == "e") || (name == "a"))
            name += "s"; // for pitches E and A, only append single -s the first time
        else if (name[0] == 'a')
            name += "as"; // for pitch A, append -as instead of -es
        else
            name += "es"; // otherwise, append normally as many es-es as necessary
    }

    return name;
}

/*!
	Calculates the new pitch using the old pitch + interval.
*/
CADiatonicPitch CADiatonicPitch::operator+(CAInterval i)
{
    CADiatonicPitch dp(noteName(), accs());

    // Only use positive intervals UP. If the interval is negative, inverse it:
    if (i.quantity() < 0) {
        if (i.quantity() != -1) {
            // First lower the pitch for i octaves + 1,
            dp.setNoteName(dp.noteName() + ((i.quantity() - 5) / 7) * 7);
        } else {
            // The exception is the negative prime (which actually doesn't exist, but we have to take care of it)
            dp.setNoteName(dp.noteName() - 7);
        }
        // then inverse the interval. (negative prime becomes octave)
        i = ~i;
        // Below, the positive interval is now added to the lowered note.
    }

    dp.setNoteName(dp.noteName() + i.quantity() - 1);
    int deltaAccs = 0;

    int relP = noteName() % 7;
    int relQnt = ((i.quantity() - 1) % 7) + 1;
    switch (relQnt) { // major or perfect intervals up:
    case CAInterval::Prime:
        deltaAccs = 0;
        break;
    case CAInterval::Second:
        if (relP == 2 || relP == 6)
            deltaAccs = 1;
        else
            deltaAccs = 0;
        break;
    case CAInterval::Third:
        if (relP == 0 || relP == 3 || relP == 4)
            deltaAccs = 0;
        else
            deltaAccs = 1;
        break;
    case CAInterval::Fourth:
        if (relP == 3)
            deltaAccs = -1;
        else
            deltaAccs = 0;
        break;
    case CAInterval::Fifth:
        if (relP == 6)
            deltaAccs = 1;
        else
            deltaAccs = 0;
        break;
    case CAInterval::Sixth:
        if (relP == 2 || relP == 5 || relP == 6)
            deltaAccs = 1;
        else
            deltaAccs = 0;
        break;
    case CAInterval::Seventh:
        if (relP == 0 || relP == 3)
            deltaAccs = 0;
        else
            deltaAccs = 1;
        break;
    }

    if (relQnt == 4 || relQnt == 5 || relQnt == 1) {
        if (i.quality() < 0)
            dp.setAccs(deltaAccs + dp.accs() + i.quality() + 1);
        else if (i.quality() > 0)
            dp.setAccs(deltaAccs + dp.accs() + i.quality() - 1);
        else
            dp.setAccs(deltaAccs + dp.accs());
    } else {
        if (i.quality() < 0)
            dp.setAccs(deltaAccs + dp.accs() + i.quality());
        else if (i.quality() > 0)
            dp.setAccs(deltaAccs + dp.accs() + i.quality() - 1);
    }

    return dp;
}

/*!
	Creates a new diatonic pitch from the given string.
*/
CADiatonicPitch CADiatonicPitch::diatonicPitchFromString(const QString s)
{
    return CADiatonicPitch(s);
}

/*!
	This function is provided for convenience.
	
	\sa diatonicPitchFromMidiPitchKey()
*/
CADiatonicPitch CADiatonicPitch::diatonicPitchFromMidiPitch(int midiPitch, CAMidiPitchMode mode)
{
    return diatonicPitchFromMidiPitchKey(midiPitch, CADiatonicKey("C"), mode);
}

/*!
	Generates prefered diatonic pitch for the given \a midiPitch.
	\a key tells generally which accidentals and note pitches to use.
	\a mode tells which accidental is the preferred one, if the diatonic pitch
	is unknown to the given key. If the auto mode is set, then C-major degrees
	C#, D#, F#, G# and Bb are taken for the given key. These are the only
	possible major thirds or minor sevenths of the Dominant chords of degrees.

	\sa CAMidiPitchMode, diatonicPitchToMidiPitch
*/
CADiatonicPitch CADiatonicPitch::diatonicPitchFromMidiPitchKey(int midiPitch, CADiatonicKey key, CAMidiPitchMode mode)
{
    int notePitch = 0, accs = 0;

    double step = 7 / 12.0;

    int octave = midiPitch / 12 - 1;
    int rest = midiPitch % 12;
    // calculate pitch to be a white key or a black key with sharp
    CADiatonicPitch p;
    p.setNoteName(qRound(step * static_cast<double>(rest) - 0.5 + 1.0 / 7 + octave * 7));
    p.setAccs((diatonicPitchToMidiPitch(p) % 12) == rest ? 0 : 1);

    // return the pitch, if it's natural to the key
    if (key.containsPitch(p)) {
        return p;
    }

    // try the degree lower / higher and adjust accidentals to have the same pitch
    CADiatonicPitch pLower = p - CAInterval(CAInterval::Diminished, CAInterval::Second);
    if (key.containsPitch(pLower)) {
        return pLower;
    }
    CADiatonicPitch pHigher = p + CAInterval(CAInterval::Diminished, CAInterval::Second);
    if (key.containsPitch(pHigher)) {
        return pHigher;
    }

    // pitch not found naturally in the key, set the diatonic pitch according to the mode parameter
    // first transpose the pitch to C-Major
    CAInterval transposeInterval(CADiatonicPitch(0, 0), key.diatonicPitch());
    if (key.gender() == CADiatonicKey::Minor) {
        transposeInterval = transposeInterval + CAInterval(CAInterval::Major, CAInterval::Sixth); // interpret minor keys as major
    }
    transposeInterval.setQuantity(((transposeInterval.quantity() - 1) % 7) + 1);
    octave = (midiPitch - transposeInterval.semitones()) / 12 - 1; // redefine octave with new key base
    switch ((midiPitch + (12 - transposeInterval.semitones())) % 12) {
    case 0:
        notePitch = 0;
        accs = 0;
        break;

    case 1:
        switch (mode) {
        case CADiatonicPitch::PreferAuto:
        case CADiatonicPitch::PreferSharps:
            notePitch = 0;
            accs = 1;
            break;
        case CADiatonicPitch::PreferFlats:
            notePitch = 1;
            accs = -1;
            break;
        }
        break;

    case 2:
        notePitch = 1;
        accs = 0;
        break;

    case 3:
        switch (mode) {
        case CADiatonicPitch::PreferAuto:
        case CADiatonicPitch::PreferSharps:
            notePitch = 1;
            accs = 1;
            break;
        case CADiatonicPitch::PreferFlats:
            notePitch = 2;
            accs = -1;
            break;
        }
        break;

    case 4:
        notePitch = 2;
        accs = 0;
        break;

    case 5:
        notePitch = 3;
        accs = 0;
        break;

    case 6:
        switch (mode) {
        case CADiatonicPitch::PreferAuto:
        case CADiatonicPitch::PreferSharps:
            notePitch = 3;
            accs = 1;
            break;
        case CADiatonicPitch::PreferFlats:
            notePitch = 4;
            accs = -1;
            break;
        }
        break;

    case 7:
        notePitch = 4;
        accs = 0;
        break;

    case 8:
        switch (mode) {
        case CADiatonicPitch::PreferAuto:
        case CADiatonicPitch::PreferSharps:
            notePitch = 4;
            accs = 1;
            break;
        case CADiatonicPitch::PreferFlats:
            notePitch = 5;
            accs = -1;
            break;
        }
        break;

    case 9:
        notePitch = 5;
        accs = 0;
        break;

    case 10:
        switch (mode) {
        case CADiatonicPitch::PreferSharps:
            notePitch = 5;
            accs = 1;
            break;
        case CADiatonicPitch::PreferAuto:
        case CADiatonicPitch::PreferFlats:
            notePitch = 6;
            accs = -1;
            break;
        }
        break;

    case 11:
        notePitch = 6;
        accs = 0;
        break;
    }

    notePitch += octave * 7;

    return CADiatonicPitch(notePitch, accs) + transposeInterval;
}

/*!
	Converts the given diatonic pitch (note name with octave and accidental) to
	standard unsigned 7-bit MIDI pitch.
*/
int CADiatonicPitch::diatonicPitchToMidiPitch(const CADiatonicPitch& pitch)
{
    // +0.3 - rounding factor for 7/12 that exactly underlays every tone in octave, if rounded
    // +12 - our logical pitch starts at Sub-contra C, midi counting starts one octave lower
    return qRound(pitch.noteName() * (12 / 7.0) + 0.3 + 12) + pitch.accs();
}
