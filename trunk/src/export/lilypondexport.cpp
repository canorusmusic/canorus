/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QRegExp>
#include <QList>
#include <QFileInfo>
#include <QTextStream>

#include "export/lilypondexport.h"

#include "core/document.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/mark.h"
#include "core/text.h"
#include "core/tuplet.h"

/*!
	\class CALilyPondExport
	\brief LilyPond export filter
	This class is used to export the document or parts of the document to LilyPond syntax.
	The most common use is to simply call one of the constructors
	\code
	CALilyPondExport( myDocument, &textStream );
	\endcode

	\a textStream is usually the file stream or the content of the score source view widget.

	\sa CALilyPondImport
*/

/*!
	Constructor for the lilypond export. Called when viewing a single voice source in
	Lily syntax or exporting the whole document to a file.

	Uses \a out for the output.
*/
CALilyPondExport::CALilyPondExport( QTextStream *out )
 : CAExport(out) {
	setIndentLevel( 0 );
}

/*!
	Exports the given voice music elements to LilyPond syntax.

	\sa CALilypondImport
*/
void CALilyPondExport::exportVoiceImpl(CAVoice *v) {
	setCurVoice(v);

	_curStreamTime = 0;
	_lastPlayableLength = CAPlayableLength::Undefined;
	bool anacrusisCheck = true;	// process upbeat eventually
	CATimeSignature *time = 0;

	// Write \relative note for the first note
	_lastNotePitch = writeRelativeIntro();

	// start of the voice block
	out() << "{\n";
	indentMore();
	indent();

	for (int i=0; i<v->musElementCount(); i++, out() << " ") { // append blank after each element
		// (CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				// CAClef
				CAClef *clef = static_cast<CAClef*>(v->musElementAt(i));
				if (clef->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\clef \"";
				out() << clefTypeToLilyPond( clef->clefType(), clef->c1(), clef->offset() );
				out() << "\"";

				break;
			}
			case CAMusElement::KeySignature: {
				// CAKeySignature
				CAKeySignature *key = static_cast<CAKeySignature*>(v->musElementAt(i));
				if (key->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\key "
				    << diatonicPitchToLilyPond( key->diatonicKey().diatonicPitch() ) << " "
				    << diatonicKeyGenderToLilyPond( key->diatonicKey().gender() );

				break;
			}
			case CAMusElement::TimeSignature: {
				// CATimeSignature, remember for anacrusis processing
				time = static_cast<CATimeSignature*>(v->musElementAt(i));
				if (time->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				out() << "\\time " << time->beats() << "/" << time->beat();

				break;
			}
			case CAMusElement::Barline: {
				// CABarline
				CABarline *bar = static_cast<CABarline*>(v->musElementAt(i));
				if (bar->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
				if (bar->barlineType() == CABarline::Single)
					out() << "|";
				else
					out() << "\\bar \"" << barlineTypeToLilyPond(bar->barlineType()) << "\"";

				break;
			}
		}

		if ( v->musElementAt(i)->isPlayable() ) {

			if (anacrusisCheck) {			// first check upbeat bar, only once
				doAnacrusisCheck( time );
				anacrusisCheck = false;
			}
			exportPlayable( static_cast<CAPlayable*>(v->musElementAt(i)) );
		}

		exportMarks(v->musElementAt(i));
	}

	// end of the voice block
	indentLess();
	indent();
	out() << "\n}";
}

void CALilyPondExport::exportPlayable( CAPlayable *elt ) {
	if ( elt->isFirstInTuplet() ) {
		out() << "\\times " << elt->tuplet()->actualNumber() << "/" << elt->tuplet()->number() << " { ";
	}

	switch (elt->musElementType()) {
	case CAMusElement::Note: {
		// CANote
		CANote *note = static_cast<CANote*>(elt);
		if (note->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time
		if (note->isPartOfChord() && note->isFirstInChord()) {
			out() << "<";
		}

		// write the note name
		out() << relativePitchToString( note->diatonicPitch(), _lastNotePitch);

		if ( !note->isPartOfChord() && _lastPlayableLength != note->playableLength() ) {
			out() << playableLengthToLilyPond( note->playableLength() );
		}

		if (note->tieStart())
			out() << "~";

		_lastNotePitch = note->diatonicPitch();
		if (!note->isPartOfChord())
			_lastPlayableLength = note->playableLength();

		// finish the chord stanza if that's the last note of the chord
		if (note->isPartOfChord() && note->isLastInChord()) {
			out() << ">";

			if ( _lastPlayableLength != note->playableLength() ) {
				out() << playableLengthToLilyPond( note->playableLength() );
			}

			_lastNotePitch = note->getChord().at(0)->diatonicPitch();
			_lastPlayableLength = note->playableLength();
		}

		// place slurs and phrasing slurs
		if (!note->isPartOfChord() && note->slurStart() ||
		    note->isPartOfChord() && note->isLastInChord() && note->getChord().at(0)->slurStart() ) {
			out() << "(";
		}
		if (!note->isPartOfChord() && note->slurEnd() ||
		    note->isPartOfChord() && note->isLastInChord() && note->getChord().at(0)->slurEnd() ) {
			out() << ")";
		}
		if (!note->isPartOfChord() && note->phrasingSlurStart() ||
		    note->isPartOfChord() && note->isLastInChord() && note->getChord().at(0)->phrasingSlurStart() ) {
			out() << "\\(";
		}
		if (!note->isPartOfChord() && note->phrasingSlurEnd() ||
		    note->isPartOfChord() && note->isLastInChord() && note->getChord().at(0)->phrasingSlurEnd() ) {
			out() << "\\)";
		}

		// add to the stream time, if the note is not part of the chord or is the last one in the chord
		if (!note->isPartOfChord() ||
		    (note->isPartOfChord() && note->isLastInChord()) )
			_curStreamTime += note->timeLength();

			break;
	}
	case CAMusElement::Rest: {
		// CARest
		CARest *rest = static_cast<CARest*>(elt);
		if (rest->timeStart()!=_curStreamTime) break;	//! \todo If the time isn't the same, insert hidden rests to fill the needed time

		out() << restTypeToLilyPond(rest->restType());

		if ( _lastPlayableLength!=rest->playableLength() ) {
			out() << playableLengthToLilyPond( rest->playableLength() );
		}

		_lastPlayableLength = rest->playableLength();
		_curStreamTime += rest->timeLength();

		break;
	}
	}

	if ( elt->isLastInTuplet() ) {
		out() << "} ";
	}
}

/*!
	Exports the music element's \a elt marks.
*/
void CALilyPondExport::exportMarks( CAMusElement *elt ) {
	for (int i=0; i<elt->markList().size(); i++) {
		CAMark *curMark = elt->markList()[i];

		switch ( curMark->markType() ) {
		case CAMark::Text: {
			out() << "^\"" << static_cast<CAText*>(curMark)->text() << "\" ";
			break;
		}
		}
	}
}

/*!
	Exports the lyrics in form:
	SopranoLyricsOne = {
		My bu -- ny is o -- ver the o -- cean __ My bu -- ny.
	}
*/
void CALilyPondExport::exportLyricsContextBlock( CALyricsContext *lc ) {
	// Print Canorus voice name as a comment to help with debugging/tweaking
	indent();
	out() << "\n% " << lc->name() << "\n";
	QString name = lc->name();
	spellNumbers(name);
	out() << name << " = \\lyricmode {\n";
	indentMore();

	indent();
	exportLyricsContextImpl(lc);

	indentLess();
	out() << "\n}\n";
}

/*!
	Exports the syllables only without the SopranoLyircsOne = {} frame.
*/
void CALilyPondExport::exportLyricsContextImpl( CALyricsContext *lc ) {
	for (int i=0; i<lc->syllableCount(); i++) {
		if (i>0) out() << " "; // space between syllables
		out() << syllableToLilyPond(lc->syllableAt(i));
	}
}

/*!
	Writes the partial command before the first note if there is an upbeat.
*/
void CALilyPondExport::doAnacrusisCheck(CATimeSignature *time) {

				if (!time) return;			// without time signature no upbeat

				// compute the lenght of the beat note,	eigth/quarter/half are supported
				int beatNoteLen = CAPlayableLength::playableLengthToTimeLength( CAPlayableLength::Quarter );
				switch (time->beat()) {
				case 4:			break;
				case 8:			beatNoteLen /= 2;	break;
				case 2:			beatNoteLen *= 2;	break;
				default:		return;		// at strange base notes no upbeat
				}

				int oneBar = time->beats()*beatNoteLen;
				int barlen = 0;
				for (int j=0; j<curVoice()->musElementCount(); j++) {
					if (curVoice()->musElementAt(j)->isPlayable()) {
						barlen += curVoice()->musElementAt(j)->timeLength();
					}
					// after one bar without barline no anacrusis (probably a staff without barlines)
					if (curVoice()->musElementAt(j)->musElementType() == CAMusElement::Barline) break;
					// don't look for more than one bar
					if (barlen >= oneBar) break;
				}
				if (barlen < oneBar) {
					CAPlayableLength res = CAPlayableLength( CAPlayableLength::HundredTwentyEighth );
					out() << "\\partial "
					<<res.musicLength()
					<<"*"<<barlen/res.playableLengthToTimeLength(res)
					<<" ";
				}
}

/*!
	Writes the voice's \relative note intro and returns the note pitch for the current voice.
	This function is usually used for writing the beginning of the voice.
	\warning This function doesn't write "{" paranthesis to mark the start of the voice!
*/
CADiatonicPitch CALilyPondExport::writeRelativeIntro() {
	int i;

	// find the first playable element and set the key signature if found any
	for ( i=0;
	      (i<curVoice()->musElementCount() &&
	      (curVoice()->musElementAt(i)->musElementType()!=CAMusElement::Note));
	      i++);

	// no playable elements present, return default c' pitch
	if (i==curVoice()->musElementCount())
		return CADiatonicPitch( 28 );

	CADiatonicPitch notePitch = static_cast<CANote*>(curVoice()->musElementAt(i))->diatonicPitch();
	notePitch.setNoteName( ((notePitch.noteName() + 3) / 7) * 7 );
	out() << "\\relative "
	      << relativePitchToString( notePitch, CADiatonicPitch(21) ) << " "; // LilyPond default C is c1

	return notePitch;
}

/*!
	This function returns the relative note pitch in LilyPond syntax on the given
	Canorus pitch and previous note pitch.

	eg. pitch=10, accs=1, prevPitch=5 => returns "f'"

	\sa notePitchToLilyPond()
*/
const QString CALilyPondExport::relativePitchToString( CADiatonicPitch p, CADiatonicPitch prevPitch) {
	// write the note name
	QString stringPitch = diatonicPitchToLilyPond(p);

	// write , or ' to lower/higher a note
	int delta = prevPitch.noteName() - p.noteName();
	while (delta > 3) { // add the needed amount of the commas
		stringPitch += ",";
		delta -= 7;
	}
	while (delta < -3) { // add the needed amount of the apostrophes
		stringPitch += "'";
		delta += 7;
	}

	return stringPitch;
}


/*!
	Converts the given clefType to LilyPond syntax.
*/
const QString CALilyPondExport::clefTypeToLilyPond( CAClef::CAClefType clefType, int c1orig, int offset ) {
	QString type;
	int c1 = c1orig + offset;

	switch (clefType) {
		case CAClef::G:
			if (c1==-2) type = "treble";
			else if (c1==-4) type = "french";
			break;
		case CAClef::F:
			if (c1==10) type = "bass";
			else if (c1==8) type = "varbaritone";
			else if (c1==12) type = "subbass";
			break;
		case CAClef::C:
			if (c1==0) type = "soprano";
			else if (c1==2) type = "mezzosoprano";
			else if (c1==4) type = "alto";
			else if (c1==6) type = "tenor";
			else if (c1==8) type = "baritone";
			break;
		case CAClef::PercussionHigh:
		case CAClef::PercussionLow:
			type = "percussion";
			break;
		case CAClef::Tab:
			type = "tab";
			break;
	}

	if ( offset > 0 )
		type.append(QString("^") + QString::number(offset+1));

	if ( offset < 0 )
		type.append(QString("_") + QString::number((offset-1)*(-1)));

	return type;
}

/*!
	Returns the key signature gender major/minor in LilyPond syntax.

	\sa keySignaturePitchToLilyPond()
*/
const QString CALilyPondExport::diatonicKeyGenderToLilyPond( CADiatonicKey::CAGender gender ) {
	switch (gender) {
		case CADiatonicKey::Major: return QString("\\major");
		case CADiatonicKey::Minor: return QString("\\minor");
	}

	return QString();
}

/*!
	Converts the note length to LilyPond syntax.
*/
const QString CALilyPondExport::playableLengthToLilyPond( CAPlayableLength playableLength ) {
	QString length;
	switch (playableLength.musicLength()) {
		case CAPlayableLength::Breve:
			length = "0";
			break;
		case CAPlayableLength::Whole:
			length = "1";
			break;
		case CAPlayableLength::Half:
			length = "2";
			break;
		case CAPlayableLength::Quarter:
			length = "4";
			break;
		case CAPlayableLength::Eighth:
			length = "8";
			break;
		case CAPlayableLength::Sixteenth:
			length = "16";
			break;
		case CAPlayableLength::ThirtySecond:
			length = "32";
			break;
		case CAPlayableLength::SixtyFourth:
			length = "64";
			break;
		case CAPlayableLength::HundredTwentyEighth:
			length = "128";
			break;
	}

	for (int j=0; j<playableLength.dotted(); j++) length += ".";

	return length;
}

/*!
	Converts the note pitch to LilyPond syntax.
*/
const QString CALilyPondExport::diatonicPitchToLilyPond( CADiatonicPitch pitch ) {
	QString name;

	name = (char)((pitch.noteName()+2)%7 + 'a');

	for (int i=0; i < pitch.accs(); i++)
		name += "is";	// append as many -is-es as necessary

	for (int i=0; i > pitch.accs(); i--) {
		if ( (name == "e") || (name == "a") )
			name += "s";	// for pitches E and A, only append single -s the first time
		else if (name[0]=='a')
			name += "as";	// for pitch A, append -as instead of -es
		else
			name += "es";	// otherwise, append normally as many es-es as necessary
	}

	return name;
}

/*!
	Converts the rest type to LilyPond syntax.
*/
const QString CALilyPondExport::restTypeToLilyPond(CARest::CARestType type) {
	switch (type) {
		case CARest::Normal:
			return "r";
			break;
		case CARest::Hidden:
			return "s";
			break;
	}
}

/*!
	Converts the barline type to LilyPond syntax.
*/
const QString CALilyPondExport::barlineTypeToLilyPond(CABarline::CABarlineType type) {
	switch (type) {
		case CABarline::Single:
			return "|";
			break;
		case CABarline::Double:
			return "||";
			break;
		case CABarline::End:
			return "|.";
			break;
		case CABarline::RepeatOpen:
			return "|:";
			break;
		case CABarline::RepeatClose:
			return ":|";
			break;
		case CABarline::RepeatCloseOpen:
			return ":|:";
			break;
		case CABarline::Dotted:
			return ":";
			break;
	}
}

const QString CALilyPondExport::syllableToLilyPond( CASyllable *s ) {
	QString ret = (s->text().isEmpty()?"_":s->text());

	if (s->hyphenStart())
		ret += " --";
	else if (s->melismaStart())
		ret += " __";

	return ret;
}

/*!
	Exports the current document to Lilypond syntax as a complete .ly file.
*/
void CALilyPondExport::exportDocumentImpl(CADocument *doc)
{
	if ( doc->sheetCount() < 1 ) {
		//TODO: no sheets, raise an error
		return;
	}

	// Print file name and Canorus version in comments at the top of the file
	out() << "% This document was generated by Canorus, version " << CANORUS_VERSION << "\n";

	// Version of Lilypond syntax being generated.
	out() << "\\version \"2.10.0\"\n";

	writeDocumentHeader();

	// For now only export the first sheet of the document
	exportSheetImpl( doc->sheetAt( 0 ) );
}


/*!
	Exports the current sheet to Lilypond syntax.
*/
void CALilyPondExport::exportSheetImpl(CASheet *sheet)
{
	setCurSheet( sheet );

	// Export voices as Lilypond variables: \StaffOneVoiceOne = \relative c { ... }
	for ( int c = 0; c < sheet->contextCount(); ++c ) {
		setCurContextIndex( c );
		switch (sheet->contextAt(c)->contextType()) {
			case CAContext::Staff:
				exportStaffVoices( static_cast<CAStaff*>(sheet->contextAt( c )) );
				break;
			case CAContext::LyricsContext:
				exportLyricsContextBlock( static_cast<CALyricsContext*>(sheet->contextAt( c )) );
				break;
		}
	}

	exportScoreBlock( sheet );
}

/*!
	Export document title, subtitle, composer, copyright etc.
*/
void CALilyPondExport::writeDocumentHeader() {
	out() << "\n\\header {\n";
	indentMore();

	indent(); out() << "title          = " << markupString( exportedDocument()->title() ) << "\n";
	indent(); out() << "subtitle       = " << markupString( exportedDocument()->subtitle() ) << "\n";
	indent(); out() << "composer       = " << markupString( exportedDocument()->composer() ) << "\n";
	indent(); out() << "arranger       = " << markupString( exportedDocument()->arranger().isEmpty()?"":(tr("arr.", "arrangement")+" "+exportedDocument()->arranger()) ) << "\n";
	indent(); out() << "poet           = " << markupString( exportedDocument()->poet() ) << "\n";
	indent(); out() << "texttranslator = " << markupString( exportedDocument()->textTranslator() ) << "\n";
	indent(); out() << "dedication     = " << markupString( exportedDocument()->dedication() ) << "\n";
	indent(); out() << "copyright      = " << markupString( exportedDocument()->copyright() ) << "\n";
	indentLess();

	out() << "}\n";
}

/*!
	Encapsulates the given string into \markup {}.
*/
QString CALilyPondExport::markupString( QString in ) {
	return QString("\\markup {\"") + escapeWeirdChars( in ) + QString("\"}");
}

/*!
	Replaces characters like backslashes and double brackets with their escaped variant.
*/
QString CALilyPondExport::escapeWeirdChars( QString in ) {
	return in.replace("\\", "\\\\").replace("\"", "\\\"");
}

/*!
	Exports all the voices in the staff to Lilypond.
	Each voice in the staff is stored as a Lilypond variable:

	StaffOneVoiceOne = \relative c { ... }
*/
void CALilyPondExport::exportStaffVoices(CAStaff *staff)
{
	for ( int v = 0; v < staff->voiceCount(); ++v ) {
		setCurVoice( staff->voiceAt( v ) );

		// Print Canorus voice name as a comment to help with debugging/tweaking
		indent();
		out() << "\n% " << curVoice()->name() << "\n";

		// Write out the voice name and the equals signexportVoice
		// Variable name is staff index and voice index
		QString voiceName;
		voiceVariableName( voiceName, curContextIndex(), v );
		out() << voiceName << " = ";

		exportVoiceImpl( curVoice() );
		out() << "\n"; // exportVoiceImpl doesn't put endline at the end
	}
}


/*!
	Modify \a name to contain a string "StaffXVoiceY" where
	X and Y are spelled-out versions of \a staffNum and \a voiceNum, respectively.
	This is for generating names of Lilypond variables from Canorus staff and voices indices,
	since Lilypond variable names must contain only alphabetical characters.

	Example: voiceVariableName( name, 1, 2 );
	--> name is "StaffOneVoiceTwo"
*/
void CALilyPondExport::voiceVariableName( QString &name, int staffNum, int voiceNum )
{
	QTextStream( &name ) << "Context" << staffNum << "Voice" << voiceNum ;
	spellNumbers( name );
}


/*!
	Exports the \score block for LilyPond from the current sheet.
	Looks like this:

	\code
	\score {
		<<
			\new Staff {
				<<
					\new Voice { \voiceOne \StaffOneVoiceOne }
					\new Voice { \voiceTwo \StaffOneVoiceTwo }
				>>
			}
			\new Staff {
				<<
					\new Voice { \voiceOne \StaffTwoVoiceOne }
					\new Voice { \voiceTwo \StaffTwoVoiceTwo }
				>>
			}
			...
		>>
	}
	\endcode
*/
void CALilyPondExport::exportScoreBlock( CASheet *sheet ) {
	out() << "\n\\score {\n";
	indentMore();
	int contextCount = sheet->contextCount();
	if ( contextCount < 1 ) {
		out() << "% No Contexts. This should probably raise an error.\n";
	}
	else {
		// open multiple contexts
		indent();
		out() <<  "<<\n" ;
		indentMore();

		// Output each staff
		for( int c = 0; c < contextCount; ++c ) {
			setCurContext( sheet->contextAt( c ) );

			switch (curContext()->contextType()) {
				case CAContext::Staff: {
					CAStaff *s = static_cast<CAStaff*>(curContext());

					indent();
					out() <<  "\\new Staff {\n" ;
					indentMore();

					// More than one voice? Add simultaneous symbol
					int voiceCount = s->voiceCount();
					if ( voiceCount > 1 ) {
						indent();
						out() <<  "<<\n" ;
						indentMore();
					}

					// Output voices
					for( int v = 0; v < voiceCount; ++v ) {

						// Print Canorus voice name as a comment to aid with debugging etc.
						QString curVoiceName( s->voiceAt( v )->name() );
						indent();
						out() << "% " << curVoiceName << "\n";

						// curVoiceLilyCommand is "\voiceOne", "\voiceTwo", etc. to get proper stem directions
						// Only use this if there is more than one voice.
						QString curVoiceLilyCommand;
						if ( voiceCount > 1 ) {
							curVoiceLilyCommand.setNum( v + 1 );
							curVoiceLilyCommand = "\\voice" + curVoiceLilyCommand;
							spellNumbers( curVoiceLilyCommand );
						}

						// Print Lily variable name
						QString voiceName;
						voiceVariableName( voiceName, c, v );
						indent();
						out() <<  "\\new Voice = \"" << voiceName << "Virtual\" { " << curVoiceLilyCommand << " \\" << voiceName << " }\n" ;
					}
					indentLess();

					// End simultaneous voice
					if ( voiceCount > 1 ) {
						indent();
						out() <<  ">>\n" ;
						indentLess();
					}

					// End \new Staff {
					indent();
					out() <<  "}\n" ;

					break;
				}
				case CAContext::LyricsContext: { // only position the lyrics contexts. Voice assignment at the end of the score block!
					CALyricsContext *lc = static_cast<CALyricsContext*>(curContext());
					QString lcName = lc->name();
					spellNumbers( lcName );

					indent();
					out() << "% " << lc->name() << "\n";
					indent();
					out() << "\\new Lyrics = \"" << lcName << "Virtual\"\n";

					break;
				}
			}

		} // for(contexts)

		// After positioning the lyrics contexts, set their associated voices!
		for (int i=0; i<contextCount; i++) {
			if (i==0) {
				indent();
				out() << "\n";
				indent();
				out() << "% Voice assignment:\n";
				indent();
				 // needed for automatic treating of slurs as melisma for lyrics - multiple syllables below the slured notes are allowed in Canorus, but not recommended
				out() << "\\set Score.melismaBusyProperties = #'()\n";
			}

			CALyricsContext *lc;
			if (lc = dynamic_cast<CALyricsContext*>(sheet->contextAt( i ))) {
				QString lcName = lc->name();
				spellNumbers(lcName);

				QString voiceName;
				voiceVariableName(
					voiceName,
					curSheet()->contextList().indexOf(lc->associatedVoice()->staff()),
					lc->associatedVoice()->staff()->voiceList().indexOf(lc->associatedVoice())
				);

				indent();
				out() << "\\context Lyrics = \"" << lcName << "Virtual\" { \\lyricsto \"" << voiceName << "Virtual\" \\" << lcName << " }\n";
			}
		}

		// End simultaneous contexts
		indentLess();

		// close multiple contexts
		indent();
		out() << ">>\n";
		indentLess();
	}

	// End score block
	out() << "}\n";
}


/*!
	Output tabs according to _indentLevel.
*/
void CALilyPondExport::indent()
{
	for ( int i = 0; i < curIndentLevel(); ++i ) {
		out() << "\t";
	}
}


/*!
	Spell out numbers in a QString: "Staff1Voice2" -> "StaffOneVoiceTwo"
	This is necessary because Lilypond variable names can only contain letters.
*/
void CALilyPondExport::spellNumbers( QString &s )
{
	s.replace( "0" , "Zero" );
	s.replace( "1" , "One" );
	s.replace( "2" , "Two" );
	s.replace( "3" , "Three" );
	s.replace( "4" , "Four" );
	s.replace( "5" , "Five" );
	s.replace( "6" , "Six" );
	s.replace( "7" , "Seven" );
	s.replace( "8" , "Eight" );
	s.replace( "9" , "Nine" );
}
