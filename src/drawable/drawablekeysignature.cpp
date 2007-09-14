/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>

#include "drawable/drawablekeysignature.h"
#include "drawable/drawablestaff.h"
#include "core/keysignature.h"
#include "drawable/drawableaccidental.h"

CADrawableKeySignature::CADrawableKeySignature(CAKeySignature *keySig, CADrawableStaff *drawableStaff, int x, int y)
 : CADrawableMusElement(keySig, drawableStaff, x, y) {
	_drawableMusElementType = CADrawableMusElement::DrawableKeySignature;
	
	int newX = x;
	CAClef *clef = drawableStaff->getClef(x);
	int idx, idx2; // pitches of accidentals
	int minY=y, maxY=y;
	
	CAKeySignature *prevKeySig = drawableStaff->getKeySignature(x);
	if (prevKeySig) {
		// get initial neutral-sharp position
		idx = 3;
		idx2 = 0;
		while ( idx + (clef?clef->c1():-2) - 28  < -1 ||
				idx2 + (clef?clef->c1():-2) - 28 < -1) {
			idx+=7;
			idx2+=7;
		}
		
 		for ( int i=0; i<7; idx += (i%2?4:-3), i++ ) {	// place neutrals for sharps
			if ( (prevKeySig->accidentals()[idx%7]!=1) ||
			     (prevKeySig->accidentals()[idx%7]==1) && (keySig->accidentals()[idx%7]==1) )
				continue;
			
			int curIdx=idx;
			if ( curIdx + (clef?clef->c1():-2) - 28  < -1 )
				curIdx+=7;
			if ( curIdx + (clef?clef->c1():-2) - 28  > drawableStaff->staff()->numberOfLines()*2-1 )
				curIdx-=7;
			
 			CADrawableAccidental *acc = new CADrawableAccidental(0, keySig, drawableStaff, newX, drawableStaff->calculateCenterYCoord(curIdx, x));
 			
 			_drawableAccidentalList << acc;
 			
 			newX += (acc->width() + 5);
			
 			if ( acc->yPos() < minY )
 				minY = acc->yPos();
 			if ( acc->yPos() + acc->height() > maxY )
 				maxY = acc->yPos() + acc->height();
 		}
	
		// get initial neutral-flat position
		idx = 6;
		idx2 = 9;
		while ( idx + (clef?clef->c1():-2) - 28  < 0 ||
				idx2 + (clef?clef->c1():-2) - 28 < 0) {
			idx+=7;
			idx2+=7;
		}
		for ( int i=0; i<7; idx += (i%2?-4:3), i++ ) {	// place neutrals for flats
			if ( (prevKeySig->accidentals()[idx%7]!=-1) ||
			     (prevKeySig->accidentals()[idx%7]==-1) && (keySig->accidentals()[idx%7]==-1) )
				continue;
			
			int curIdx=idx;
			if ( curIdx + (clef?clef->c1():-2) - 28  < -1 )
				curIdx+=7;
			if ( curIdx + (clef?clef->c1():-2) - 28  > drawableStaff->staff()->numberOfLines()*2-1 )
				curIdx-=7;
			
			CADrawableAccidental *acc = new CADrawableAccidental(0, keySig, drawableStaff, newX, drawableStaff->calculateCenterYCoord(curIdx, x));
			
			_drawableAccidentalList << acc;
			
			newX += (acc->width() + 5);
			
			if ( acc->yPos() < minY )
				minY = acc->yPos();
			if ( acc->yPos() + acc->height() > maxY )
				maxY = acc->yPos() + acc->height();
		}
	}
	
	// get initial sharp position
	idx = 3;
	idx2 = 0;
	while ( idx + (clef?clef->c1():-2) - 28  < -1 ||
			idx2 + (clef?clef->c1():-2) - 28 < -1) {
		idx+=7;
		idx2+=7;
	}
	
	for ( int i=0; i<7; idx += (i%2?4:-3), i++ ) {	// place sharps
		if (keySig->accidentals()[idx%7]!=1)
			continue;
		
		int curIdx=idx;
		if ( curIdx + (clef?clef->c1():-2) - 28  < -1 )
			curIdx+=7;
		if ( curIdx + (clef?clef->c1():-2) - 28  > drawableStaff->staff()->numberOfLines()*2-1 )
			curIdx-=7;
		
			CADrawableAccidental *acc = new CADrawableAccidental(1, keySig, drawableStaff, newX, drawableStaff->calculateCenterYCoord(curIdx, x));
		
		_drawableAccidentalList << acc;
		
		newX += (acc->width() + 5);
		
		if ( acc->yPos() < minY )
			minY = acc->yPos();
		if ( acc->yPos() + acc->height() > maxY )
			maxY = acc->yPos() + acc->height();
	}
	
	// get initial flat position
	idx = 6;
	idx2 = 9;
	while ( idx + (clef?clef->c1():-2) - 28  < 0 ||
			idx2 + (clef?clef->c1():-2) - 28 < 0) {
		idx+=7;
		idx2+=7;
	}
	for ( int i=0; i<7; idx += (i%2?-4:3), i++ ) {	// place flats,
		if (keySig->accidentals()[idx%7]!=-1)
			continue;
		
		int curIdx=idx;
		if ( (curIdx + (clef?clef->c1():-2) - 28)  < -1 )
			curIdx+=7;
		if ( (curIdx + (clef?clef->c1():-2) - 28)  > (drawableStaff->staff()->numberOfLines()*2-1) )
			curIdx-=7;
		
		CADrawableAccidental *acc = new CADrawableAccidental(-1, keySig, drawableStaff, newX, drawableStaff->calculateCenterYCoord(curIdx, x));
		
		_drawableAccidentalList << acc;
		
		newX += (acc->width() + 5);
		
		if ( acc->yPos() < minY )
			minY = acc->yPos();
		if ( acc->yPos() + acc->height() > maxY )
			maxY = acc->yPos() + acc->height();
	}
	
	_width = newX - x;
	_height = maxY - minY;
 	_yPos = minY;
	
 	_neededWidth = _width;
 	_neededHeight = _height;
}

CADrawableKeySignature::~CADrawableKeySignature() {
	for (int i=0; i<_drawableAccidentalList.size(); i++)
		delete _drawableAccidentalList[i];
		
	_drawableAccidentalList.clear();
}

void CADrawableKeySignature::draw(QPainter *p, CADrawSettings s) {
	int xOrig = s.x;
	int yOrig = s.y;
	
	for (int i=0; i<_drawableAccidentalList.size(); i++) {
		s.x = xOrig + (int)((_drawableAccidentalList[i]->xPos() - xPos())*s.z);
		s.y = yOrig + (int)((_drawableAccidentalList[i]->yPos() - yPos())*s.z);
		_drawableAccidentalList[i]->draw(p, s);
		 
	}
}

CADrawableKeySignature* CADrawableKeySignature::clone(CADrawableContext* newContext) {
	return (new CADrawableKeySignature(keySignature(), (CADrawableStaff*)((newContext)?newContext:_drawableContext), xPos(), _drawableContext->yPos()));
}
