/** @file canorusml.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Georg Rudolph, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QtXml>

#include "ui/mainwin.h"	//needed for program version global constant

#include "core/canorusml.h"
#include "core/sheet.h"
#include "core/context.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/note.h"
#include "core/clef.h"
#include "core/muselement.h"

CACanorusML::CACanorusML() {
}

CACanorusML::~CACanorusML() {
}

void CACanorusML::saveDocument(QTextStream& out, CADocument *doc) {
	out << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	
	//CADocument start
	out << "<document";
	if (!doc->title().isEmpty())
		out << " title=\"" << doc->title() << "\"";
	if (!doc->composer().isEmpty())
		out << " composer=\"" << doc->composer() << "\"";
	out << ">\n";
	
	for (int sheetIdx=0; sheetIdx < doc->sheetCount(); sheetIdx++) {
		//CASheet start
		out << "<sheet";
		out << " name=\"" << doc->sheetAt(sheetIdx)->name() << "\"";
		out << ">\n";
		
		for (int contextIdx=0; contextIdx < doc->sheetAt(sheetIdx)->contextCount(); contextIdx++) {
			//(CAContext)
			CAContext *c = doc->sheetAt(sheetIdx)->contextAt(contextIdx);
			
			switch (c->contextType()) {
				case CAContext::Staff:
					//CAStaff
					CAStaff *staff = (CAStaff*)c;
					out << "<staff instrumentName=\"" << staff->instrumentName() << "\">\n";
					
					for (int voiceIdx=0; voiceIdx < staff->voiceCount(); voiceIdx++) {
						//CAVoice
						CAVoice *v = staff->voiceAt(voiceIdx);
						out << "<voice name=\"" << v->name() << "\" midiChannel=\"" << v->midiChannel() << "\" midiProgram=\"" << v->midiProgram() << "\">\n";

						out << createMLVoice(v);	//write down the actual contents of the voice
						
						//CAVoice end
						out << "</voice>\n";
					}
					
					//CAStaff end
					out << "</staff>\n";
					break;
			}
			
			
		}
		
		//CASheet end
		out << "</sheet>\n";
	}
	
	//CADocument end
	out << "</document>\n";
}

void CACanorusML::openDocument(QTextStream& in, CADocument *doc) {
}

const QString CACanorusML::createMLVoice(CAVoice *v) {
	QString voiceString;
	int lastPitch = -1;
	int lastLength = -1;
	
	for (int i=0; i<v->musElementCount(); i++) {
		//(CAMusElement)
		switch (v->musElementAt(i)->musElementType()) {
			case CAMusElement::Clef: {
				//CAClef
				CAClef *clef = (CAClef*)v->musElementAt(i);
				voiceString += "<clef>";
				switch (clef->clefType()) {
					case CAClef::Treble:
						voiceString += "treble";
						break;
				}
				voiceString += "</clef>";
			
				break;
			}
			case CAMusElement::Note: {
				//CANote
				CANote *note = (CANote*)v->musElementAt(i);
				voiceString += note->pitchString();
				
				int delta = lastPitch - note->pitch();
				if (delta < 0) delta *= (-1);
				
				voiceString += " ";
				lastPitch = note->pitch();
				//lastLength = note->lengthString();
			}
		}
	}
}
