/** @file drawablekeysignature.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
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
 	if (keySig->keySignatureType() == CAKeySignature::MajorMinor) {
 		int idx, minY=y, maxY=y;
 		
 		CAKeySignature *prevKeySig = drawableStaff->getKeySignature(x);
 		//TODO: How are the accidentals pitches determined (which octave)?! Currently, I only call calculateHighestCenterYCoord() for all the positions. -Matevz
 		if (prevKeySig) {
 			idx = 3;
 			for (int i=0; i<7; i++) {	//place naturals for sharps
				if ((prevKeySig->accidentals()[idx]!=1) || (keySig->accidentals()[idx]==1))
					continue;
				
 				CADrawableAccidental *acc = new CADrawableAccidental(0, keySig, drawableStaff, newX, drawableStaff->calculateHighestCenterYCoord(idx, x));
 				
 				_drawableAccidentalList << acc;
 				
 				newX += (acc->width() + 5);
 				idx = (idx+4)%7;
 				
 				if (acc->yPos() < minY)
 					minY = acc->yPos();
 				if (acc->yPos() + acc->height() > maxY)
 					maxY = acc->yPos() + acc->height();
 			}
 			
			idx = 6;
 			for (int i=0; i<7; i++) {	//place naturals for flats
				if ((prevKeySig->accidentals()[idx]!=-1) || (keySig->accidentals()[idx]==-1))
					continue;
					
 				CADrawableAccidental *acc = new CADrawableAccidental(0, keySig, drawableStaff, newX, drawableStaff->calculateHighestCenterYCoord(idx, x));
 				
 				_drawableAccidentalList << acc;
 				
 				newX += (acc->width() + 5);
 				idx = (idx+3)%7;	

	 			if (acc->yPos() < minY)
 					minY = acc->yPos();
 				if (acc->yPos() + acc->height() > maxY)
 					maxY = acc->yPos() + acc->height();
 			}
 		}
 		
 		idx = 3;
 		for (int i=0; i<7; i++) {	//place sharps
			if (keySig->accidentals()[idx]!=1)
				continue;
				
 			CADrawableAccidental *acc = new CADrawableAccidental(1, keySig, drawableStaff, newX, drawableStaff->calculateHighestCenterYCoord(idx, x));
 			
 			_drawableAccidentalList << acc;
 			
 			newX += (acc->width() + 5);
 			idx = (idx+4)%7;
 			
 			if (acc->yPos() < minY)
 				minY = acc->yPos();
 			if (acc->yPos() + acc->height() > maxY)
 				maxY = acc->yPos() + acc->height();
 		}
 		
		idx = 6;
 		for (int i=0; i<7; i++) {	//place flats
			if (keySig->accidentals()[idx]!=-1)
				continue;
			
 			CADrawableAccidental *acc = new CADrawableAccidental(-1, keySig, drawableStaff, newX, drawableStaff->calculateHighestCenterYCoord(idx, x));
 			
 			_drawableAccidentalList << acc;
 			
 			newX += (acc->width() + 5);
 			idx = (idx+3)%7;

 			if (acc->yPos() < minY)
 				minY = acc->yPos();
 			if (acc->yPos() + acc->height() > maxY)
 				maxY = acc->yPos() + acc->height();
 		}
 	
 		_width = newX - x;
 		_height = maxY - minY;
	 	_yPos = minY;
	 	
	 	_neededWidth = _width;
	 	_neededHeight = _height;
 	}
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

CADrawableKeySignature* CADrawableKeySignature::clone() {
	return (new CADrawableKeySignature(keySignature(), (CADrawableStaff*)drawableContext(), xPos(), _drawableContext->yPos()));
}
