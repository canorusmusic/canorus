#include "note.h"

CANote::CANote(int length, int x, int y) : CAMusElement(x, y) {
	_length = length;
	_width = 9;
	_height = 34;
}

void CANote::draw(QPainter *p, CADrawSettings s) {
#define QUARTER_STEM_LENGTH 20
	switch (_length) {
		case 4:
			s.y += (int)(QUARTER_STEM_LENGTH*s.z);
			p->setFont(QFont("Emmentaler",(int)(16*s.z)));
			p->drawText(s.x,s.y,QString(0xE125));
			QRect bBox = p->boundingRect(0,0,0,0,0,QString(0xE125));
			int w = bBox.width(); //get width from font metrics
			int h = (int)(0.5*w) + (int)(QUARTER_STEM_LENGTH*s.z);
			QPen pen;
			pen.setWidth((int)(1*s.z));
			pen.setCapStyle(Qt::RoundCap);
			p->setPen(pen);
			p->drawLine((int)(s.x+w-0.5*s.z), (int)(s.y-1*s.z), (int)(s.x+w-0.5*s.z), s.y-(int)(QUARTER_STEM_LENGTH*s.z));
			
			break;
		
	}
}

CANote *CANote::clone() {
	CANote *d = new CANote(_length, _xPos, _yPos);
	
	return d;
}
