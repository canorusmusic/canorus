/*
 * Copyright (c) 2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 *
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#include <QMessageBox>

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

CALilyPondImport::CALilyPondImport(QString& in, CAVoice *voice) {
	_in = in;
	_curLine = _curChar = 0;
	
	importVoice(voice);
	QString messages;
	if (_errors.size()) {
		for (int i=0; i<_errors.size(); i++) messages += _errors[i];
		QMessageBox::critical(0, QObject::tr("Import error"), messages);
	}
}

CALilyPondImport::~CALilyPondImport() {
}

void CALilyPondImport::addError(QString description, int curLine, int curChar) {
	_errors << QString(QObject::tr("<i>Fatal error, line %1, char %2:</i><br>"))
	           .arg(curLine?curLine:_curLine)
	           .arg(curChar?curChar:_curChar)
	           + description + "<br>";
}

bool CALilyPondImport::importVoice(CAVoice *voice) {
	setCurVoice(voice);
	CAPitch prevPitch = { 21, 0 };
	CALength prevLength = { CAPlayable::Quarter, 0 };
	bool chordCreated=false;
	
	for (QString curElt = parseNextElement(); (!in().isEmpty()); curElt = parseNextElement()) {
		if (curElt=="\\relative") {
			// initial \relative notePitch
			QString notePitch = parseNextElement();
			if (!isNote(notePitch)) {
				addError("\\relative doesn't include pitch.");
				continue;
			}
			
			prevPitch = relativePitchFromLilyPond(notePitch, 21);
		} else
		if (curElt=="{") {
			// start of the voice
			pushDepth(Voice);
		} else
		if (curElt=="}") {
			// end of the voice
			popDepth();
		} else
		if (curElt=="<") {
			// start of the chord
			pushDepth(Chord);
		} else
		if (curElt==">") {
			// end of the chord
			popDepth();
			chordCreated=false;
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				prevPitch.pitch = static_cast<CANote*>(curVoice()->lastMusElement())->chord().at(0)->pitch();
				prevPitch.accs = static_cast<CANote*>(curVoice()->lastMusElement())->chord().at(0)->accidentals();
			} else {
				addError(QString("Chord should be finished with a note."));
			}
		} else
		if (curElt=="~") {
			if ( curVoice()->lastMusElement()->musElementType()==CAMusElement::Note ) {
				CANote *note = static_cast<CANote*>(curVoice()->lastMusElement());
				note->setTieStart(
					new CASlur( CASlur::TieType, CASlur::SlurPreferred, note->staff(), note, 0 )
				);
			} else {
				addError(QString("Tie symbol must be right after the note and not %1. Tie ignored.").arg(CAMusElement::musElementTypeToString(curVoice()->lastMusElement()->musElementType())));
			}
		} else
		if (isNote(curElt)) {
			// CANote
			prevPitch = relativePitchFromLilyPond(curElt, prevPitch.pitch);
			CALength length = playableLengthFromLilyPond(curElt);
			if (length.length!=CAPlayable::Undefined) // length may not be set
				prevLength = length;
			
			CANote *note;
			if (curDepth()!=Chord || !chordCreated) {
				// the note is not part of the chord or is the first note in the chord
				note = new CANote(prevLength.length, curVoice(), prevPitch.pitch, prevPitch.accs, curVoice()->lastTimeEnd(), prevLength.dotted);
				if (curDepth()==Chord)
					chordCreated = true;
			} else {
				// the note is part of the already built chord
				note = new CANote(prevLength.length, curVoice(), prevPitch.pitch, prevPitch.accs, curVoice()->lastTimeStart(), prevLength.dotted);
			}
			
			// add tie
			if ( curElt.contains("~") )
				note->setTieStart(
					new CASlur( CASlur::TieType, CASlur::SlurPreferred, note->staff(), note, 0 )
				);
			
			note->updateTies(); // close any opened ties if present
			curVoice()->appendMusElement(note);
		} else
		if (isRest(curElt)) {
			// CARest
			CALength length = playableLengthFromLilyPond(curElt);
			if (length.length!=CAPlayable::Undefined) // length may not be set
				prevLength = length;
			
			CARest::CARestType type = restTypeFromLilyPond(curElt);
			
			curVoice()->appendMusElement(new CARest(type, prevLength.length, curVoice(), curVoice()->lastTimeEnd(), prevLength.dotted));
		} else
		if (curElt=="|") {
			// CABarline::Single
			// lookup an element with the same type at the same time
			CABarline *bar = new CABarline(CABarline::Single, curVoice()->staff(), curVoice()->lastTimeEnd());
			CABarline *sharedBar = static_cast<CABarline*>(findSharedElement(bar));

			if (!sharedBar) {
				curVoice()->staff()->insertSignAfter(bar, curVoice()->musElementCount()?curVoice()->lastMusElement():0, true);
			} else {
				curVoice()->appendMusElement(sharedBar);
				delete bar;
			}
		} else
		if (curElt=="\\bar") {
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
				curVoice()->staff()->insertSignAfter(bar, curVoice()->musElementCount()?curVoice()->lastMusElement():0, true);
			} else {
				curVoice()->appendMusElement(sharedBar);
				delete bar;
			}
		} else
		if (curElt=="\\clef") {
			// CAClef
			QString typeString = peekNextElement();
			CAClef::CAClefType type = clefTypeFromLilyPond(peekNextElement());
			
			if (type==CAClef::Undefined) {
				addError(QString("Error while parsing clef type. Clef type %1 unknown.").arg(typeString));
			}
			
			// remove clef type from the input
			parseNextElement();
			
			CAClef *clef = new CAClef(type, curVoice()->staff(), curVoice()->lastTimeEnd());
			CAClef *sharedClef = static_cast<CAClef*>(findSharedElement(clef));
			
			if (!sharedClef) {
				curVoice()->staff()->insertSignAfter(clef, curVoice()->musElementCount()?curVoice()->lastMusElement():0, true);
			} else {
				curVoice()->appendMusElement(sharedClef);
				delete clef;
			}			
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
				curVoice()->staff()->insertSignAfter(keySig, curVoice()->musElementCount()?curVoice()->lastMusElement():0, true);
			} else {
				curVoice()->appendMusElement(sharedKeySig);
				delete keySig;
			}
		} else
		if (curElt=="\\time") {
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
				curVoice()->staff()->insertSignAfter(timeSig, curVoice()->musElementCount()?curVoice()->lastMusElement():0, true);
			} else {
				curVoice()->appendMusElement(sharedTimeSig);
				delete timeSig;
			}
		}
	}
	
	return true;
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
	// gather a list of all the music elements of that type in the staff at that time
	QList<CAMusElement*> foundElts = curVoice()->staff()->getEltByType(elt->musElementType(), elt->timeStart());
	
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
CALilyPondImport::CAPitch CALilyPondImport::relativePitchFromLilyPond(const QString constNName, int prevPitch) {
	QString noteName = constNName;
	
	// determine pitch
	int curPitch = noteName[0].toLatin1() - 'a' + 5	// determine the 0-6 pitch from note name
	               - (prevPitch % 7);	
	while (curPitch<-3)	//normalize pitch - the max +/- interval is fourth
		curPitch+=7;
	while (curPitch>3)
		curPitch-=7;
	curPitch += prevPitch;
	
	// determine accidentals
	signed char curAccs = 0;
	while (noteName.indexOf("is") != -1) {
		curAccs++;
		noteName.remove(0, noteName.indexOf("is") + 2);
	}
	while ((noteName.indexOf("es") != -1) || (noteName.indexOf("as") != -1)) {
		curAccs--;
		noteName.remove(0, ((noteName.indexOf("es")==-1) ? noteName.indexOf("as")+2 : noteName.indexOf("es")+2));
	}
	
	// add octave up/down
	for (int i=0; i<noteName.size(); i++) {
		if (noteName[i]=='\'') {
			curPitch+=7;
		} else if (noteName[i]==',') {
			curPitch-=7;
		}
	}
	
	CAPitch ret = {curPitch, curAccs};
	return ret;
}

/*!
	Generates playable lentgth and number of dots from the note/rest string in LilyPond syntax.
	If the playable element doesn't include length, { CAPlayable::CAPlayableLength::Undefined, 0 } is returned.
	
	\sa relativePitchFromString()
*/
CALilyPondImport::CALength CALilyPondImport::playableLengthFromLilyPond(const QString elt) {
	CALength ret = { CAPlayable::Undefined, 0 };
	
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
		     i++, ret.dotted++);
		
		if (dStart == -1)
			dStart = elt.indexOf(QRegExp("[\\D]"), start);
		if (dStart == -1)
			dStart = elt.size();
		
		ret.length = static_cast<CAPlayable::CAPlayableLength>(elt.mid(start, dStart-start).toInt());
	}
	
	return ret;
}

/*!
	Genarates rest type from the LilyPond syntax for the given rest.
*/
CARest::CARestType CALilyPondImport::restTypeFromLilyPond(const QString elt) {
	if (elt[0]=='r' || elt[0]=='R')
		return CARest::Normal;
	else
		return CARest::Hidden;
}

/*!
	Genarates clef type from the LilyPond syntax for the given clef from format "clefType".
*/
CAClef::CAClefType CALilyPondImport::clefTypeFromLilyPond(const QString constClef) {
	// remove any quotes/double quotes
	QString clef(constClef);
	clef.remove(QRegExp("[\"']"));
	
	if (clef=="treble") return CAClef::Treble; else
	if (clef=="bass") return CAClef::Bass; else
	if (clef=="alto") return CAClef::Alto; else
	if (clef=="soprano") return CAClef::Soprano; else
	if (clef=="tenor") return CAClef::Tenor; else
	if (clef=="percussion") return CAClef::PercussionHigh; else
	return CAClef::Undefined;
}

/*!
	Returns the number and type of accidentals for the given key signature.
*/
signed char CALilyPondImport::keySigAccsFromLilyPond(QString keySig, CAKeySignature::CAMajorMinorGender gender) {
	int pitch;
	if (gender==CAKeySignature::Major)
		pitch = keySig[0].toLatin1()-'a' - 2;
	else if (gender==CAKeySignature::Minor)
		pitch = keySig[0].toLatin1()-'a';
	
	signed char accs = (pitch*2)%7 * (pitch<0?(-1):1);
	if (keySig[0]=='f') accs -= 7;
	
	QString key(keySig);
	accs -= 7*keySig.count("as");
	accs -= 7*keySig.count("es");
	accs += 7*keySig.count("is");
	
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
