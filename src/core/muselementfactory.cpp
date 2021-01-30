/*!
	Copyright (c) 2006-2020, Reinhard Katzmann, Matevž Jekovec, Canorus development team

	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/muselementfactory.h"
#include "score/articulation.h"
#include "score/bookmark.h"
#include "score/chordnamecontext.h"
#include "score/dynamic.h"
#include "score/figuredbasscontext.h"
#include "score/functionmarkcontext.h"
#include "score/instrumentchange.h"
#include "score/playable.h"
#include "score/sheet.h"
#include "score/text.h"
#include "score/tuplet.h"

#include "canorus.h" // needed for CASettings
#include "core/settings.h"

/*!
	\class CAMusElementFactory
	\brief creation, removal, configuration of music elements

	This class is used when creating a new music element, settings its values, but not having it
	placed inside the score (eg. music element still in the GUI, but user didn't clicked on the score
	to place it yet).

	The factory contains the methods to set properties of all the available music elements ranging from
	playable elements to slurs and lyrics. The actual music element is created and added to the scene
	when one of the configure functions are called. These functions return True, if the element was
	successfully created or False otherwise.

	This class should be typically used in the following order:
	  1) User selects a music element to place. CAMusElementFactory::setMusElementType() is called.
	  2) GUI for music element properties is updated according to the current factory values.
	  3) User sets music element properties he wants (eg. number of accidentals in key signature).
	     CAMusElementFactory::setSomeMusicElementProperty( music element property value ) is called
	     (eg. CAMusElementFactory::setKeySigNumberOfAccs(3) for A-Major/fis-minor).
	  4) User places the music element. CAMusElementFactory::configureSomeMusicElement()
	     (eg. CAMusElementFactory::configureKeySignature(currentStaff, prevElement)).
	  5) If the configure function returns True, the GUI is refreshed and the note is drawn.

	Changing the properties of already placed elements (eg. in edit mode) is done without using the factory.

	\sa CAMainWin, CAMusElement
*/

/*!
	Creates an empty, defeault music elements factory.
*/
CAMusElementFactory::CAMusElementFactory()
{
    _playableLength = CAPlayableLength(CAPlayableLength::Quarter);
    _eNoteStemDirection = CANote::StemPreferred;
    _iPlayableDotted = 0;
    _eRestType = CARest::Normal;
    _iTimeSigBeats = 4;
    _iTimeSigBeat = 4;
    _diatonicKeyNumberOfAccs = 0;
    _diatonicKeyGender = CADiatonicKey::Major;
    _eClef = CAClef::Treble;
    _iClefOffset = 0;
    _iNoteAccs = 0;
    _iNoteExtraAccs = 0;
    _eBarlineType = CABarline::Single;
    _eSlurType = CASlur::TieType;
    _slurStyle = CASlur::SlurSolid;

    _fbmNumber = 6;
    _fbmAccs = 0;
    _fbmAccsVisible = false;

    _fmFunction = CAFunctionMark::T; // Name of the function
    _fmChordArea = CAFunctionMark::Undefined; // Chord area of the function
    _fmTonicDegree = CAFunctionMark::T; // Tonic degree of the function
    _fmFunctionMinor = false;
    _fmChordAreaMinor = false;
    _fmTonicDegreeMinor = false;
    _fmEllipse = false;
    _musElementType = CAMusElement::Undefined;
    mpoEmpty = std::make_shared<CANote>(CADiatonicPitch(), CAPlayableLength(), nullptr, 0); // dummy element
    mpoMusElement = mpoEmpty;

    _dynamicText = "mf";
    _dynamicVolume = 80;
    _instrument = 0;

    _fermataType = CAFermata::NormalFermata;
    _tempoBeat = CAPlayableLength(CAPlayableLength::Half);
    _tempoBpm = 72;

    _crescendoFinalVolume = 50;
    _crescendoType = CACrescendo::Crescendo;

    _repeatMarkType = CARepeatMark::Volta;
    _repeatMarkVoltaNumber = 1;

    _fingeringFinger = CAFingering::First;
    _fingeringOriginal = false;
}

/*!
	Destroys the music elements factory.
*/
CAMusElementFactory::~CAMusElementFactory()
{
    removeMusElem(true);
    mpoEmpty.reset();
}

/*!
	Removes the current music element.
	Destroys the music element, if \a bReallyRemove is true (default is false).
*/
void CAMusElementFactory::removeMusElem(bool bReallyRemove /* = false */)
{
    if (mpoMusElement && mpoMusElement != mpoEmpty && bReallyRemove)
        mpoMusElement.reset();

    mpoMusElement = mpoEmpty;
}

void CAMusElementFactory::addPlayableDotted(int add, CAPlayableLength l)
{
    _playableLength.setDotted((_playableLength.dotted() + add) % 4);
    // FIXME: magic number 4 is max. number of flags.
    // If 128th will be visible in the gui, _playableLength.dotted() could be one higher.
    //
    // the more flags a to be inserted note has, the less dots are allowed:
    //
    switch (l.musicLength()) {
    case CAPlayableLength::Eighth:
        if (_playableLength.dotted() > 3)
            _playableLength.setDotted(0);
        break;
    case CAPlayableLength::Sixteenth:
        if (_playableLength.dotted() > 2)
            _playableLength.setDotted(0);
        break;
    case CAPlayableLength::ThirtySecond:
        if (_playableLength.dotted() > 1)
            _playableLength.setDotted(0);
        break;
    case CAPlayableLength::SixtyFourth:
        if (_playableLength.dotted() > 0)
            _playableLength.setDotted(0);
        break;
    default:
        break;
        ;
    }
};

/*!
	Configures a new clef music element in \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureClef(CAStaff* staff,
    CAMusElement* right)
{
    bool success = false;
    if (staff && staff->voiceList().size()) {
        mpoMusElement = std::make_shared<CAClef>(_eClef, staff, 0, _iClefOffset);
        success = staff->voiceList()[0]->insert(right, mpoMusElement.get());
        if (!success)
            removeMusElem(true);
    }
    return success;
}

/*!
	Configures a new key signature music element with \a iKeySignature accidentals, \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureKeySignature(CAStaff* staff,
    CAMusElement* right)
{
    bool success = false;
    if (staff && staff->voiceList().size()) {
        mpoMusElement = std::make_shared<CAKeySignature>(CADiatonicKey(_diatonicKeyNumberOfAccs, _diatonicKeyGender),
            staff,
            0);
        success = staff->voiceList()[0]->insert(right, mpoMusElement.get());
        if (!success)
            removeMusElem(true);
    }
    return success;
}

/*!
	Configures a new time signature music element with \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureTimeSignature(CAStaff* staff,
    CAMusElement* right)
{
    bool success = false;
    if (staff && staff->voiceList().size()) {
        mpoMusElement = std::make_shared<CATimeSignature>(_iTimeSigBeats, _iTimeSigBeat,
            staff,
            0);
        success = staff->voiceList()[0]->insert(right, mpoMusElement.get());
        if (!success)
            removeMusElem(true);
    }
    return success;
}

/*!
	Configures a new barline with \a context and right before the \a right element.
*/
bool CAMusElementFactory::configureBarline(CAStaff* staff,
    CAMusElement* right)
{
    bool success = false;
    if (staff && staff->voiceList().size()) {
        mpoMusElement = std::make_shared<CABarline>(_eBarlineType,
            staff,
            0);
        success = staff->voiceList()[0]->insert(right, mpoMusElement.get());
        if (!success)
            removeMusElem(true);
    }
    return success;
}

/*!
	Configures new note music element in the \a voice before element \a right.
	If \a addToChord is true and right is note, the element is added to the note instead of inserted.
*/
bool CAMusElementFactory::configureNote(int pitch,
    CAVoice* voice,
    CAMusElement* right,
    bool addToChord)
{
    bool bSuccess = false;
    removeMusElem();

    if (right && addToChord) {
        mpoMusElement = std::make_shared<CANote>(CADiatonicPitch(pitch, _iNoteAccs),
            static_cast<CANote*>(right)->playableLength(),
            voice,
            0, // timeStart is set when inserting to voice
            static_cast<CANote*>(right)->timeLength());

        bSuccess = voice->insert(right, mpoMusElement.get(), true);
        if (static_cast<CANote*>(right)->tuplet()) {
            static_cast<CANote*>(right)->tuplet()->addNote(static_cast<CAPlayable*>(mpoMusElement.get()));
            static_cast<CANote*>(mpoMusElement.get())->setTuplet(static_cast<CANote*>(right)->tuplet());
        }
    } else {
        mpoMusElement = std::make_shared<CANote>(CADiatonicPitch(pitch, _iNoteAccs),
            _playableLength,
            voice,
            0);

        // add an empty syllable or reposit syllables
        static_cast<CANote*>(mpoMusElement.get())->setStemDirection(_eNoteStemDirection);
        bSuccess = voice->insert(right, mpoMusElement.get(), false);

        if (voice->lastNote() == mpoMusElement.get()) {
            // note was appended, reposition elements in dependent contexts accordingly
            for (CALyricsContext* lc : voice->lyricsContextList()) {
                lc->repositSyllables();
            }
            for (CAContext* context : voice->staff()->sheet()->contextList()) {
                switch (context->contextType()) {
                case CAContext::FunctionMarkContext:
                    static_cast<CAFunctionMarkContext*>(context)->repositFunctions();
                    break;
                case CAContext::FiguredBassContext:
                    static_cast<CAFiguredBassContext*>(context)->repositFiguredBassMarks();
                    break;
                case CAContext::ChordNameContext:
                    static_cast<CAChordNameContext*>(context)->repositChordNames();
                    break;
                default:
                    break;
                }
            }
        } else {
            // note was inserted somewhere inbetween, insert empty element in dependent contexts accordingly
            for (CALyricsContext* lc : voice->lyricsContextList()) {
                lc->addEmptySyllable(mpoMusElement->timeStart(), mpoMusElement->timeLength());
            }
            for (CAContext* context : voice->staff()->sheet()->contextList()) {
                switch (context->contextType()) {
                case CAContext::FunctionMarkContext:
                    static_cast<CAFunctionMarkContext*>(context)->addEmptyFunction(
                        mpoMusElement->timeStart(), mpoMusElement->timeLength());
                    break;
                case CAContext::FiguredBassContext:
                    static_cast<CAFiguredBassContext*>(context)->addEmptyFiguredBassMark(
                        mpoMusElement->timeStart(), mpoMusElement->timeLength());
                    break;
                case CAContext::ChordNameContext:
                    static_cast<CAChordNameContext*>(context)->addEmptyChordName(
                        mpoMusElement->timeStart(), mpoMusElement->timeLength());
                    break;
                default:
                    break;
                }
            }
        }
    }

    if (bSuccess) {
        static_cast<CANote*>(mpoMusElement.get())->updateTies();
    } else
        removeMusElem(true);

    return bSuccess;
}

/*!
	Configures the new tie, slur or phrasing slur for the notes \a noteStart and \a noteEnd.
*/
bool CAMusElementFactory::configureSlur(CAStaff* staff,
    CANote* noteStart, CANote* noteEnd)
{
    bool success = false;
    removeMusElem();
    auto slur = std::make_shared<CASlur>(slurType(), CASlur::SlurPreferred, staff, noteStart, noteEnd);
    mpoMusElement = slur;

    slur->setSlurStyle(slurStyle());
    switch (slurType()) {
    case CASlur::TieType:
        noteStart->setTieStart(slur.get());
        if (noteEnd)
            noteEnd->setTieEnd(slur.get());
        success = true;
        break;
    case CASlur::SlurType:
        noteStart->setSlurStart(slur.get());
        if (noteEnd)
            noteEnd->setSlurEnd(slur.get());
        success = true;
        break;
    case CASlur::PhrasingSlurType:
        noteStart->setPhrasingSlurStart(slur.get());
        if (noteEnd)
            noteEnd->setPhrasingSlurEnd(slur.get());
        success = true;
        break;
    }

    if (!success)
        removeMusElem(true);

    return success;
}

/*!
	Configures the new mark and adds it to the \a elt.
*/
bool CAMusElementFactory::configureMark(CAMusElement* elt)
{
    bool success = false;

    if (elt->musElementType() == CAMusElement::Mark)
        return false;

    switch (markType()) {
    case CAMark::Dynamic: {
        if (elt->musElementType() == CAMusElement::Note) {
            mpoMusElement = std::make_shared<CADynamic>(dynamicText(), dynamicVolume(), static_cast<CANote*>(elt));
            success = true;
        }
        break;
    }
    case CAMark::Crescendo: {
        if (elt->musElementType() == CAMusElement::Note) {
            mpoMusElement = std::make_shared<CACrescendo>(crescendoFinalVolume(), static_cast<CANote*>(elt), crescendoType());
            success = true;
        }
        break;
    }
    case CAMark::Text: {
        if (elt->isPlayable()) {
            mpoMusElement = std::make_shared<CAText>("", static_cast<CAPlayable*>(elt));
            success = true;
        }
        break;
    }
    case CAMark::BookMark: {
        mpoMusElement = std::make_shared<CABookMark>("", elt);
        success = true;
        break;
    }
    case CAMark::InstrumentChange: {
        if (elt->musElementType() == CAMusElement::Note) {
            mpoMusElement = std::make_shared<CAInstrumentChange>(instrument(), static_cast<CANote*>(elt));
            success = true;
        }
        break;
    }
    case CAMark::Tempo: {
        mpoMusElement = std::make_shared<CATempo>(tempoBeat(), tempoBpm(), elt);
        success = true;
        break;
    }
    case CAMark::Ritardando: {
        if (elt->isPlayable()) {
            mpoMusElement = std::make_shared<CARitardando>(50, static_cast<CAPlayable*>(elt), elt->timeLength() * 2, ritardandoType());
            success = true;
        }
        break;
    }
    case CAMark::Pedal: {
        mpoMusElement = std::make_shared<CAMark>(CAMark::Pedal, elt);
        success = true;
        break;
    }
    case CAMark::Fermata: {
        if (elt->isPlayable()) {
            mpoMusElement = std::make_shared<CAFermata>(static_cast<CAPlayable*>(elt), fermataType());
            success = true;
        } else if (elt->musElementType() == CAMusElement::Barline) {
            mpoMusElement = std::make_shared<CAFermata>(static_cast<CABarline*>(elt), fermataType());
            success = true;
        }
        break;
    }
    case CAMark::RepeatMark: {
        if (elt->musElementType() == CAMusElement::Barline) {
            mpoMusElement = std::make_shared<CARepeatMark>(static_cast<CABarline*>(elt), repeatMarkType(), (repeatMarkType() == CARepeatMark::Volta ? repeatMarkVoltaNumber() : 0));
            success = true;
        }
        break;
    }
    case CAMark::RehersalMark: {
        if (elt->musElementType() == CAMusElement::Barline) {
            mpoMusElement = std::make_shared<CAMark>(CAMark::RehersalMark, elt);
            success = true;
        }
        break;
    }
    case CAMark::Fingering: {
        if (elt->musElementType() == CAMusElement::Note) {
            mpoMusElement = std::make_shared<CAFingering>(fingeringFinger(), static_cast<CANote*>(elt), isFingeringOriginal());
            success = true;
        }
        break;
    }
    case CAMark::Articulation: {
        if (elt->musElementType() == CAMusElement::Note) {
            mpoMusElement = std::make_shared<CAArticulation>(articulationType(), static_cast<CANote*>(elt));
            success = true;
        }
        break;
    }
    case CAMark::Undefined: {
        fprintf(stderr, "Warning: CAMusElementFactory::configureMark - Undefined Mark");
        break;
    }
    }

    if (success) {
        elt->addMark(static_cast<CAMark*>(mpoMusElement.get()));
        return true;
    }

    return false;
}

/*!
	Configures a new rest music element in voice \a voice before element \a right.
*/
bool CAMusElementFactory::configureRest(CAVoice* voice, CAMusElement* right)
{
    bool success = false;
    if (voice) {
        mpoMusElement = std::make_shared<CARest>(restType(),
            _playableLength,
            voice,
            0);
        success = voice->insert(right, mpoMusElement.get());

        if (!success)
            removeMusElem(true);
        else {
            for (CALyricsContext* lc : voice->lyricsContextList()) {
                lc->repositSyllables();
            }
            for (CAContext* context : voice->staff()->sheet()->contextList()) {
                switch (context->contextType()) {
                case CAContext::ChordNameContext:
                    static_cast<CAChordNameContext*>(context)->repositChordNames();
                    break;
                default:
                    break;
                }
            }
        }
    }
    return success;
}

/*!
	Configures a new figured bass mark with \a timeStart and \a timeLength in context \a fbc.
*/
bool CAMusElementFactory::configureFiguredBassNumber(CAFiguredBassMark* fbm)
{
    if ((_fbmNumber == 0 && (!_fbmAccsVisible)) || (!fbm)) {
        return false;
    }

    if (_fbmAccsVisible) {
        fbm->addNumber(_fbmNumber, _fbmAccs);
    } else {
        fbm->addNumber(_fbmNumber);
    }
    mpoMusElement = std::shared_ptr<CAFiguredBassMark>(fbm);

    return true;
}

/*!
	Configures a new function mark with \a timeStart and \a timeLength in context \a fmc.
*/
bool CAMusElementFactory::configureFunctionMark(CAFunctionMarkContext* fmc, int timeStart, int timeLength)
{
    CAFunctionMark* fm = new CAFunctionMark(
        fmFunction(), isFMFunctionMinor(),
        CADiatonicKey::diatonicKeyToString(CADiatonicKey(_diatonicKeyNumberOfAccs, _diatonicKeyGender)),
        fmc, timeStart, timeLength,
        fmChordArea(), isFMChordAreaMinor(),
        fmTonicDegree(), isFMTonicDegreeMinor(),
        "", /// \todo Function mark altered/added degrees
        isFMEllipse());

    fmc->addFunctionMark(fm);
    mpoMusElement = std::shared_ptr<CAFunctionMark>(fm);

    return true;
}

/*!
	Configures a new tuplet containing the given \a noteList.
 */
bool CAMusElementFactory::configureTuplet(QList<CAPlayable*>)
{
    return false;
}

/*!
	\fn CAMusElementFactory::musElement()
	Reads the current music element and returns its pointer.
*/

/*!
	\fn CAMusElementFactory::musElementType()
	Returns the current music element type.
*/

/*!
	\fn CAMusElementFactory::setMusElementType(CAMusElement::CAMusElementType eMEType)
	Sets the new current music element type \a eMEType, does not create a new element of this type!
*/

/*!
	\var CAMusElementFactory::mpoMusElement
	Newly created music element itself.

	\sa musElement()
*/

/*!
	\var CAMusElementFactory::_ePlayableLength
	Length of note/rest to be added.
*/

/*!
	\var CAMusElementFactory::_iPlayableDotted
	Number of dots to be inserted for the note/rest.
*/

/*!
	\var CAMusElementFactory::_iNoteExtraAccs
	Extra note accidentals for new notes which user adds/removes with +/- keys.
*/

/*!
	\var CAMusElementFactory::_iNoteAccs
	Note accidentals at specific coordinates updated regularily when in insert mode.
*/

/*!
	\var CAMusElementFactory::_iTimeSigBeats
	Time signature number of beats to be inserted.
*/

/*!
	\var CAMusElementFactory::_iTimeSigBeat
	Time signature beat to be inserted.
*/

/*!
	\var CAMusElementFactory::_eClef
	Type of the clef to be inserted.
*/
