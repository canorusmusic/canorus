/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QFont>

#include "drawable/drawablefunctionmarking.h"
#include "drawable/drawablefunctionmarkingcontext.h"
#include "core/functionmarking.h"

/*!
	\class CADrawableFunctionMarking
	\brief Drawable class for function markings
*/

CADrawableFunctionMarking::CADrawableFunctionMarking(CAFunctionMarking *function, CADrawableFunctionMarkingContext *context, int x, int y)
 : CADrawableMusElement(function, context, x, y) {
 	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarking;
 	
 	_extenderLineVisible = false;
 	_extenderLineOnly = false;
 	CAMusElement *prevMusElt;
 	// function marking is stable
 	if (// tonic degree is tonic
 	    functionMarking()->tonicDegree()==CAFunctionMarking::T &&
 		// and the function marking is not part of the ellipse
 		(!functionMarking()->isPartOfEllipse())
 	)
		switch (functionMarking()->function()) { /// \todo Width determination should be done automatically using QPainter::boundingRect() method
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
			case CAFunctionMarking::K:		_text="K"; _width=12; break;
		}
	else
	// function marking is not stable - its tonic degree is not Tonic
		switch (functionMarking()->function()) { /// \todo Width determination should be done automatically using QPainter::boundingRect() method
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
			case CAFunctionMarking::K:		_text="K"; _width=10; break;
		}	
	
	if (function->isMinor()) { //prepend a small circle
		_text.prepend(QString(0x02DA));
		_width+=6;
		_xPos -= 6;
	}
	
 	_height=15;
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableFunctionMarking::~CADrawableFunctionMarking() {
	
}

void CADrawableFunctionMarking::draw(QPainter *p, CADrawSettings s) {
	int rightBorder = s.x + qRound( _width * s.z);
	
	if ( !isExtenderLineOnly() ) { 
		QFont font("FreeSans");
		if (functionMarking()->tonicDegree()==CAFunctionMarking::T)
			font.setPixelSize( qRound(19*s.z) );
		else
			font.setPixelSize( qRound(17*s.z) );
		
		p->setPen(QPen(s.color));
		p->setFont(font);
		p->drawText(s.x, s.y + qRound( _height*s.z ), _text);
		s.x += qRound(p->boundingRect(0,0,0,0,0,_text).width() + 1*s.z);
	}
	
	if ( isExtenderLineVisible() )
		p->drawLine( s.x, s.y+qRound( (_height/2.0)*s.z ),
		             rightBorder, s.y+qRound( (_height/2.0)*s.z ) );
}

CADrawableFunctionMarking *CADrawableFunctionMarking::clone(CADrawableContext* newContext) {
	return new CADrawableFunctionMarking(functionMarking(), (newContext)?(CADrawableFunctionMarkingContext*)newContext:drawableFunctionMarkingContext(), ((CAFunctionMarking*)_musElement)->isMinor()?xPos()+6:xPos(), yPos());
}

/*!
	\class CADrawableFunctionMarkingSupport
	\brief Rectangles, key names, numbers below/above function, lines etc.
*/
// KeyName constructor
CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, const QString key, CADrawableContext *c, int x, int y)
 : CADrawableMusElement(0, c, x, y) {	// support functions point to no music element
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_key = key;
	_function1=0; _function2=0;
	
	if (type==Key) {
		_width = 0;	/// \todo Width determination should be done automatically using QPainter::boundingRect() method
		for (int i=0; i<key.size(); i++) {	// character widths are calculated using FreeSans font, pixelSize 17
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
		_width += 5;	// colon after the key name (:)
		_height = 14;
	}
	
	_neededWidth = _width;
	_neededHeight = _height;
	setSelectable( false );
}

CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, CADrawableFunctionMarking *f1, CADrawableContext *c, int x, int y, CADrawableFunctionMarking *f2)
 : CADrawableMusElement(0, c, x, y) {	// support functions point to no music element
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_function1 = f1;
	_function2 = f2;
	_width=0;
	_extenderLineVisible = false;
	_rectWider = false;
	
	if (f1->functionMarking()->isTonicDegreeMinor()) {
		_width+=6;
	}
	if (type==ChordArea) {
		// character widths are calculated using FreeSans font, pixelSize 17
		/// \todo Width determination should be done automatically using QPainter::boundingRect() method
		if (f1->functionMarking()->chordArea()==CAFunctionMarking::T) { _width=10; }
		else if (f1->functionMarking()->chordArea()==CAFunctionMarking::S) { _width=11; }
		else if (f1->functionMarking()->chordArea()==CAFunctionMarking::D) { _width=12; }
		
		_width += 12;	//paranthesis
		_height = 14;
	} else if (type==Tonicization) {
		// character widths are calculated using FreeSans font, pixelSize 19
		/// \todo Width determination should be done automatically using QPainter::boundingRect() method
		if (!f2) {
			switch (f1->functionMarking()->tonicDegree()) {
				case CAFunctionMarking::I:		_width+=5; break;
				case CAFunctionMarking::II:		_width+=10; break;
				case CAFunctionMarking::III:	_width+=15; break;
				case CAFunctionMarking::IV:		_width+=16; break;
				case CAFunctionMarking::V:		_width+=11; break;
				case CAFunctionMarking::VI:		_width+=16; break;
				case CAFunctionMarking::VII:	_width+=21; break;
				case CAFunctionMarking::T:		_width+=10; break;
				case CAFunctionMarking::S:		_width+=11; break;
				case CAFunctionMarking::D:		_width+=12; break;
			}
		} else {
			_width = f2->xPos() + f2->width() - f1->xPos();		// multiple tonicization
		}
		_height = 15;
	} else if (type==Rectangle) {
		if (f2->xPos()+f2->width()-f1->xPos() < f1->xPos()+f1->width()-f2->xPos())
			_width = f1->xPos()+f1->width()-f2->xPos() + 6;	// used for vertical modulation
		else 
			_width = f2->xPos()+f2->width()-f1->xPos() + 6;	// used for horizontal modulation/chordarea rectangle
		
		_xPos -= 3;
		_height = (f2->yPos()+f2->height()-f1->yPos()+6>f1->yPos()+f1->height()-f2->yPos()+6)?f2->yPos()+f2->height()-f1->yPos()+6:f1->yPos()+f1->height()-f2->yPos()+6;
		_yPos = f2->yPos()<f1->yPos()?f2->yPos()-3:f1->yPos()-3;
	} else if (type==Ellipse) {
		_width = f2->xPos()+f2->width()-f1->xPos();
		_height = 14;
	}
	
	_neededWidth = _width;
	_neededHeight = _height;
	setSelectable( false );
}

// Alterations constructor
CADrawableFunctionMarkingSupport::CADrawableFunctionMarkingSupport(CADrawableFunctionMarkingSupportType type, CAFunctionMarking *function, CADrawableContext *c, int x, int y)
 : CADrawableMusElement(function, c, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkingSupport;
	_drawableFunctionMarkingSupportType = type;
	_function1=0;
	_function2=0;
	
	_extenderLineVisible = false;
	_rectWider = false;
	
	_height=function->addedDegrees().size()*13 + function->alteredDegrees().size()*8;
	
	if (function->function()==CAFunctionMarking::Undefined)	// paranthesis needed as well
		_width=9+(int)(0.6*_height+0.5);
	else
		_width=6;
	
	
	_neededWidth = _width;
	_neededHeight = _height;
	setSelectable( false );
}

CADrawableFunctionMarkingSupport::~CADrawableFunctionMarkingSupport() {
}

void CADrawableFunctionMarkingSupport::draw(QPainter *p, const CADrawSettings s) {
	QFont font("FreeSans");
	QString text;
	CAFunctionMarking::CAFunctionType type;
	bool minor;
	
	//prepare drawing stuff
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			font.setPixelSize(qRound( 17*s.z ));
			break;
		case ChordArea:
			font.setPixelSize(qRound( 17*s.z ));
			type = _function1->functionMarking()->chordArea();
			minor=_function1->functionMarking()->isChordAreaMinor();
			break;
		case Tonicization:
			font.setPixelSize(qRound( 19*s.z ));
			type = _function1->functionMarking()->tonicDegree();
			minor=_function1->functionMarking()->isTonicDegreeMinor();
			break;
		case Ellipse:
			font.setPixelSize(qRound( 14*s.z ));
			break;
		case Alterations:
			font.setPixelSize(qRound( 9*s.z ));
			break;
	}
	
	// fill in the text values for functions
 	if (_drawableFunctionMarkingSupportType==Tonicization || _drawableFunctionMarkingSupportType==ChordArea) {
		switch (type) {
			// character widths are calculated using FreeSans font, pixelSize 19
			case CAFunctionMarking::I:		text="I"; break;
			case CAFunctionMarking::II:		text="II"; break;
			case CAFunctionMarking::III:	text="III"; break;
			case CAFunctionMarking::IV:		text="IV"; break;
			case CAFunctionMarking::V:		text="V"; break;
			case CAFunctionMarking::VI:		text="VI"; break;
			case CAFunctionMarking::VII:	text="VII"; break;
			case CAFunctionMarking::T:		text="T"; break;
			case CAFunctionMarking::S:		text="S"; break;
			case CAFunctionMarking::D:		text="D"; break;
			case CAFunctionMarking::F:		text="F"; break;
			case CAFunctionMarking::N:		text="N"; break;
			case CAFunctionMarking::L:		text="L"; break;
			case CAFunctionMarking::K:		text="K"; break;
		}
		
		if (minor)
			text.prepend(QString(0x02DA));
 	}
 	
	p->setPen(QPen(s.color));
	p->setFont(font);
	
	// draw
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			p->drawText(s.x, s.y+(int)(_height*s.z+0.5), _key+":");
			break;
		case ChordArea:
			p->drawText(s.x, s.y+(int)(_height*s.z+0.5), QString("(")+text+")");
			break;
		case Tonicization:
			if (!_function2) {	// tonicization is below a single function
				p->drawText(s.x, s.y+(int)(_height*s.z+0.5), text);
				if (_extenderLineVisible)
					p->drawLine(s.x + p->boundingRect(0,0,0,0,0,text).width(), (int)(s.y+height()*s.z/2.0+0.5),
								(int)(s.x + width()*s.z+0.5), (int)(s.y+height()*s.z/2.0+0.5));
			} else {
				p->drawText(	// tonicization is below multiple functions
					// get the x-coordinate where to start text rendering
					(int)(s.x + (_function2->xPos() + _function2->width() - _function1->xPos())*s.z/2.0 - p->boundingRect(0,0,0,0,0,text).width()/2.0 + 0.5),
					s.y+(int)(_height*s.z+0.5),
					text
				);
				// draw line below side functions
				p->drawLine(s.x, s.y, (int)(s.x + (_function2->xPos()+_function2->width()-_function1->xPos())*s.z+0.5), s.y);
				
				// draw extender line of the support function
				if (_extenderLineVisible)
					p->drawLine(s.x + (int)((_function2->xPos()+_function2->width()-_function1->xPos())*s.z/2.0 + p->boundingRect(0,0,0,0,0,text).width()/2.0+1*s.z), (int)(s.y+height()*s.z/2.0+0.5),
								(int)(s.x + width()*s.z+0.5), (int)(s.y+height()*s.z/2.0+0.5));
				
			}
			break;
		case Ellipse:
			// draw vertical lines
			p->drawLine(s.x, (int)(s.y+2*s.z+0.5), s.x, (int)(s.y+height()*s.z/2.0+0.5));	//left vertical line
			p->drawLine((int)(s.x+width()*s.z+0.5), (int)(s.y+2*s.z+0.5), (int)(s.x+width()*s.z+0.5), (int)(s.y+height()*s.z/2.0+0.5));	//right vertical line
			// draw horizontal lines
			p->drawLine(s.x, (int)(s.y+height()*s.z/2.0+0.5), (int)(s.x+width()*s.z/2.0 - p->boundingRect(0,0,0,0,0,"E").width()/2.0 - 2*s.z + 0.5), (int)(s.y+height()*s.z/2.0+0.5));	//left horizontal line
			p->drawLine((int)(s.x+width()*s.z/2.0 + p->boundingRect(0,0,0,0,0,"E").width()/2.0 + 2*s.z + 0.5), (int)(s.y+height()*s.z/2.0+0.5), (int)(s.x+width()*s.z+0.5), (int)(s.y+height()*s.z/2.0+0.5));	//right horizontal line
			
			p->drawText((int)(s.x+width()*s.z/2.0 - p->boundingRect(0,0,0,0,0,"E").width()/2.0 + 0.5), (int)(s.y+height()*s.z-1*s.z+0.5), "E");
			
			break;
			
		case Rectangle:
			p->drawRect(s.x, s.y, (int)(width()*s.z+0.5), (int)(height()*s.z+0.5));
			break;
		
		case Alterations:
			CAFunctionMarking *f1 = (CAFunctionMarking*)(_musElement);
			int curX = s.x, curY = s.y;
			
			if (f1->function()==CAFunctionMarking::Undefined)
				curX+=(int)(0.3*_height*s.z+0.5);	// make space for left paranthesis, if needed
			// draw added degrees
			for (int i=0; i<f1->addedDegrees().size(); i++) {
				// draw upper number
				p->drawText((int)(curX+2*s.z+0.5), (int)(curY+8*s.z+0.5), f1->addedDegrees().at(i)<0?QString::number(f1->addedDegrees().at(i)*(-1)):QString::number(f1->addedDegrees().at(i)));
				// draw additional + or - for added or substracted degree
				p->drawText((int)(curX+2*s.z+0.5), (int)(curY+13*s.z+0.5), f1->addedDegrees().at(i)<0?"-":"+");
				curY+=(int)(13*s.z+0.5);
			}
			
			// draw altered degrees
			for (int i=0; i<f1->alteredDegrees().size(); i++) {
				//draw numbers with + or - prefixed
				p->drawText(curX, (int)(curY+8*s.z+0.5), f1->alteredDegrees().at(i)>0?QString("+")+QString::number(+f1->alteredDegrees().at(i)):QString::number(f1->alteredDegrees().at(i)));
				curY+=(int)(8*s.z+0.5);
			}
			
			// draw paranthesis, if needed
			if (f1->function()==CAFunctionMarking::Undefined) {
				curX-=(int)(0.3*_height*s.z+0.5);
				font.setPixelSize((int)(_height*s.z+0.5));
				p->setFont(font);
				p->drawText(curX, (int)(curY-_height*s.z/6.0+0.5), "(");
				p->drawText((int)(curX+(_width-0.3*_height)*s.z+0.5), (int)(curY-_height*s.z/6.0+0.5), ")");
			}
	}
}

CADrawableFunctionMarkingSupport *CADrawableFunctionMarkingSupport::clone(CADrawableContext* newContext) {
	switch (_drawableFunctionMarkingSupportType) {
		case Key:
			return new CADrawableFunctionMarkingSupport(Key, _key, (newContext)?newContext:_drawableContext, _xPos, _yPos);
			break;
		case ChordArea:
		case Tonicization:
			return new CADrawableFunctionMarkingSupport(_drawableFunctionMarkingSupportType, _function1, (newContext)?newContext:_drawableContext, _xPos, _yPos, _function2);
			break;
		case Ellipse:
			return new CADrawableFunctionMarkingSupport(Ellipse, _function1, (newContext)?newContext:_drawableContext, (int)(_xPos - _function1->width()/2.0+0.5), _yPos, _function2);
		case Rectangle:
			return new CADrawableFunctionMarkingSupport(Rectangle, _function1, (newContext)?newContext:_drawableContext, _xPos+3, _yPos+3, _function2);
		case Alterations:
			return new CADrawableFunctionMarkingSupport(Alterations, (CAFunctionMarking*)(_musElement), (newContext)?newContext:_drawableContext, _xPos, _yPos);
	}
}
