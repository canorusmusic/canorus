/*!
	Copyright (c) 2008-2022, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/transpose.h"

#include "score/chordname.h"
#include "score/chordnamecontext.h"
#include "score/diatonicpitch.h"
#include "score/functionmark.h"
#include "score/functionmarkcontext.h"
#include "score/keysignature.h"
#include "score/muselement.h"
#include "score/note.h"
#include "score/sheet.h"
#include "score/staff.h"
#include "score/voice.h"

/*!
	\class CATranspose
	\brief Class used for transposing a set of notes for the given interval

	This is a control class used for making a transposition of a set of music elements,
	contexts or even the whole score sheet.

	Use:
	1) Create a CATranspose class
	2) Pass the music elements you want to transpose in the constructor or by calling
	   addMusElement() or addContext().
	3) Transpose the elements by calling transposeByKeySig(), transposeByInterval(),
	   transposeBySemitones() or reinterpretAccidentals().

	\sa CAInterval::fromSemitones()
 */

CATranspose::CATranspose()
{
}

CATranspose::CATranspose(CASheet* sheet)
{
    for (int i = 0; i < sheet->contextList().size(); i++) {
        addContext(sheet->contextList()[i]);
    }
}

CATranspose::CATranspose(QList<CAContext*> contexts)
{
    for (int i = 0; i < contexts.size(); i++) {
        addContext(contexts[i]);
    }
}

CATranspose::CATranspose(QList<CAMusElement*> selection)
{
    _elements = QSet<CAMusElement*>::fromList(selection);
}

CATranspose::~CATranspose()
{
}

void CATranspose::addSheet(CASheet* s)
{
    for (int i = 0; i < s->contextList().size(); i++) {
        addContext(s->contextList()[i]);
    }
}

void CATranspose::addContext(CAContext* context)
{
    switch (context->contextType()) {
    case CAContext::Staff: {
        CAStaff* staff = static_cast<CAStaff*>(context);
        for (int j = 0; j < staff->voiceList().size(); j++) {
            _elements.unite(QSet<CAMusElement*>::fromList(staff->voiceList()[j]->musElementList()));
        }
        break;
    }
    case CAContext::FunctionMarkContext: {
        QList<CAFunctionMark*> markList = static_cast<CAFunctionMarkContext*>(context)->functionMarkList();
        for (int i = 0; i < markList.size(); i++) {
            addMusElement(markList[i]);
        }
        break;
    }
    case CAContext::ChordNameContext: {
        QList<CAChordName*> cnList = static_cast<CAChordNameContext*>(context)->chordNameList();
        for (int i = 0; i < cnList.size(); i++) {
            addMusElement(cnList[i]);
        }
        break;
    }
    default:
        break;
    }
}

/*!
	Transposes the music elements by the given number of semitones.
	Number can be negative for the direction down.
 */
void CATranspose::transposeBySemitones(int semitones)
{
    transposeByInterval(CAInterval::fromSemitones(semitones));
}

/*!
	Transposes the music elements from the given key signature \a from to the key signature
	\a to in the given \a direction. Direction can be 1 for up or -1 for down.
 */
void CATranspose::transposeByKeySig(CADiatonicKey from, CADiatonicKey to, int direction)
{
    CAInterval interval(from.diatonicPitch(), to.diatonicPitch());
    if (((direction < 0) && (to.diatonicPitch().noteName() - from.diatonicPitch().noteName() > 0)) || ((direction > 0) && (to.diatonicPitch().noteName() - from.diatonicPitch().noteName() < 0))) {
        interval = ~interval;
    }

    if (direction < 0) {
        interval.setQuantity(interval.quantity() * (-1));
    }

    transposeByInterval(interval); /// \todo Notes should be transposed differently when transposing from major -> minor for example
}

/*!
	Transposes the music elements by the given interval.
	If the interval quantity is negative, elements are transposed down.
 */
void CATranspose::transposeByInterval(CAInterval interval)
{
    for (CAMusElement* elt : _elements) {
        switch (elt->musElementType()) {
        case CAMusElement::Note:
            static_cast<CANote*>(elt)->setDiatonicPitch(static_cast<CANote*>(elt)->diatonicPitch() + interval);
            break;
        case CAMusElement::KeySignature:
            static_cast<CAKeySignature*>(elt)->setDiatonicKey(static_cast<CAKeySignature*>(elt)->diatonicKey() + interval);
            break;
        case CAMusElement::ChordName:
            *static_cast<CAChordName*>(elt)+=(interval);
            break;
        case CAMusElement::FunctionMark:
            static_cast<CAFunctionMark*>(elt)->setKey(static_cast<CAFunctionMark*>(elt)->key() + interval);
            break;
        case CAMusElement::MidiNote: // ToDo
            break;
        default:
            break;
        }
    }
}

/*!
	Changes note accidentals dependent on \a type:
	1) If type==1, sharps -> flats
	2) If type==-1, flats -> sharps
	3) if type==0, invert

	This function also changes Key signatures dependent on \a type, if their
	number of accidentals is greater or equal than 5 or lesser or equal than -5.
*/
void CATranspose::reinterpretAccidentals(int type)
{
    for (CAMusElement* elt : _elements) {
        switch (elt->musElementType()) {
        case CAMusElement::Note:
        case CAMusElement::ChordName: {
            CADiatonicPitch newPitch;
            if (elt->musElementType() == CAMusElement::Note) {
                newPitch = static_cast<CANote*>(elt)->diatonicPitch();
            } else if (elt->musElementType() == CAMusElement::ChordName) {
                newPitch = static_cast<CAChordName*>(elt)->diatonicPitch();
            }

            if (type >= 0 && newPitch.accs() > 0) {
                newPitch = newPitch + CAInterval(-2, 2);
            } else if (type <= 0 && newPitch.accs() < 0) {
                newPitch = newPitch - CAInterval(-2, 2);
            }

            if (elt->musElementType() == CAMusElement::Note) {
                static_cast<CANote*>(elt)->setDiatonicPitch(newPitch);
            } else if (elt->musElementType() == CAMusElement::ChordName) {
                static_cast<CAChordName*>(elt)->setDiatonicPitch(newPitch);
            }

            break;
        }
        case CAMusElement::KeySignature: {
            CAKeySignature* keySig = static_cast<CAKeySignature*>(elt);
            CADiatonicKey newDiatonicKey = keySig->diatonicKey();
            if (type >= 0 && keySig->diatonicKey().numberOfAccs() >= 5) {
                newDiatonicKey = CADiatonicKey(keySig->diatonicKey().diatonicPitch() + CAInterval(-2, 2), keySig->diatonicKey().gender());
            } else if (type <= 0 && keySig->diatonicKey().numberOfAccs() <= -5) {
                newDiatonicKey = CADiatonicKey(keySig->diatonicKey().diatonicPitch() - CAInterval(-2, 2), keySig->diatonicKey().gender());
            }
            keySig->setDiatonicKey(newDiatonicKey);
            break;
        }
        default:
            break;
        }
    }
}
