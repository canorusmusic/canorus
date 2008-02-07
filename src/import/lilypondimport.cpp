/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QTextStream>
#include <QRegExp>

#include <iostream> // DEBUG

#include "import/lilypondimport.h"
#include "core/note.h"
#include "core/playable.h"
#include "core/slur.h"

/*!
	Delimiters which separate various music elements in LilyPond syntax. These are new lines, tabs, blanks etc.
	
	\sa nextElement(), parseNextElement()
*/
const QRegExp CALilyPondImport::WHITESPACE_DELIMITERS = QRegExp("[\\s]");

/*!
	Delimiters which separate various music elements in LilyPond syntax, but are specific for LilyPond syntax.
	They are reported as its own element when parsing the next element.
	
	\sa nextElement(), parseNextElement()
*/ 
const QRegExp CALilyPondImport::SYNTAX_DELIMITERS = QRegExp("[<>{}]"); 

/*!
	Combined WHITESPACE_DELIMITERS and SYNTAX_DELIMITERS.
*/
const QRegExp CALilyPondImport::DELIMITERS =
	QRegExp(
		CALilyPondImport::WHITESPACE_DELIMITERS.pattern().left(CALilyPondImport::WHITESPACE_DELIMITERS.pattern().size()-1) +
		CALilyPondImport::SYNTAX_DELIMITERS.pattern().mid(1)
	);

CALilyPondImport::CALilyPondImport( const QString in )
 : CAImport(in) {
	initLilyPondImport();
}

CALilyPondImport::CALilyPondImport( QTextStream *in )
 : CAImport(in) {
	initLilyPondImport();	
}

CALilyPondImport::~CALilyPondImport() {
}

void CALilyPondImport::initLilyPondImport() {
	_curLine = _curChar = 0;
	_curSlur = 0; _curPhrasingSlur = 0;
	_templateVoice = 0;
}

void CALilyPondImport::addError(QString description, int curLine, int curChar) {
	_errors << QString(QObject::tr("<i>Fatal error, line %1, char %2:</i><br>"))
	           .arg(curLine?curLine:_curLine)
	           .arg(curChar?curChar:_curChar)
	           + description + "<br>";
}

CAVoice *CALilyPondImport::importVoiceImpl() {
	CAVoice *voice = new CAVoice( "", 0 );
	
	if (templateVoice())
		voice->cloneVoiceProperties( templateVoice() );
	
	setCurVoice(voice);
	CADiatonicPitch prevPitch( 21, 0 );
	CAPlayableLength prevLength( CAPlayableLength::Quarter, 0 );
	bool chordCreated=false;
	bool changed=false;
	
	for (QString curElt = parseNextElement();
	     (!in().isEmpty());
	     curElt = ((curElt.size() && changed)?curElt:parseNextElement())) { // go to next element, if current one is empty or not changed
	    changed=true; // changed is default to true and false, if none of if clauses were found
		if (curElt.startsWith("\\relative")) {
			// initial \relative notePitch
			QString notePitch = parseNextElement();
			if (!isNote(notePitch)) {
				addError("\\relative doesn't include pitch.");
				continue;
			}
			
			prevPitch = relativePitchFromLilyPond(notePitch, 21);
			curElt.remove(0,9);
		} else
		if (curElt.startsWith("{")) {
			// start of the voice
			pushDepth(Voice);
			curElt.remove(0,1);			
		} else
		if (curElt.startsWith("}")) {
			// end of the voice
			popDepth();
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("<")) {
			// start of the chord
			pushDepth(Chord);
			curElt.remove(0,1);
		} else
		if (curElt.startsWith(">")) {
			// end of the chord
			popDepth();
			chordCreated=false;
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				prevPitch = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0)->diatonicPitch();
			} else {
				addError(QString("Chord should be finished with a note."));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("~")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement());
				note->setTieStart(
					new CASlur( CASlur::TieType, CASlur::SlurPreferred, note->staff(), note, 0 )
				);
			} else {
				addError(QString("Tie symbol must be right after the note and not %1. Tie ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("(")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				_curSlur = new CASlur( CASlur::SlurType, CASlur::SlurPreferred, note->staff(), note, 0 );
				note->setSlurStart(_curSlur);
			} else {
				addError(QString("Slur symbol must be right after the note and not %1. Slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if ( curElt.startsWith(")") && _curSlur ) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				note->setSlurEnd(_curSlur);
				_curSlur->setNoteEnd(note);
				_curSlur=0;
			} else {
				addError(QString("Slur symbol must be right after the note and not %1. Slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("\\(")) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				_curPhrasingSlur = new CASlur( CASlur::PhrasingSlurType, CASlur::SlurPreferred, note->staff(), note, 0 );
				note->setPhrasingSlurStart(_curPhrasingSlur);
			} else {
				addError(QString("Phrasing slur symbol must be right after the note and not %1. Phrasing slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,2);
		} else
		if ( curElt.startsWith("\\)") && _curPhrasingSlur ) {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement())->getChord().at(0);
				note->setPhrasingSlurEnd(_curPhrasingSlur);
				_curPhrasingSlur->setNoteEnd(note);
				_curPhrasingSlur=0;
			} else {
				addError(QString("Phrasing slur symbol must be right after the note and not %1. Phrasing slur ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
			curElt.remove(0,2);
		} else
		if (isNote(curElt)) {
			// CANote
			prevPitch = relativePitchFromLilyPond(curElt, prevPitch, true);
			CAPlayableLength length = playableLengthFromLilyPond(curElt, true);
			if (length.musicLength()!=CAPlayableLength::Undefined) // length may not be set
				prevLength = length;
			
			CANote *note;
			if (curDepth()!=Chord || !chordCreated) {
				// the note is not part of the chord or is the first note in the chord
				note = new CANote( prevPitch, prevLength, curVoice(), curVoice()->lastTimeEnd() );
				if (curDepth()==Chord)
					chordCreated = true;
				curVoice()->append( note, false );
			} else {
				// the note is part of the already built chord
				note = new CANote( prevPitch, prevLength, curVoice(), curVoice()->lastTimeStart() );
				curVoice()->append( note, true );
			}
			
			note->updateTies(); // close any opened ties if present
		} else
		if (isRest(curElt)) {
			// CARest
			CARest::CARestType type = restTypeFromLilyPond(curElt, true);
			CAPlayableLength length = playableLengthFromLilyPond(curElt, true);
			if (length.musicLength()!=CAPlayableLength::Undefined) // length may not be set
				prevLength = length;
						
			curVoice()->append( new CARest( type, prevLength, curVoice(), curVoice()->lastTimeEnd() ) );
		} else
		if (curElt.startsWith("|")) {
			// CABarline::Single
			// lookup an element with the same type at the same time
			CABarline *bar = new CABarline(CABarline::Single, curVoice()->staff(), curVoice()->lastTimeEnd());
			CABarline *sharedBar = static_cast<CABarline*>(findSharedElement(bar));

			if (!sharedBar) {
				curVoice()->append( bar );
			} else {
				curVoice()->append( sharedBar );
				delete bar;
			}
			curElt.remove(0,1);
		} else
		if (curElt.startsWith("\\bar")) {
			// CABarline
			QString typeString = peekNextElement();
			CABarline::CABarlineType type = barlineTypeFromLilyPond(peekNextElement());
			
			if (type==CABarline::Undefined) {
				addError(QString("Error while parsing barline type. Barline type %1 unknown.").arg(typeString));
			}
			
			// remove clef type from the input
			parseNextElement();
			
			// lookup an element with the same type at the same time
			CABarline *bar = new CABarline(type, curVoice()->staff(), curVoice()->lastTimeEnd());
			CABarline *sharedBar = static_cast<CABarline*>(findSharedElement(bar));

			if (!sharedBar) {
				curVoice()->append( bar );
			} else {
				curVoice()->append( sharedBar );
				delete bar;
			}
			curElt.remove(0,4);
		} else
		if (curElt.startsWith("\\clef")) {
			// CAClef
			QString typeString = peekNextElement();
			CAClef::CAPredefinedClefType type = predefinedClefTypeFromLilyPond( peekNextElement() );
			int offset = clefOffsetFromLilyPond( peekNextElement() );
			
			if (type==CAClef::Undefined) {
				addError(QString("Error while parsing clef type. Clef type %1 unknown.").arg(typeString));
			}
			
			// remove clef type from the input
			parseNextElement();
			
			CAClef *clef = new CAClef( type, curVoice()->staff(), curVoice()->lastTimeEnd(), offset );
			CAClef *sharedClef = static_cast<CAClef*>(findSharedElement(clef));
			
			if (!sharedClef) {
				curVoice()->append( clef );
			} else {
				curVoice()->append( sharedClef );
				delete clef;
			}
			curElt.remove(0,5);
		} else
		if (curElt=="\\key") {
			// CAKeySignature
			// pitch
			QString keyString = peekNextElement();
			if (!isNote(keyString)) {
				addError(QString("Error while parsing key signature. Key pitch %1 unknown.").arg(keyString));
				continue;
			}
			parseNextElement(); // remove pitch
			
			// gender
			QString genderString = peekNextElement();
			CAKeySignature::CAMajorMinorGender gender = keySigGenderFromLilyPond(genderString);
			if (gender==CAKeySignature::Undefined) {
				addError(QString("Error while parsing key signature gender. Gender %1 unknown.").arg(genderString));
				continue;
			}
			parseNextElement();
			
			signed char accs = keySigAccsFromLilyPond(keyString, gender);
			
			CAKeySignature *keySig = new CAKeySignature(CAKeySignature::MajorMinor, accs, gender, curVoice()->staff(), curVoice()->lastTimeEnd());
			CAKeySignature *sharedKeySig = static_cast<CAKeySignature*>(findSharedElement(keySig));
			
			if (!sharedKeySig) {
				curVoice()->append( keySig );
			} else {
				curVoice()->append( sharedKeySig );
				delete keySig;
			}
			curElt.remove(0,4);
		} else
		if (curElt.startsWith("\\time")) {
			// CATimeSignature
			QString timeString = peekNextElement();
			// time signature should have beats/beat format
			if (timeString.indexOf(QRegExp("\\d+/\\d+")) == -1) {
				addError(QString("Invalid time signature beats format %1. Beat and number of beats should be written <beats>/<beat>.").arg(timeString));
				continue;
			}
			CATime time = timeSigFromLilyPond(timeString);
			parseNextElement();
			
			CATimeSignature *timeSig = new CATimeSignature(time.beats, time.beat, curVoice()->staff(), curVoice()->lastTimeEnd());
			CATimeSignature *sharedTimeSig = static_cast<CATimeSignature*>(findSharedElement(timeSig));
			
			if (!sharedTimeSig) {
				curVoice()->append( timeSig );
			} else {
				curVoice()->append( sharedTimeSig );
				delete timeSig;
			}
			curElt.remove(0,5);
		} else
			changed=false;
	}
	
	return voice;
}

CALyricsContext *CALilyPondImport::importLyricsContextImpl() {
	CALyricsContext *lc = new CALyricsContext( "", 1, static_cast<CASheet*>(0) );
	
	CASyllable *lastSyllable = 0;
	int timeSDummy=0; // dummy timestart to keep the order of inserted syllables. Real timeStarts are sets when repositSyllables() is called
	for (QString curElt = parseNextElement(); (!in().isEmpty() || !curElt.isEmpty() ); curElt = parseNextElement(), timeSDummy++) {
		QString text = curElt;
		if (curElt == "_")
			text = "";
		
		if (lastSyllable && text=="--") {
			lastSyllable->setHyphenStart(true);
		} else
		if (lastSyllable && text=="__") {
			lastSyllable->setMelismaStart(true);
		} else {
			lc->addSyllable( lastSyllable = new CASyllable( text, false, false, lc, timeSDummy, 0 ) );
		}
	}
	lc->repositSyllables(); // sets syllables timeStarts and timeLengths
	
	return lc;
}

/*!
	Returns the first element in input stream ended with one of the delimiters and shorten input stream for the element.
	
	\todo Only one-character syntax delimiters are supported so far. 
	\sa peekNextElement()
*/
const QString CALilyPondImport::parseNextElement() {
	// find the first non-whitespace character
	int start = in().indexOf(QRegExp("\\S"));
	if (start==-1)
		start = 0;
	
	int i = in().indexOf(DELIMITERS, start);
	if (i==-1)
		i=in().size();
	
	QString ret;
	if (i==start) {
		// syntax delimiter only
		ret = in().mid(start,1); // \todo Support for syntax delimiters longer than 1 character
		in().remove(0, start+1);
	} else {
		// ordinary whitespace/syntax delimiter
		ret = in().mid(start, i-start);
		in().remove(0, i);
	}
	
	return ret;
}

/*!
	Returns the first element in input stream ended with one of the delimiters but don't shorten the stream.
	
	\sa parseNextElement()
*/
const QString CALilyPondImport::peekNextElement() {
	// find the first non-whitespace character
	int start = in().indexOf(QRegExp("\\S"));
	if (start==-1)
		start = 0;
	
	int i = in().indexOf(DELIMITERS, start);
	if (i==-1)
		i=in().size();
	
	QString ret;
	if (i==start) {
		// syntax delimiter only
		ret = in().left(1); // \todo
	} else {
		// ordinary whitespace/syntax delimiter
		ret = in().mid(start, i-start);
	}
	
	return ret;
}

/*!
	Gathers a list of music elements with the given element's start time and returns the first music element in the
	gathered list with the same attributes.
	
	This method is usually called when voices have "shared" music elements (barlines, clefs etc.). However, in LilyPond
	syntax the music element can/should be present in all the voices. This function finds this shared music element, if
	it already exists.
	
	If the music element with the same properties exists, user should delete its own instance and add an already
	existing instance of the returned shared music element to the voice.
	
	\sa CAMusElement::compare()
*/
CAMusElement* CALilyPondImport::findSharedElement(CAMusElement *elt) {
	if ( !curVoice() || !curVoice()->staff() )
		return 0;
	
	// gather a list of all the music elements of that type in the staff at that time
	QList<CAMusElement*> foundElts = curVoice()->staff()->getEltByType( elt->musElementType(), elt->timeStart() );
	
	// compare gathered music elements properties
	for (int i=0; i<foundElts.size(); i++)
		if (!foundElts[i]->compare(elt))             // element has exactly the same properties
			if (!curVoice()->contains(foundElts[i])) // element isn't present in the voice yet
				return foundElts[i];
	
	return 0;
}

/*!
	Returns true, if the given LilyPond element is a note.
	
	\sa isRest()
*/
bool CALilyPondImport::isNote(const QString elt) {
	return QString(elt[0]).contains(QRegExp("[a-g]"));
}

/*!
	Returns true, if the given LilyPond element is a rest.
	
	\sa isNote()
*/
bool CALilyPondImport::isRest(const QString elt) {
	return (elt[0]=='r' || elt[0]=='s' || elt[0]=='R');
}

/*!
	Generates the note pitch and number of accidentals from the note written in LilyPond syntax.
	
	\sa playableLengthFromLilyPond()
*/
CADiatonicPitch CALilyPondImport::relativePitchFromLilyPond(QString& constNName, CADiatonicPitch prevPitch, bool parse) {
	QString noteName = constNName;
	
	// determine pitch
	int curPitch = noteName[0].toLatin1() - 'a' + 5	// determine the 0-6 pitch from note name
	               - (prevPitch.noteName() % 7);	
	while (curPitch<-3)	//normalize pitch - the max +/- interval is fourth
		curPitch+=7;
	while (curPitch>3)
		curPitch-=7;
	curPitch += prevPitch.noteName();
	
	// determine accidentals
	signed char curAccs = 0;
	while (noteName.indexOf("is") != -1) {
		curAccs++;
		noteName.remove(0, noteName.indexOf("is") + 2);
		if (parse)
			constNName.remove(0, constNName.indexOf("is") + 2);
	}
	while ((noteName.indexOf("es") != -1) || (noteName.indexOf("as") != -1)) {
		curAccs--;
		noteName.remove(0, ((noteName.indexOf("es")==-1) ? (noteName.indexOf("as")+2) : (noteName.indexOf("es")+2)) );
		if (parse)
			constNName.remove(0, ((constNName.indexOf("es")==-1) ? (constNName.indexOf("as")+2) : (constNName.indexOf("es")+2)) );
	}
	if (!curAccs && parse)
		constNName.remove(0, 1);
	
	// add octave up/down
	for (int i=0; i<noteName.size(); i++) {
		if (noteName[i]=='\'') {
			curPitch+=7;
			if (parse)
				constNName.remove(0,1);
		} else if (noteName[i]==',') {
			curPitch-=7;
			if (parse)
				constNName.remove(0,1);
		}
	}
	
	return CADiatonicPitch( curPitch, curAccs );
}

/*!
	Generates playable lentgth and number of dots from the note/rest string in LilyPond syntax.
	If the playable element doesn't include length, { CAPlayable::CAPlayableLength::Undefined, 0 } is returned.
	This function also shortens the given string for the playable length, if \a parse is True.
	
	\sa relativePitchFromString()
*/
CAPlayableLength CALilyPondImport::playableLengthFromLilyPond(QString& elt, bool parse) {
	CAPlayableLength ret;
	
	// index of the first number
	int start = elt.indexOf(QRegExp("[\\d]"));
	if (start == -1)  // no length written
		return ret;
	else {            // length written
		// count dots
		int d=0;
		int dStart;
		for (int i = dStart = elt.indexOf(".",start);
		     i!=-1 && i<elt.size() && elt[i]=='.';
		     i++, ret.setDotted( ret.dotted()+1 ));
		
		if (dStart == -1)
			dStart = elt.indexOf(QRegExp("[\\D]"), start);
		if (dStart == -1)
			dStart = elt.size();
		
		ret.setMusicLength( static_cast<CAPlayableLength::CAMusicLength>(elt.mid(start, dStart-start).toInt()) );
		if (parse)
			elt.remove(start, dStart-start+ret.dotted());
	}
	
	return ret;
}

/*!
	Genarates rest type from the LilyPond syntax for the given rest.
	This function also shortens the given string for the rest type, if \a parse is True.
*/
CARest::CARestType CALilyPondImport::restTypeFromLilyPond( QString& elt, bool parse ) {
	CARest::CARestType t = CARest::Normal;
	
	if (elt[0]=='r' || elt[0]=='R')
		t = CARest::Normal;
	else
		t = CARest::Hidden;
	
	if (parse)
		elt.remove( 0, 1 );
	
	return t;
}

/*!
	Genarates clef type from the LilyPond syntax for the given clef from format "clefType".
*/
CAClef::CAPredefinedClefType CALilyPondImport::predefinedClefTypeFromLilyPond( const QString constClef ) {
	// remove any quotes/double quotes
	QString clef(constClef);
	clef.remove(QRegExp("[\"']"));
	
	if ( clef.contains("treble") || clef.contains("violin") || clef.contains("G") ) return CAClef::Treble;
	if ( clef.contains("french") ) return CAClef::French;
	if ( clef.contains("bass") || clef.contains("F") ) return CAClef::Bass;
	if ( clef.contains("varbaritone") ) return CAClef::Varbaritone;
	if ( clef.contains("subbass") ) return CAClef::Subbass;
	if ( clef.contains("mezzosoprano") ) return CAClef::Mezzosoprano;
	if ( clef.contains("soprano") ) return CAClef::Soprano;
	if ( clef.contains("alto") ) return CAClef::Alto;
	if ( clef.contains("tenor") ) return CAClef::Tenor;
	if ( clef.contains("baritone") ) return CAClef::Baritone;
	if ( clef=="percussion" ) return CAClef::Percussion;
	if ( clef=="tab" ) return CAClef::Tablature;
	
	return CAClef::Treble;
}

/*!
	Returns the Canorus octava or whichever interval above or below the clef.
*/
int CALilyPondImport::clefOffsetFromLilyPond( const QString constClef ) {
	// remove any quotes/double quotes
	QString clef(constClef);
	clef.remove(QRegExp("[\"']"));
	
	if ( !clef.contains("_") && !clef.contains("^") )
		return 0;
	
	int m;
	int idx = clef.indexOf("^");
	if (idx==-1) {
		idx = clef.indexOf("_");
		m=-1;
	} else
		m=1;
	
	return clef.right( clef.size()-(idx+1) ).toInt()*m;
}

/*!
	Returns the number and type of accidentals for the given key signature.
*/
signed char CALilyPondImport::keySigAccsFromLilyPond(QString keySig, CAKeySignature::CAMajorMinorGender gender) {
	signed char accs = static_cast<signed char>( ((keySig[0].toAscii() - 'a') * 2 + 4) % 7 - 4 );
	
	QString key(keySig);
	accs -= 7*keySig.count("as");
	accs -= 7*keySig.count("es");
	accs += 7*keySig.count("is");
	
	if (gender==CAKeySignature::Major)
		accs += 3;
	
	return accs;
}

/*!
	Returns the key signature gender from format \\genderType.
*/
CAKeySignature::CAMajorMinorGender CALilyPondImport::keySigGenderFromLilyPond(QString gender) {
	if (gender=="\\major")
		return CAKeySignature::Major;
	else if (gender=="\\minor")
		return CAKeySignature::Minor;
	else
		return CAKeySignature::Undefined;
}

/*!
	Returns the time signature beat and beats in beats/beat format.
*/
CALilyPondImport::CATime CALilyPondImport::timeSigFromLilyPond(QString timeSig) {
	int beats=0, beat=0;
	
	beats = timeSig.mid(0, timeSig.indexOf("/")).toInt();
	beat = timeSig.mid(timeSig.indexOf("/")+1).toInt();
	
	CATime time = { beats, beat };
	return time;
}

/*!
	Genarates barline type from the LilyPond syntax for the given barline from format "barlineType".
*/
CABarline::CABarlineType CALilyPondImport::barlineTypeFromLilyPond(QString constBarline) {
	// remove any quotes/double quotes
	QString barline(constBarline);
	barline.remove(QRegExp("[\"']"));
	
	if (barline=="|") return CABarline::Single; else
	if (barline=="||") return CABarline::Double; else
	if (barline=="|.") return CABarline::End; else
	if (barline=="|:") return CABarline::RepeatOpen; else
	if (barline==":|") return CABarline::RepeatClose; else
	if (barline==":|:") return CABarline::RepeatCloseOpen; else
	if (barline==":") return CABarline::Dotted; else
	return CABarline::Undefined;
}

const QString CALilyPondImport::readableStatus() {
	switch (status()) {
	case 0:
		return tr("Ready");
	case 1:
		return tr("Importing...");
	case -1:
		return tr("Error while importing!\nLine %1:%2.").arg(curLine()).arg(curChar());
	}
}
