#include "note.h"

CANote::CANote(int length, int x, int y) {
	_length = length;
	_xPos = x;
	_yPos = y;
	_width = 9;
	_height = 34;
}

void CANote::draw(QPainter *p, int x, int y, float z) {
#define QUARTER_STEM_LENGTH 20
	switch (_length) {
		case 4:
			y += (int)(QUARTER_STEM_LENGTH*z);
			p->setFont(QFont("Emmentaler",(int)(16*z)));
			p->drawText(x,y,QString(0xE125));
			QRect bBox = p->boundingRect(0,0,0,0,0,QString(0xE125));
			int w = bBox.width(); //get width from font metrics
			int h = (int)(0.5*w) + (int)(QUARTER_STEM_LENGTH*z);
			QPen pen;
			pen.setWidth((int)(1*z));
			pen.setCapStyle(Qt::RoundCap);
			p->setPen(pen);
			p->drawLine((int)(x+w-0.5*z), (int)(y-1*z), (int)(x+w-0.5*z), y-(int)(QUARTER_STEM_LENGTH*z));
			
			break;
		
	}
}
