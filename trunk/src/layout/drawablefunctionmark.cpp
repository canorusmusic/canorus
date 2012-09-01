/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QFont>
#include <stdio.h>

#include "layout/drawablefunctionmark.h"
#include "layout/drawablefunctionmarkcontext.h"
#include "score/functionmark.h"

/*!
	\class CADrawableFunctionMark
	\brief The drawable instance of the function mark.
	These music elements are ordinary music elements selectable by the user.

	\sa CADrawableFunctionMarkSupport
*/

CADrawableFunctionMark::CADrawableFunctionMark(CAFunctionMark *function, CADrawableFunctionMarkContext *context, double x, double y)
 : CADrawableMusElement(function, context, x, y) {
 	setDrawableMusElementType( CADrawableMusElement::DrawableFunctionMark );

 	_extenderLineVisible = false;
 	_extenderLineOnly = false;
 	//CAMusElement *prevMusElt;
 	setWidth( 11 ); // default width
 	// function mark is stable
 	if (// tonic degree is tonic
 	    functionMark()->tonicDegree()==CAFunctionMark::T &&
 		// and the function mark is not part of the ellipse
 		(!functionMark()->isPartOfEllipse())
 	)
		switch (functionMark()->function()) { /// \todo Width determination should be done automatically using QPainter::boundingRect() method
			//character widths are calculated using FreeSans font, pixelSize 19
			case CAFunctionMark::I:		_text="I"; _width=5; break;
			case CAFunctionMark::II:		_text="II"; _width=10; break;
			case CAFunctionMark::III:	_text="III"; _width=15; break;
			case CAFunctionMark::IV:		_text="IV"; _width=18; break;
			case CAFunctionMark::V:		_text="V"; _width=13; break;
			case CAFunctionMark::VI:		_text="VI"; _width=18; break;
			case CAFunctionMark::VII:	_text="VII"; _width=23; break;
			case CAFunctionMark::T:		_text="T"; _width=12; break;
			case CAFunctionMark::S:		_text="S"; _width=13; break;
			case CAFunctionMark::D:		_text="D"; _width=14; break;
			case CAFunctionMark::F:		_text="F"; _width=12; break;
			case CAFunctionMark::N:		_text="N"; _width=14; break;
			case CAFunctionMark::L:		_text="L"; _width=11; break;
			case CAFunctionMark::K:		_text="K"; _width=12; break;
			case CAFunctionMark::Undefined:
				break;
		}
	else
	// function mark is not stable - its tonic degree is not Tonic
		switch (functionMark()->function()) { /// \todo Width determination should be done automatically using QPainter::boundingRect() method
			//character widths are calculated using FreeSans font, pixelSize 17
			case CAFunctionMark::I:		_text="I"; _width=5; break;
			case CAFunctionMark::II:		_text="II"; _width=10; break;
			case CAFunctionMark::III:	_text="III"; _width=15; break;
			case CAFunctionMark::IV:		_text="IV"; _width=16; break;
			case CAFunctionMark::V:		_text="V"; _width=11; break;
			case CAFunctionMark::VI:		_text="VI"; _width=16; break;
			case CAFunctionMark::VII:	_text="VII"; _width=21; break;
			case CAFunctionMark::T:		_text="T"; _width=10; break;
			case CAFunctionMark::S:		_text="S"; _width=11; break;
			case CAFunctionMark::D:		_text="D"; _width=12; break;
			case CAFunctionMark::F:		_text="F"; _width=10; break;
			case CAFunctionMark::N:		_text="N"; _width=12; break;
			case CAFunctionMark::L:		_text="L"; _width=9; break;
			case CAFunctionMark::K:		_text="K"; _width=10; break;
			case CAFunctionMark::Undefined:
				break;
		}

	if (function->isMinor()) { //prepend a small circle
		_text.prepend(QString(0x02DA));
		setWidth( width()+6);
		setXPos( xPos()-6 );
	}

 	setHeight(15);
}

CADrawableFunctionMark::~CADrawableFunctionMark() {

}

void CADrawableFunctionMark::draw(QPainter *p, CADrawSettings s) {
	int rightBorder = s.x + qRound( width() * s.z);

	QFont font("FreeSans");
	if (functionMark()->tonicDegree()==CAFunctionMark::T)
		font.setPixelSize( qRound(19*s.z) );
	else
		font.setPixelSize( qRound(17*s.z) );

	p->setPen(QPen(s.color));
	p->setFont(font);

	if ( !isExtenderLineOnly() ) {
		p->drawText(s.x, s.y + qRound( height()*s.z ), _text);
		s.x += qRound(p->boundingRect(0,0,0,0,0,_text).width() + 1*s.z);
	}

	if ( isExtenderLineVisible() )
		p->drawLine( s.x, s.y+qRound( (height()/2.0)*s.z ),
		             rightBorder, s.y+qRound( (_height/2.0)*s.z ) );
}

CADrawableFunctionMark *CADrawableFunctionMark::clone(CADrawableContext* newContext) {
	return new CADrawableFunctionMark(functionMark(), (newContext)?(CADrawableFunctionMarkContext*)newContext:drawableFunctionMarkContext(), ((CAFunctionMark*)_musElement)->isMinor()?xPos()+6:xPos(), yPos());
}

/*!
	\class CADrawableFunctionMarkSupport
	\brief Rectangles, key names, numbers below/above function, lines etc.
	Support class which draws the key of the function, rectangle around it, chord area, ellipse etc.
	These drawable music elements aren't selectable, but they can't be drawn by a single CADrawableFunctionMark because they're usually dependent on more than one function mark.

	\sa CADrawableFunctionMark
*/

/*!
	KeyName constructor.
*/
CADrawableFunctionMarkSupport::CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType type, const QString key, CADrawableContext *c, double x, double y)
 : CADrawableMusElement(0, c, x, y) {	// support functions point to no music element
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkSupport;
	_drawableFunctionMarkSupportType = type;
	_key = key;
	_function1=0; _function2=0;

	if (type==Key) {
		setWidth( 0 );  /// \todo Width determination should be done automatically using QPainter::boundingRect() method
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

	setSelectable( false );
}

/*!
	ChordArea, Tonicization, Modulation/ChordArea Rectangle, Ellipse constructor.
*/
CADrawableFunctionMarkSupport::CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType type, CADrawableFunctionMark *f1, CADrawableContext *c, double x, double y, CADrawableFunctionMark *f2)
 : CADrawableMusElement(0, c, x, y) {	// support functions point to no music element
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkSupport;
	_drawableFunctionMarkSupportType = type;
	_function1 = f1;
	_function2 = f2;
	setWidth( 0 );
	_extenderLineVisible = false;
	_rectWider = false;

	if (f1->functionMark()->isTonicDegreeMinor()) {
		_width+=6;
	}
	if (type==ChordArea) {
		// character widths are calculated using FreeSans font, pixelSize 17
		/// \todo Width determination should be done automatically using QPainter::boundingRect() method
		if (f1->functionMark()->chordArea()==CAFunctionMark::T) { _width=10; }
		else if (f1->functionMark()->chordArea()==CAFunctionMark::S) { _width=11; }
		else if (f1->functionMark()->chordArea()==CAFunctionMark::D) { _width=12; }

		_width += 12;	//paranthesis
		_height = 14;
	} else if (type==Tonicization) {
		// character widths are calculated using FreeSans font, pixelSize 19
		/// \todo Width determination should be done automatically using QPainter::boundingRect() method
		if (!f2) {
			switch (f1->functionMark()->tonicDegree()) {
				case CAFunctionMark::I:		_width+=5; break;
				case CAFunctionMark::II:		_width+=10; break;
				case CAFunctionMark::III:	_width+=15; break;
				case CAFunctionMark::IV:		_width+=16; break;
				case CAFunctionMark::V:		_width+=11; break;
				case CAFunctionMark::VI:		_width+=16; break;
				case CAFunctionMark::VII:	_width+=21; break;
				case CAFunctionMark::T:		_width+=10; break;
				case CAFunctionMark::S:		_width+=11; break;
				case CAFunctionMark::D:		_width+=12; break;
				case CAFunctionMark::F:
				case CAFunctionMark::K:
				case CAFunctionMark::L:
				case CAFunctionMark::N:
				case CAFunctionMark::Undefined:
					fprintf(stderr,"Warning: CADrawableFunctionMarkSupport::CADrawableFunctionMarkSupport - Unhandled mark %d",f1->functionMark()->tonicDegree());
					break;
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

	setSelectable( false );
}

/*!
	Alterations constructor.
*/
CADrawableFunctionMarkSupport::CADrawableFunctionMarkSupport(CADrawableFunctionMarkSupportType type, CAFunctionMark *function, CADrawableContext *c, double x, double y)
 : CADrawableMusElement(function, c, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableFunctionMarkSupport;
	_drawableFunctionMarkSupportType = type;
	_function1=0;
	_function2=0;

	_extenderLineVisible = false;
	_rectWider = false;

	_height=function->addedDegrees().size()*13 + function->alteredDegrees().size()*8;

	if (function->function()==CAFunctionMark::Undefined)	// paranthesis needed as well
		_width=9+qRound(0.6*_height);
	else
		_width=6;


	setSelectable( false );
}

CADrawableFunctionMarkSupport::~CADrawableFunctionMarkSupport() {
}

void CADrawableFunctionMarkSupport::draw(QPainter *p, const CADrawSettings s) {
	QFont font("FreeSans");
	QString text;
	CAFunctionMark::CAFunctionType type;
	bool minor;

	//prepare drawing stuff
	switch (_drawableFunctionMarkSupportType) {
		case Key:
			font.setPixelSize(qRound( 17*s.z ));
			break;
		case ChordArea:
			font.setPixelSize(qRound( 17*s.z ));
			type = _function1->functionMark()->chordArea();
			minor=_function1->functionMark()->isChordAreaMinor();
			break;
		case Tonicization:
			font.setPixelSize(qRound( 19*s.z ));
			type = _function1->functionMark()->tonicDegree();
			minor=_function1->functionMark()->isTonicDegreeMinor();
			break;
		case Ellipse:
			font.setPixelSize(qRound( 14*s.z ));
			break;
		case Alterations:
			font.setPixelSize(qRound( 9*s.z ));
			break;
		case Rectangle:
			break;
	}

	// fill in the text values for functions
 	if (_drawableFunctionMarkSupportType==Tonicization || _drawableFunctionMarkSupportType==ChordArea) {
		switch (type) {
			// character widths are calculated using FreeSans font, pixelSize 19
			case CAFunctionMark::I:		text="I"; break;
			case CAFunctionMark::II:		text="II"; break;
			case CAFunctionMark::III:	text="III"; break;
			case CAFunctionMark::IV:		text="IV"; break;
			case CAFunctionMark::V:		text="V"; break;
			case CAFunctionMark::VI:		text="VI"; break;
			case CAFunctionMark::VII:	text="VII"; break;
			case CAFunctionMark::T:		text="T"; break;
			case CAFunctionMark::S:		text="S"; break;
			case CAFunctionMark::D:		text="D"; break;
			case CAFunctionMark::F:		text="F"; break;
			case CAFunctionMark::N:		text="N"; break;
			case CAFunctionMark::L:		text="L"; break;
			case CAFunctionMark::K:		text="K"; break;
			case CAFunctionMark::Undefined:
				break;
		}

		if (minor)
			text.prepend(QString(0x02DA));
 	}

	p->setPen(QPen(s.color));
	p->setFont(font);

	// draw
	switch (_drawableFunctionMarkSupportType) {
		case Key:
			p->drawText(s.x, s.y+qRound(_height*s.z), _key+":");
			break;
		case ChordArea:
			p->drawText(s.x, s.y+qRound(_height*s.z), QString("(")+text+")");
			break;
		case Tonicization:
			if (!_function2) {	// tonicization is below a single function
				p->drawText(s.x, s.y+qRound(_height*s.z), text);
				if (_extenderLineVisible)
					p->drawLine(s.x + p->boundingRect(0,0,0,0,0,text).width(), qRound(s.y+height()*s.z/2.0),
								qRound(s.x + width()*s.z), qRound(s.y+height()*s.z/2.0));
			} else {
				p->drawText(	// tonicization is below multiple functions
					// get the x-coordinate where to start text rendering
					qRound(s.x + (_function2->xPos() + _function2->width() - _function1->xPos())*s.z/2.0 - p->boundingRect(0,0,0,0,0,text).width()/2.0),
					s.y+qRound(_height*s.z),
					text
				);
				// draw line below side functions
				p->drawLine(s.x, s.y, qRound(s.x + (_function2->xPos()+_function2->width()-_function1->xPos())*s.z), s.y);

				// draw extender line of the support function
				if (_extenderLineVisible)
					p->drawLine(s.x + qRound((_function2->xPos()+_function2->width()-_function1->xPos())*s.z/2.0 + p->boundingRect(0,0,0,0,0,text).width()/2.0+1*s.z), qRound(s.y+height()*s.z/2.0),
								qRound(s.x + width()*s.z), qRound(s.y+height()*s.z/2.0));

			}
			break;
		case Ellipse:
			// draw vertical lines
			p->drawLine(s.x, qRound(s.y+2*s.z), s.x, qRound(s.y+height()*s.z/2.0)); //left vertical line
			p->drawLine(qRound(s.x+width()*s.z), qRound(s.y+2*s.z), qRound(s.x+width()*s.z), qRound(s.y+height()*s.z/2.0)); //right vertical line
			// draw horizontal lines
			p->drawLine(s.x, qRound(s.y+height()*s.z/2.0), qRound(s.x+width()*s.z/2.0 - p->boundingRect(0,0,0,0,0,"E").width()/2.0 - 2*s.z), qRound(s.y+height()*s.z/2.0));	//left horizontal line
			p->drawLine(qRound(s.x+width()*s.z/2.0 + p->boundingRect(0,0,0,0,0,"E").width()/2.0 + 2*s.z), qRound(s.y+height()*s.z/2.0), qRound(s.x+width()*s.z), qRound(s.y+height()*s.z/2.0));	//right horizontal line

			p->drawText(qRound(s.x+width()*s.z/2.0 - p->boundingRect(0,0,0,0,0,"E").width()/2.0), qRound(s.y+height()*s.z-1*s.z), "E");

			break;

		case Rectangle:
			p->drawRect(s.x, s.y, qRound(width()*s.z), qRound(height()*s.z));
			break;

		case Alterations:
			CAFunctionMark *f1 = (CAFunctionMark*)(_musElement);
			int curX = s.x, curY = s.y;

			if (f1->function()==CAFunctionMark::Undefined)
				curX+=qRound(0.3*_height*s.z);	// make space for left paranthesis, if needed
			// draw added degrees
			for (int i=0; i<f1->addedDegrees().size(); i++) {
				// draw upper number
				p->drawText(qRound(curX+2*s.z), qRound(curY+8*s.z), f1->addedDegrees().at(i)<0?QString::number(f1->addedDegrees().at(i)*(-1)):QString::number(f1->addedDegrees().at(i)));
				// draw additional + or - for added or substracted degree
				p->drawText(qRound(curX+2*s.z), qRound(curY+13*s.z), f1->addedDegrees().at(i)<0?"-":"+");
				curY+=qRound(13*s.z);
			}

			// draw altered degrees
			for (int i=0; i<f1->alteredDegrees().size(); i++) {
				//draw numbers with + or - prefixed
				p->drawText(curX, qRound(curY+8*s.z), f1->alteredDegrees().at(i)>0?QString("+")+QString::number(+f1->alteredDegrees().at(i)):QString::number(f1->alteredDegrees().at(i)));
				curY+=qRound(8*s.z);
			}

			// draw paranthesis, if needed
			if (f1->function()==CAFunctionMark::Undefined) {
				curX-=qRound(0.3*_height*s.z);
				font.setPixelSize(qRound(_height*s.z));
				p->setFont(font);
				p->drawText(curX, qRound(curY-_height*s.z/6.0), "(");
				p->drawText(qRound(curX+(_width-0.3*_height)*s.z), qRound(curY-_height*s.z/6.0), ")");
			}
	}
}

CADrawableFunctionMarkSupport *CADrawableFunctionMarkSupport::clone(CADrawableContext* newContext) {
	switch (_drawableFunctionMarkSupportType) {
		case Key:
			return new CADrawableFunctionMarkSupport(Key, _key, (newContext)?newContext:_drawableContext, _xPos, _yPos);
			break;
		case ChordArea:
		case Tonicization:
			return new CADrawableFunctionMarkSupport(_drawableFunctionMarkSupportType, _function1, (newContext)?newContext:_drawableContext, _xPos, _yPos, _function2);
			break;
		case Ellipse:
			return new CADrawableFunctionMarkSupport(Ellipse, _function1, (newContext)?newContext:_drawableContext, qRound(_xPos - _function1->width()/2.0), _yPos, _function2);
			break;
		case Rectangle:
			return new CADrawableFunctionMarkSupport(Rectangle, _function1, (newContext)?newContext:_drawableContext, _xPos+3, _yPos+3, _function2);
			break;
		case Alterations:
			return new CADrawableFunctionMarkSupport(Alterations, (CAFunctionMark*)(_musElement), (newContext)?newContext:_drawableContext, _xPos, _yPos);
			break;
		default:
			return 0;
	}
}
