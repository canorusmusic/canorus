/*!
	Copyright (c) 2019-2022, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/chordname.h"
#include "score/chordnamecontext.h"

/*!
	\class CAChordName
	\brief Chord name

	Chord name (e.g. C, F#m, Gsus4 etc.) inside the CAChordNameContext.
*/

CAChordName::CAChordName(CADiatonicPitch pitch, QString qualityModifier, CADiatonicPitch bassNote, CAChordNameContext* c, int timeStart, int timeLength)
    : CAMusElement(c, timeStart, timeLength)
{
    setMusElementType(ChordName);

    setDiatonicPitch(pitch);
    setQualityModifier(qualityModifier);
    setBassNote(bassNote);
}

CAChordName::~CAChordName()
{
}

/*!
    Clears the pitch and modifier.
    This function is usually called when deleting the chord name from the UI, where it shouldn't actually
    be removed, but only cleared.
*/
void CAChordName::clear()
{
    setDiatonicPitch(CADiatonicPitch::Undefined);
    setQualityModifier("");
    setBassNote(CADiatonicPitch::Undefined);
}

CAChordName* CAChordName::clone(CAContext* context)
{
    if (context && context->contextType() != CAContext::ChordNameContext) {
        return nullptr;
    }

    return new CAChordName(
        diatonicPitch(),
        qualityModifier(),
        bassNote(),
        static_cast<CAChordNameContext*>(context),
        timeStart(),
        timeLength());
}

int CAChordName::compare(CAMusElement* elt)
{
    if (elt->musElementType() != CAMusElement::ChordName)
        return -2;

    if (static_cast<CAChordName*>(elt)->diatonicPitch() != diatonicPitch())
        return 1;

    if (static_cast<CAChordName*>(elt)->qualityModifier() != qualityModifier())
        return 2;

    if (static_cast<CAChordName*>(elt)->bassNote() != bassNote())
        return 4;

    return 0;
}

/*!
    Transposes the chord for the given \a interval.
    The new pitch is correctly bounded.
 */
CAChordName& CAChordName::operator+=(CAInterval interval)
{
    if (diatonicPitch()!=CADiatonicPitch::Undefined) {
        CADiatonicPitch p = diatonicPitch() + interval;
        p.setNoteName(p.noteName() % 7);
        if (p.noteName() < 0) {
            p.setNoteName(p.noteName() + 7);
        }
        setDiatonicPitch(p);
    }

    if (bassNote()!=CADiatonicPitch::Undefined) {
        CADiatonicPitch p = bassNote() + interval;
        p.setNoteName(p.noteName() % 7);
        if (p.noteName() < 0) {
            p.setNoteName(p.noteName() + 7);
        }
        setBassNote(p);
    }

    return *this;
}


/*!
 * \brief CAChordName::importFromLilyPond parses lilypondish syntax for chord name and applies it
 * \param text chord name in lilypond syntax, for example "cis:m7/b"
 * \return True, if no errors encountered during parsing; False otherwise.
 */
bool CAChordName::importFromString(const QString& text)
{
    int bnd = text.indexOf('/');
    _bassNote = (bnd == -1) ? CADiatonicPitch() : CADiatonicPitch(text.mid(bnd+1));
    QString chordWithoutBassNote = (bnd == -1) ? text : text.left(bnd);

    int d = chordWithoutBassNote.indexOf(':');
    _diatonicPitch = CADiatonicPitch((d == -1) ? chordWithoutBassNote : chordWithoutBassNote.left(d));

    if (_diatonicPitch.noteName() == CADiatonicPitch::Undefined) {
        // syntax error
        _qualityModifier = chordWithoutBassNote;
        return false;
    }

    if (d != -1) {
        _qualityModifier = chordWithoutBassNote.mid(d + 1);
    } else {
        _qualityModifier = "";
    }

    return true;
}
