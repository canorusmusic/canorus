/*!
	Copyright (c) 2006-2020, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/note.h"
#include "score/clef.h"
#include "score/mark.h"
#include "score/staff.h"
#include "score/voice.h"

/*!
	\class CANote
	\brief Represents a note in the score.

	This class represents every note in the score. It inherits the base class CAPlayable.

	Chords in Canorus don't have its own class, but are represented as a list of notes.
	The first note in the chord identifies it and contains chord-level marks.
*/

/*!
	Creates a new note with playable length \a length in voice \a voice with pitch \a pitch
	and accidentals \a accs, with starting time in the score \a timeStart and number of dots \a dotted.
	timeLength is calculated automatically from the playable length.

	\sa CAPlayableLength, CAPlayable, CAVoice, _pitch, _accs
*/
CANote::CANote(CADiatonicPitch pitch, CAPlayableLength length, CAVoice* voice, int timeStart, int timeLength)
    : CAPlayable(length, voice, timeStart, timeLength)
{
    _musElementType = CAMusElement::Note;
    _forceAccidentals = false;
    _stemDirection = StemPreferred;

    setTieStart(nullptr);
    setSlurStart(nullptr);
    setPhrasingSlurStart(nullptr);
    setTieEnd(nullptr);
    setSlurEnd(nullptr);
    setPhrasingSlurEnd(nullptr);

    setDiatonicPitch(pitch);
}

CANote::~CANote()
{
    if (tieEnd())
        tieEnd()->setNoteEnd(nullptr);

    if (tieStart())
        setTieStart(nullptr);

    // other slurs are removed or moved in CAVoice::removeElement()

    clearMarkList();
}

/*!
	\enum CANote::CAStemDirection
	\brief Direction of the note's stem

	This type represents the direction of the note's stem.
	\todo Stem itself will probably be created as a separate class in the future. This will give us
	possibility for a chord to have a common stem, apply additional stem properties etc. -Matevz

	Possible values:
		- StemNeutral
			Up if under the middle line, down if above the middle line.
		- StemUp
			Always up.
		- StemDown
			Always down.
		- StemPreferred
			Use the voice's preferred direction.
*/

/*!
	Clones the note with same pitch, voice, timeStart and other properties.
	Does *not* create clones of ties, slurs and phrasing slurs!
*/
std::shared_ptr<CAPlayable> CANote::clonePlayable(CAVoice* voice)
{
    auto d = std::make_shared<CANote>(diatonicPitch(), playableLength(), voice, timeStart(), timeLength());
    d->setStemDirection(stemDirection());

    for (int i = 0; i < markList().size(); i++) {
        auto m = (markList()[i]->cloneMark(d.get()));
        d->addMark(m.get());
    }

    return d;
}

/*!
	Dependent on the current clef calculates and returns the vertical
	note position in the staff looking bottom-up:
	-  0 - first line
	-  1 - first space
	-  2 - second line
	- -1 - first space below the first line
	- -2 - first ledger line below the staff (eg. C1 in treble clef)
*/
int CANote::notePosition()
{
    CAClef* clef = nullptr;
    if (voice() && voice()->staff()) {
        // find the corresponding clef
        int i = 0;
        while (i < voice()->staff()->clefRefs().size() && staff()->clefRefs()[i]->timeStart() <= timeStart()) {
            i++;
        }
        i--;

        if (i >= 0) {
            clef = static_cast<CAClef*>(voice()->staff()->clefRefs()[i]);
        }
    }

    return (diatonicPitch().noteName() + (clef ? clef->c1() : -2) - 28);
}

/*!
	Generates the note name on the given pitch \a pitch with accidentals \a accs.
	Note name ranges are from C,, for sub-contra octave to c''''' for fifth octave.
	This method is usually used for showing the note pitch in status bar.

	\sa _pitch, _accs
*/
const QString CANote::generateNoteName(int pitch, int accs)
{
    QString name;

    name = static_cast<char>(((pitch < 0 ? pitch * (-1) : pitch) + 2) % 7 + 'a');
    while (accs > 0) {
        name += "is";
        accs--;
    }
    while (accs < 0) {
        if (name != "e" && name != "a")
            name += "es";
        else
            name += "s";
        accs++;
    }

    if (pitch < 21)
        name = name.toUpper();

    for (int i = 0; i < (pitch - 21) / 7; i++)
        name.append('\'');

    for (int i = 0; i < (pitch - 20) / (-7); i++)
        name.append(',');

    return name;
}

/*!
	Returns true, if the note is part of a chord; otherwise false.
*/
bool CANote::isPartOfChord()
{
    int idx = voice()->musElementList().indexOf(this);

    // is there a note with the same start time after ours?
    if (idx + 1 < voice()->musElementList().size() && voice()->musElementList()[idx + 1]->musElementType() == CAMusElement::Note && voice()->musElementList()[idx + 1]->timeStart() == _timeStart)
        return true;

    // is there a note with the same start time before ours?
    if (idx > 0 && voice()->musElementList()[idx - 1]->musElementType() == CAMusElement::Note && voice()->musElementList()[idx - 1]->timeStart() == _timeStart)
        return true;

    return false;
}

/*!
	Returns true, if the note is the first in the list of the chord; otherwise false.
*/
bool CANote::isFirstInChord()
{
    int idx = voice()->musElementList().indexOf(this);

    //is there a note with the same start time before ours?
    if (idx > 0 && voice()->musElementList()[idx - 1]->musElementType() == CAMusElement::Note && voice()->musElementList()[idx - 1]->timeStart() == _timeStart)
        return false;

    return true;
}

/*!
	Returns true, if the note is the last in the list of the chord; otherwise false.
*/
bool CANote::isLastInChord()
{
    int idx = voice()->musElementList().indexOf(this);

    //is there a note with the same start time after ours?
    if (idx + 1 < voice()->musElementList().size() && voice()->musElementList()[idx + 1]->musElementType() == CAMusElement::Note && voice()->musElementList()[idx + 1]->timeStart() == _timeStart)
        return false;

    return true;
}

/*!
	Generates a list of notes with the same start time - the whole chord - in the current voice.
	Notes in chord keep the order present in the voice. This is usually bottom-up.

	Returns a single element in the list - only the note itself, if the note isn't part of the chord.

	\sa CAVoice::addNoteToChord()
*/
QList<CANote*> CANote::getChord()
{
    QList<CANote*> list;
    int idx = voice()->musElementList().indexOf(this) - 1;

    while (idx >= 0 && voice()->musElementList()[idx]->musElementType() == CAMusElement::Note && voice()->musElementList()[idx]->timeStart() == timeStart())
        idx--;

    for (idx++;
         (idx >= 0 && idx < voice()->musElementList().size()) && (voice()->musElementList()[idx]->musElementType() == CAMusElement::Note) && (voice()->musElementList()[idx]->timeStart() == timeStart());
         idx++)
        list << static_cast<CANote*>(voice()->musElementList()[idx]);

    return list;
}

int CANote::compare(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::Note)
        return -1;

    int diffs = 0;
    if (diatonicPitch() != static_cast<CANote*>(elt)->diatonicPitch())
        diffs++;
    if (playableLength() != static_cast<CAPlayable*>(elt)->playableLength())
        diffs++;

    return diffs;
}

/*!
	Sets the stem direction and update tie, slur and phrasing slur direction.
*/
void CANote::setStemDirection(CAStemDirection dir)
{
    _stemDirection = dir;
}

/*!
	Looks at the tieStart() and tieEnd() ties and unties the note and tie if the
	previous/next note pitch differs.
*/
void CANote::updateTies()
{
    // break the tie, if needed
    if (tieStart() && tieStart()->noteEnd() && diatonicPitch() != tieStart()->noteEnd()->diatonicPitch()) {
        // break the tie, if the first note isn't the same pitch
        tieStart()->noteEnd()->setTieEnd(nullptr);
        tieStart()->setNoteEnd(nullptr);
    }
    if (tieEnd() && tieEnd()->noteStart() && diatonicPitch() != tieEnd()->noteStart()->diatonicPitch()) {
        // break the tie, if the next note isn't the same pitch
        tieEnd()->setNoteEnd(nullptr);
        setTieEnd(nullptr);
    }

    // fix/create a tie, if needed
    QList<CANote*> noteList;
    if (voice())
        noteList = voice()->getNoteList();

    // checks a tie of the potential left note
    CANote* leftNote = nullptr;
    for (int i = 0; i < noteList.count() && noteList[i]->timeEnd() <= timeStart(); i++) { // get the left note
        if (noteList[i]->timeEnd() == timeStart() && noteList[i]->diatonicPitch() == diatonicPitch()) {
            leftNote = noteList[i];
            break;
        }
    }
    if (leftNote && leftNote->tieStart()) {
        leftNote->tieStart()->setNoteEnd(this);
        setTieEnd(leftNote->tieStart());
    }

    // checks a tie of the potential right note
    CANote* rightNote = nullptr;
    for (int i = 0; i < noteList.count() && noteList[i]->timeStart() <= timeEnd(); i++) { // get the right note
        if (noteList[i]->timeStart() == timeEnd() && noteList[i]->diatonicPitch() == diatonicPitch()) {
            rightNote = noteList[i];
            break;
        }
    }
    if (rightNote && tieStart()) {
        rightNote->setTieEnd(tieStart());
        tieStart()->setNoteEnd(rightNote);
    }
}

/*!
	Converts stem direction CAStemDirection to QString.
	This is usually used when saving the score.

	\sa CAStemDirection, CACanorusML
*/
const QString CANote::stemDirectionToString(CANote::CAStemDirection dir)
{
    switch (dir) {
    case CANote::StemUp:
        return "stem-up";
    case CANote::StemDown:
        return "stem-down";
    case CANote::StemNeutral:
        return "stem-neutral";
    case CANote::StemPreferred:
        return "stem-preferred";
    case CANote::StemUndefined:
        return "stem-preferred";
    }
    return "stem-preferred";
}

/*!
	Converts stem direction in QString format to CAStemDirection.
	This is usually used when loading a score.

	\sa CAStemDirection, CACanorusML
*/
CANote::CAStemDirection CANote::stemDirectionFromString(const QString dir)
{
    if (dir == "stem-up") {
        return CANote::StemUp;
    } else if (dir == "stem-down") {
        return CANote::StemDown;
    } else if (dir == "stem-neutral") {
        return CANote::StemNeutral;
    } else if (dir == "stem-preferred") {
        return CANote::StemPreferred;
    } else
        return CANote::StemPreferred;
}

/*!
	Returns the actual stem direction (the one which is drawn). Always returns stem up or stem down.
*/
CANote::CAStemDirection CANote::actualStemDirection()
{
    switch (stemDirection()) {
    case StemUp:
    case StemDown:
        return stemDirection();

    case StemNeutral:
        if (staff() && notePosition() < staff()->numberOfLines() - 1) // position from 0 to half of the number of lines - where position has step of 2 per line
            return StemUp;
        else
            return StemDown;

    case StemPreferred:
        if (!voice()) {
            return StemUp;
        }

        switch (voice()->stemDirection()) {
        case StemUp:
        case StemDown:
            return voice()->stemDirection();

        case StemNeutral:
            if (staff() && notePosition() < staff()->numberOfLines() - 1) // position from 0 to half of the number of lines - where position has step of 2 per line
                return StemUp;
            else
                return StemDown;
        default: // We should always have a defined stem here
            return StemUndefined;
        }
    default:
        return StemUndefined;
    }
}

/*!
	Determines the right slur direction of the note.
	Slur should be on the other side of the stem, if the stem direction is neutral
	or on the same side if the stem direction is set strictly to up and down (or preferred).
*/
CASlur::CASlurDirection CANote::actualSlurDirection()
{
    CAStemDirection dir = actualStemDirection();

    if (stemDirection() == StemNeutral || (stemDirection() == StemPreferred && voice() && voice()->stemDirection() == StemNeutral)) {
        if (dir == StemUp)
            return CASlur::SlurDown;
        else
            return CASlur::SlurUp;
    } else {
        if (dir == StemUp)
            return CASlur::SlurUp;
        else
            return CASlur::SlurDown;
    }
}

/*!
	\fn CANote::accidentals()
	Returns number and type of accidentals of the note.

	\sa _accs, setAccidentals()
*/

/*!
	\fn CANote::setAccidentals(int accs)
	Sets a number and type of accidentals to \a accs of the note.

	\sa _accs, accidentals()
*/

/*!
	\var CANote::_accs
	Number of note accidentals:
		- 0 - neutral
		- 1 - sharp
		- -1 - flat etc.

	\sa accidentals(), setAccidentals()
*/

/*!
	\fn CANote::pitch()
	Returns the note pitch in internal Canorus units.

	\sa _pitch, setPitch()
*/

/*!
	\fn CANote::setPitch(int pitch)
	Sets the note pitch to \a pitch.

	\sa _pitch, pitch()
*/

/*!
	\var CANote::_pitch
	Pitch in internal Canorus format.
		- 0 - sub-contra C
		- 1 - sub-contra D
		- 28 - middle C
		- 56 - c5 etc.

	\sa pitch(), setPitch(), midiPitchToPitch(), pitchToMidiPitch(), pitchToString(), generateNoteName()
*/

/*!
	\var CANote::_stemDirection
	Direction of the note's stem, if any.

	\sa CAStemDirection
*/

/*!
	\var CANote::_forceAccidentals
	Always draw notes accidentals.

	\sa _accs
*/
