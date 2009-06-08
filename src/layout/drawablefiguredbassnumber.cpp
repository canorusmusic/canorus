/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "layout/drawablefiguredbassnumber.h"
#include "layout/drawablefiguredbasscontext.h"
#include "score/figuredbassmark.h"
#include "canorus.h"
#include <QPen>
#include <QPainter>

const double CADrawableFiguredBassNumber::DEFAULT_NUMBER_SIZE = 17;

CADrawableFiguredBassNumber::CADrawableFiguredBassNumber( CAFiguredBassMark *f, int number, CADrawableFiguredBassContext *context, double x, double y )
 : CADrawableMusElement(f, context, x, y), _number(number) {
	setDrawableMusElementType( DrawableFiguredBassNumber );
	int textWidth = 11;
	setWidth( textWidth < 11 ? 11 : textWidth ); // set minimum text width at least 11 points
	setHeight( qRound(DEFAULT_NUMBER_SIZE)*figuredBassMark()->numbers().size() );
}

CADrawableFiguredBassNumber::~CADrawableFiguredBassNumber() {
}

void CADrawableFiguredBassNumber::draw(QPainter *p, const CADrawSettings s) {
	QPen pen(s.color);
	pen.setWidth( qRound(1.2*s.z) );
	pen.setCapStyle( Qt::RoundCap );
	p->setPen( pen );
	QFont font("Emmentaler");
	font.setPixelSize( qRound(DEFAULT_NUMBER_SIZE*s.z*1.3) );
	p->setFont( font );

	QString text;
	if (figuredBassMark()->accs().contains(_number)) {
		if (figuredBassMark()->accs()[_number]==-2) {
			text += QString(CACanorus::fetaCodepoint("accidentals.flatflat"));
		} else
		if (figuredBassMark()->accs()[_number]==-1) {
			text += QString(CACanorus::fetaCodepoint("accidentals.flat"));
		} else
		if (figuredBassMark()->accs()[_number]==0) {
			text += QString(CACanorus::fetaCodepoint("accidentals.natural"));
		} else
		if (figuredBassMark()->accs()[_number]==1) {
			text += QString(CACanorus::fetaCodepoint("accidentals.sharp"));
		} else
		if (figuredBassMark()->accs()[_number]==2) {
			text += QString(CACanorus::fetaCodepoint("accidentals.sharpsharp"));
		}
	}

	if (_number) {
		text += QString::number(_number);
	} else {
		text += " ";
	}

	p->drawText( s.x, s.y+qRound(0.8*DEFAULT_NUMBER_SIZE*s.z), text );
}

CADrawableFiguredBassNumber *CADrawableFiguredBassNumber::clone(CADrawableContext *c) {
	return new CADrawableFiguredBassNumber(
		figuredBassMark(),
		_number,
		(c?static_cast<CADrawableFiguredBassContext*>(c):static_cast<CADrawableFiguredBassContext*>(drawableContext())),
		xPos(),
		yPos()
	);
}
