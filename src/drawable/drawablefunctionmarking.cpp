/** @file drawable/drawablefunctionmarking.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <QFont>

#include "drawable/drawablefunctionmarking.h"
#include "drawable/drawablefunctionmarkingcontext.h"
#include "core/functionmarking.h"

////////////////////////////////////////////////////
// class CADrawableFunctionMarking
////////////////////////////////////////////////////
CADrawableFunctionMarking::CADrawableFunctionMarking(CAFunctionMarking *function, CADrawableFunctionMarkingContext *context, int x, int y)
 : CADrawableMusElement(function, context, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarking;
 	
 	_extenderLineVisible = false;
 	if (functionMarking()->tonicDegree()==CAFunctionMarking::None)
		switch (functionMarking()->function()) { //TODO: Width determination should be done automatically using QPainter::boundingRect() method
			//character widths are calculated using FreeSans font, pixelSize 19
			case CAFunctionMarking::I:		_text="I"; _width=5; break;
			case CAFunctionMarking::II:		_text="II"; _width=10; break;
			case CAFunctionMarking::III:	_text="III"; _width=15; break;
			case CAFunctionMarking::IV:		_text="IV"; _width=18; break;
			case CAFunctionMarking::V:		_text="V"; _width=13; break;
			case CAFunctionMarking::VI:		_text="VI"; _width=18; break;
			case CAFunctionMarking::VII:	_text="VII"; _width=23; break;
			case CAFunctionMarking::T:		_text="T"; _width=12; break;
			case CAFunctionMarking::S:		_text="S"; _width=13; break;
			case CAFunctionMarking::D:		_text="D"; _width=14; break;
			case CAFunctionMarking::F:		_text="F"; _width=12; break;
			case CAFunctionMarking::N:		_text="N"; _width=14; break;
			case CAFunctionMarking::L:		_text="L"; _width=11; break;
		}
	else
		switch (functionMarking()->function()) { //TODO: Width determination should be done automatically using QPainter::boundingRect() method
			//character widths are calculated using FreeSans font, pixelSize 17
			case CAFunctionMarking::I:		_text="I"; _width=5; break;
			case CAFunctionMarking::II:		_text="II"; _width=10; break;
			case CAFunctionMarking::III:	_text="III"; _width=15; break;
			case CAFunctionMarking::IV:		_text="IV"; _width=16; break;
			case CAFunctionMarking::V:		_text="V"; _width=11; break;
			case CAFunctionMarking::VI:		_text="VI"; _width=16; break;
			case CAFunctionMarking::VII:	_text="VII"; _width=21; break;
			case CAFunctionMarking::T:		_text="T"; _width=10; break;
			case CAFunctionMarking::S:		_text="S"; _width=11; break;
			case CAFunctionMarking::D:		_text="D"; _width=12; break;
			case CAFunctionMarking::F:		_text="F"; _width=10; break;
			case CAFunctionMarking::N:		_text="N"; _width=12; break;
			case CAFunctionMarking::L:		_text="L"; _width=9; break;
		}	
	
 	_fontWidth = 10;
	if (function->isMinor()) { //prepend a small circle
		_text.prepend(QString(0x02DA));
		_width+=6;
		_fontWidth+=6;
		_xPos -= 6;
	}
	
 	_height=15;
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableFunctionMarking::~CADrawableFunctionMarking() {
	
}

void CADrawableFunctionMarking::draw(QPainter *p, CADrawSettings s) {
	QFont font("FreeSans");
	font.setPixelSize((int)(19*s.z)); //use pixelSize instead of size as we want fixed font size no matter on screen DPI
	p->setPen(QPen(s.color));
	p->setFont(font);
	p->drawText(s.x, s.y+(int)(_height*s.z+0.5), _text);
	
	if (_extenderLineVisible)
		p->drawLine(s.x + (int)(_fontWidth*s.z+0.5), s.y+(int)((_height/2.0)*s.z+0.5),
		            s.x + (int)(_width*s.z+0.5), s.y+(int)((_height/2.0)*s.z+0.5));
}

CADrawableFunctionMarking *CADrawableFunctionMarking::clone() {
	return new CADrawableFunctionMarking(functionMarking(), drawableFunctionMarkingContext(), ((CAFunctionMarking*)_musElement)->isMinor()?xPos()+6:xPos(), yPos());
}

////////////////////////////////////////////////////
// class CADrawableFunctionMarkingSupport
////////////////////////////////////////////////////
CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, const QString key, CADrawableContext *c, int x, int y)
 : CADrawableMusElement(0, c, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_key = key;
	
	if (type==Key) {
		_width = 0;	//TODO: Width determination should be done automatically using QPainter::boundingRect() method
		for (int i=0; i<key.size(); i++) {	//character widths are calculated using FreeSans font, pixelSize 17
			if (key[i]=='C') _width+=12;
			else if (key[i]=='D') _width+=12;
			else if (key[i]=='E') _width+=11;
			else if (key[i]=='F') _width+=10;
			else if (key[i]=='G') _width+=13;
			else if (key[i]=='A') _width+=11;
			else if (key[i]=='B') _width+=11;
			else if (key[i]=='c') _width+=9;
			else if (key[i]=='d') _width+=9;
			else if (key[i]=='e') _width+=9;
			else if (key[i]=='f') _width+=5;
			else if (key[i]=='g') _width+=9;
			else if (key[i]=='a') _width+=9;
			else if (key[i]=='b') _width+=9;
			else if (key[i]=='i') _width+=5;
			else if (key[i]=='s') _width+=9;
		}
		_width += 5;	//colon after the key name (:)
		_height = 14;
	}
	
	_neededWidth = _width;
	_neededHeight = _height;
	_selectable = false;
}

CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, CAFunctionMarking::CAFunctionType chordArea, bool minor, CADrawableContext *c, int x, int y)
 : CADrawableMusElement(0, c, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_chordAreaMinor = minor;
	_chordArea = chordArea;
	
	//character widths are calculated using FreeSans font, pixelSize 17
	//TODO: Width determination should be done automatically using QPainter::boundingRect() method
	if (chordArea==CAFunctionMarking::T) { _width=10; }
	else if (chordArea==CAFunctionMarking::S) { _width=11; }
	else if (chordArea==CAFunctionMarking::D) { _width=12; }
	
	_width += 12;	//paranthesis
	if (_chordAreaMinor)
		_width += 6;
	_height = 14;
	_neededWidth = _width;
	_neededHeight = _height;
	_selectable = false;
}

CADrawableFunctionMarkingSupport::~CADrawableFunctionMarkingSupport() {
}

void CADrawableFunctionMarkingSupport::draw(QPainter *p, const CADrawSettings s) {
	QFont font("FreeSans");
	font.setPixelSize((int)(17*s.z));
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			p->drawText(s.x, s.y+(int)(_height*s.z+0.5), _key+":");
			break;
		case ChordArea:
			QString text;
			switch (_chordArea) {
				case CAFunctionMarking::T: text="T"; break;
				case CAFunctionMarking::S: text="S"; break;
				case CAFunctionMarking::D: text="D"; break;
			}
			if (_chordAreaMinor) {
				text.prepend(QString(0x02DA));
			}
			p->drawText(s.x, s.y+(int)(_height*s.z+0.5), QString("(")+text+")");
			break;			
	}
}

CADrawableFunctionMarkingSupport *CADrawableFunctionMarkingSupport::clone() {
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			return new CADrawableFunctionMarkingSupport(Key, _key, _drawableContext, _xPos, _yPos);
			break;
		case ChordArea:
			return new CADrawableFunctionMarkingSupport(Key, _chordArea, _chordAreaMinor, _drawableContext, _xPos, _yPos);
			break;
	}
}
