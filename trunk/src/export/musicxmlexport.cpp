/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>
#include <QDomDocument>
#include <QDomImplementation>
#include <QDomElement>
#include <QTextStream>

#include "export/musicxmlexport.h"

#include "score/document.h"
#include "score/sheet.h"
#include "score/context.h"
#include "score/staff.h"
#include "score/voice.h"
#include "score/note.h"
#include "score/rest.h"
#include "score/clef.h"
#include "score/muselement.h"
#include "score/keysignature.h"
#include "score/timesignature.h"
#include "score/barline.h"

#include "score/mark.h"
#include "score/text.h"
#include "score/tempo.h"
#include "score/bookmark.h"
#include "score/articulation.h"
#include "score/crescendo.h"
#include "score/instrumentchange.h"
#include "score/dynamic.h"
#include "score/ritardando.h"
#include "score/fermata.h"
#include "score/repeatmark.h"
#include "score/fingering.h"

#include "score/lyricscontext.h"
#include "score/syllable.h"

#include "score/functionmarkcontext.h"
#include "score/functionmark.h"

CAMusicXmlExport::CAMusicXmlExport( QTextStream *stream )
 : CAExport(stream) {
	_xmlDoc = 0;
}

CAMusicXmlExport::~CAMusicXmlExport() {
}

/*!
	Exports the document to MusicXML 3.0 format.
	It uses DOM object internally for writing the XML output.
 
	The implementation relies heavily on the tutorial found at musicxml.com.
 */
void CAMusicXmlExport::exportSheetImpl(CASheet *sheet) {
	out().setCodec("UTF-8");
	setCurSheet( sheet );

	// we need to check if the document is not set, for example at exporting the first sheet
	if (sheet->document()) {
		setCurDocument( sheet->document() );
	}
	
	// DOCTYPE
	QDomImplementation di;
	QDomDocument xmlDoc(
		di.createDocumentType("score-partwise", "-//Recordare//DTD MusicXML 3.0 Partwise//EN", "http://www.musicxml.org/dtds/partwise.dtd")
	);
	_xmlDoc = &xmlDoc;

	// Add encoding
	xmlDoc.appendChild(xmlDoc.createProcessingInstruction("xml",
			"version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\""));

	// Root node - <canorus-document>
	QDomElement xmlScorePartwise = xmlDoc.createElement("score-partwise");
	xmlScorePartwise.setAttribute("version", "3.0");
	
	QDomElement xmlPartList = xmlDoc.createElement("part-list");
	QList<CAStaff*> staffList = sheet->staffList();
	
	// first export part information
	for (int i=0; i<staffList.size(); i++) {
		QDomElement xmlScorePart = xmlDoc.createElement("score-part");
		xmlScorePart.setAttribute("id", QString("P")+QString::number(i+1));
		
		QDomElement xmlPartName = xmlDoc.createElement("part-name");
		QDomText xmlPartNameText = xmlDoc.createTextNode(staffList[i]->name());
		xmlPartName.appendChild(xmlPartNameText);
		xmlScorePart.appendChild(xmlPartName);
		
		xmlPartList.appendChild(xmlScorePart);
	}
	xmlScorePartwise.appendChild(xmlPartList);
	
	// then export the part content
	for (int i=0; i<staffList.size(); i++) {
		QDomElement xmlPart = xmlDoc.createElement("part");
		xmlPart.setAttribute("id", QString("P")+QString::number(i+1));
		exportStaffImpl( staffList[i], xmlPart );
		xmlScorePartwise.appendChild(xmlPart);
	}
	
	xmlDoc.appendChild(xmlScorePartwise);
	out() << xmlDoc.toString();
}

/*!
 * Exports the given staff to the provided DOM part element.
 */
void CAMusicXmlExport::exportStaffImpl(CAStaff* staff, QDomElement& xmlPart) {
	int measureNumber = 1;
	int voicesFinished=0;
	
	QList<CAVoice*> voiceList = staff->voiceList();
	int *curIndex = new int[voiceList.size()]; // frontline of exported elements
	for (int i=0; i<voiceList.size(); i++) { curIndex[i]=0; }
	
	while (voicesFinished < voiceList.size()) {
		// write the measure content
		QDomElement xmlMeasure = _xmlDoc->createElement("measure");
		xmlMeasure.setAttribute("number",measureNumber);
		
		exportMeasure(voiceList, curIndex, xmlMeasure);
		
		xmlPart.appendChild(xmlMeasure);
		
		// check the end of staff
		voicesFinished=0;
		for (int i=0; i<voiceList.size(); i++) {
			if (curIndex[i]>=voiceList[i]->musElementList().size()-1) {
				// voice is finished, if a final barline is reached (curIndex=size()-1)
				// or, if a final note is reached and there is no barline afterwards (curIndex=size())
				voicesFinished++;
			}
		}
		
		measureNumber++;
	}
}

/*!
 * Exports the voice elements at provided indices to the given DOM measure
 * element.
 */
void CAMusicXmlExport::exportMeasure(QList<CAVoice*>& voiceList, int *curIndex, QDomElement& xmlMeasure) {
	QList<CAMusElement*> attributeChanges;
	
	// find the target barline which closes the measure
	// since barlines are common to all voices, scanning the first voice suffices
	// meanwhile, remember any clef/key/time changes
	CABarline *targetBarline = 0;
	int j=curIndex[0]+1;
	while (j<voiceList[0]->musElementList().size() &&
			voiceList[0]->musElementList()[j]->musElementType()!=CAMusElement::Barline ) {
		CAMusElement::CAMusElementType t = voiceList[0]->musElementList()[j-1]->musElementType();
		
		if (t==CAMusElement::Clef || t==CAMusElement::TimeSignature || t==CAMusElement::KeySignature) {
			attributeChanges << voiceList[0]->musElementList()[j-1];
		}
		j++;
	}
	
	if (j<voiceList[0]->musElementList().size()) {
		targetBarline = static_cast<CABarline*>(voiceList[0]->musElementList()[j]);
	}

	// check for attributes changes in the first pass
	QDomElement xmlAttributes = _xmlDoc->createElement("attributes");
	
	QDomElement xmlDivisions = _xmlDoc->createElement("divisions");
	QDomText xmlDivisionsText = _xmlDoc->createTextNode(QString::number(32)); // 32 divisions per quarter gives us 128th - the shortest Canorus length
	xmlDivisions.appendChild(xmlDivisionsText);
	xmlAttributes.appendChild(xmlDivisions);
	
	for (int i=0; i<attributeChanges.size(); i++) {
		switch (attributeChanges[i]->musElementType()) {
			case CAMusElement::Clef: {
				QDomElement xmlClef = _xmlDoc->createElement("clef");
				exportClef(static_cast<CAClef*>(attributeChanges[i]), xmlClef);
				xmlAttributes.appendChild(xmlClef);
				break;
			}

			case CAMusElement::TimeSignature: {
				QDomElement xmlTimeSig = _xmlDoc->createElement("time");
				exportTimeSig(static_cast<CATimeSignature*>(attributeChanges[i]), xmlTimeSig);
				xmlAttributes.appendChild(xmlTimeSig);
				break;
			}

			case CAMusElement::KeySignature: {
				QDomElement xmlKeySig = _xmlDoc->createElement("key");
				exportKeySig(static_cast<CAKeySignature*>(attributeChanges[i]), xmlKeySig);
				xmlAttributes.appendChild(xmlKeySig);
				break;
			}

			default: {
				break;
			}
		}
	}
	xmlMeasure.appendChild(xmlAttributes);
	
	// TODO: check for dynamics (mf, pp)
	
	// export notes and rests
	for (int i=0; i<voiceList.size(); i++) {
		CAVoice *v = voiceList[i];
		while (curIndex[i]<v->musElementList().size() &&
		       v->musElementList()[curIndex[i]]!=targetBarline) {
			if (v->musElementList()[curIndex[i]]->isPlayable()) {
				CAMusElement *elt = v->musElementList()[curIndex[i]];
				QDomElement xmlNote = _xmlDoc->createElement("note");
				
				QDomElement xmlDuration = _xmlDoc->createElement("duration");
				// duration=timeLength/8 comes from the hardcoded divisions (set to 32)
				int duration = CAPlayableLength::playableLengthToTimeLength(static_cast<CAPlayable*>(elt)->playableLength()) / 8;
				QDomText xmlDurationValue = _xmlDoc->createTextNode(QString::number(duration));
				xmlDuration.appendChild(xmlDurationValue);
				xmlNote.appendChild(xmlDuration);
				
				for (int j=0; j<static_cast<CAPlayable*>(elt)->playableLength().dotted(); j++) {
					QDomElement xmlDot = _xmlDoc->createElement("dot");
					xmlNote.appendChild(xmlDot);
				}
				
				QDomElement xmlVoice = _xmlDoc->createElement("voice");
				QDomText xmlVoiceNr = _xmlDoc->createTextNode( QString::number(v->voiceNumber()) );
				xmlVoice.appendChild(xmlVoiceNr);
				xmlNote.appendChild(xmlVoice);

				if (elt->musElementType()==CAMusElement::Note) {
					exportNote(static_cast<CANote*>(elt), xmlNote);
				} else
				if (elt->musElementType()==CAMusElement::Rest) {
					exportRest(static_cast<CARest*>(elt), xmlNote);
				}
				xmlMeasure.appendChild(xmlNote);
			}
			curIndex[i]++;
		}
	}
}

void CAMusicXmlExport::exportClef(CAClef* clef, QDomElement& xmlClef) {
	QString sign;
	int line=0;
	switch (clef->clefType()) {
		case CAClef::G: sign = "G"; line=2; break;
		case CAClef::F: sign = "F"; line=4; break;
		case CAClef::PercussionHigh: sign = "percussion"; line=0; break;
		case CAClef::PercussionLow: sign = "percussion"; line=0; break;
		case CAClef::Tab: sign = "TAB"; line=5; break;
		case CAClef::C: sign = "C"; line = (clef->c1()+clef->offset())/2 + 1; break;
		default: break;
	}
	if (sign.size()) {
		QDomElement xmlSign = _xmlDoc->createElement("sign");
		QDomText xmlSignValue = _xmlDoc->createTextNode( sign );
		xmlSign.appendChild(xmlSignValue);
		xmlClef.appendChild(xmlSign);
	}
	
	if (line) {
		QDomElement xmlLine = _xmlDoc->createElement("line");
		QDomText xmlLineValue = _xmlDoc->createTextNode( QString::number(line) );
		xmlLine.appendChild(xmlLineValue);
		xmlClef.appendChild(xmlLine);
	}
		
	if (clef->offset()) {
		QDomElement xmlClefOctaveChange = _xmlDoc->createElement("clef-octave-change");
		QDomText xmlClefOctaveChangeValue = _xmlDoc->createTextNode( QString::number(clef->offset()/8) );
		xmlClefOctaveChange.appendChild(xmlClefOctaveChangeValue);
		xmlClef.appendChild(xmlClefOctaveChange);
	}
}

void CAMusicXmlExport::exportTimeSig(CATimeSignature* time, QDomElement& xmlTime) {
	QDomElement xmlBeats = _xmlDoc->createElement("beats");
	QDomText xmlBeatsValue = _xmlDoc->createTextNode( QString::number(time->beats()) );
	xmlBeats.appendChild(xmlBeatsValue);
	xmlTime.appendChild(xmlBeats);
	
	QDomElement xmlBeatType = _xmlDoc->createElement("beat-type");
	QDomText xmlBeatTypeValue = _xmlDoc->createTextNode( QString::number(time->beat()) );
	xmlBeatType.appendChild(xmlBeatTypeValue);
	xmlTime.appendChild(xmlBeatType);
}

void CAMusicXmlExport::exportKeySig(CAKeySignature* key, QDomElement& xmlKey) {
	QDomElement xmlFifths = _xmlDoc->createElement("fifths");
	QDomText xmlFifthsValue = _xmlDoc->createTextNode( QString::number(key->diatonicKey().numberOfAccs()) );
	xmlFifths.appendChild(xmlFifthsValue);
	xmlKey.appendChild(xmlFifths);
	
	QString mode;
	if (key->diatonicKey().gender()==CADiatonicKey::Major) {
		mode = "major";
	} else
	if (key->diatonicKey().gender()==CADiatonicKey::Minor) {
		mode = "minor";
	}
	if (mode.size()) {
		QDomElement xmlMode = _xmlDoc->createElement("mode");
		QDomText xmlModeValue = _xmlDoc->createTextNode( mode );
		xmlMode.appendChild(xmlModeValue);
		xmlKey.appendChild(xmlMode);
	}
}

void CAMusicXmlExport::exportNote(CANote* note, QDomElement& xmlNote) {
	if (note->isPartOfChord() && !note->isFirstInChord()) {
		QDomElement xmlChord = _xmlDoc->createElement("chord");
		xmlNote.appendChild(xmlChord);
	}
	
	QString stemDirection;
	if (note->stemDirection()==CANote::StemUp ||
	    (note->stemDirection()==CANote::StemPreferred && note->voice()->stemDirection()==CANote::StemUp)) {
		stemDirection = "up";
	} else
	if (note->stemDirection()==CANote::StemDown ||
	    (note->stemDirection()==CANote::StemPreferred && note->voice()->stemDirection()==CANote::StemDown)) {
		stemDirection = "down";
	}
	if (stemDirection.size()) {
		QDomElement xmlStemDirection = _xmlDoc->createElement("stem");
		QDomText xmlStemDirectionValue = _xmlDoc->createTextNode(stemDirection);
		xmlStemDirection.appendChild(xmlStemDirectionValue);
		xmlNote.appendChild(xmlStemDirection);
	}
	
	QDomElement xmlPitch = _xmlDoc->createElement("pitch");
	QDomElement xmlStep = _xmlDoc->createElement("step");
	QDomText xmlStepValue = _xmlDoc->createTextNode(QChar(static_cast<char>((note->diatonicPitch().noteName()+2)%7 + 'A')));
	xmlStep.appendChild(xmlStepValue);
	xmlPitch.appendChild(xmlStep);
	if (note->diatonicPitch().accs()) {
		QDomElement xmlAlter = _xmlDoc->createElement("alter");
		QDomText xmlAlterValue = _xmlDoc->createTextNode( QString::number(note->diatonicPitch().accs()) );
		xmlAlter.appendChild(xmlAlterValue);
		xmlPitch.appendChild(xmlAlter);
	}
	QDomElement xmlOctave = _xmlDoc->createElement("octave");
	QDomText xmlOctaveValue = _xmlDoc->createTextNode( QString::number(note->diatonicPitch().noteName()/7) );
	xmlOctave.appendChild(xmlOctaveValue);
	xmlPitch.appendChild(xmlOctave);
	xmlNote.appendChild(xmlPitch);
	
	QString type;
	switch ( note->playableLength().musicLength() ) {
		case CAPlayableLength::Breve: type = "breve"; break;
		case CAPlayableLength::Whole: type = "whole"; break;
		case CAPlayableLength::Half: type = "half"; break;
		case CAPlayableLength::Quarter: type = "quarter"; break;
		case CAPlayableLength::Eighth: type ="eighth"; break;
		case CAPlayableLength::Sixteenth: type = "16th"; break;
		case CAPlayableLength::ThirtySecond: type = "32nd"; break;
		case CAPlayableLength::SixtyFourth: type = "64th"; break;
		case CAPlayableLength::HundredTwentyEighth: type = "128th"; break;
		default: break;
	}
	if (type.size()) {
		QDomElement xmlType = _xmlDoc->createElement("type");
		QDomText xmlTypeValue = _xmlDoc->createTextNode(type);
		xmlType.appendChild(xmlTypeValue);
		xmlNote.appendChild(xmlType);
	}
}

void CAMusicXmlExport::exportRest(CARest* rest, QDomElement& xmlNote) {
	QDomElement xmlRest = _xmlDoc->createElement("rest");
	xmlNote.appendChild(xmlRest);
}
