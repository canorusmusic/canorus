/*!
	Copyright (c) 2006-2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QPainter>
#include <QComboBox> // needed for populateComboBox()

#include "drawable/drawablekeysignature.h"
#include "drawable/drawablestaff.h"
#include "core/clef.h"
#include "core/keysignature.h"
#include "drawable/drawableaccidental.h"

/*!
	Default constructor.

	\a y marks the top line Y coordinate of the staff in absolute world units.
*/
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

/*!
	This function adds key signatures to the given combobox in order
	major-minor-major-... from most flats to most sharps.

	This function usually called when initializing the main window.

	\sa CADrawableKeySignature::comboBoxRowToDiatonicKey(), CADrawableKeySignature::populateComboBoxDirection()
 */
void CADrawableKeySignature::populateComboBox( QComboBox *c ) {
	c->addItem( QIcon("images/accidental/accs-7.svg"), QObject::tr("C-flat major") );
	c->addItem( QIcon("images/accidental/accs-7.svg"), QObject::tr("a-flat minor") );
	c->addItem( QIcon("images/accidental/accs-6.svg"), QObject::tr("G-flat major") );
	c->addItem( QIcon("images/accidental/accs-6.svg"), QObject::tr("e-flat minor") );
	c->addItem( QIcon("images/accidental/accs-5.svg"), QObject::tr("D-flat major") );
	c->addItem( QIcon("images/accidental/accs-5.svg"), QObject::tr("b-flat minor") );
	c->addItem( QIcon("images/accidental/accs-4.svg"), QObject::tr("A-flat major") );
	c->addItem( QIcon("images/accidental/accs-4.svg"), QObject::tr("f minor") );
	c->addItem( QIcon("images/accidental/accs-3.svg"), QObject::tr("E-flat major") );
	c->addItem( QIcon("images/accidental/accs-3.svg"), QObject::tr("c minor") );
	c->addItem( QIcon("images/accidental/accs-2.svg"), QObject::tr("B-flat major") );
	c->addItem( QIcon("images/accidental/accs-2.svg"), QObject::tr("g minor") );
	c->addItem( QIcon("images/accidental/accs-1.svg"), QObject::tr("F major") );
	c->addItem( QIcon("images/accidental/accs-1.svg"), QObject::tr("d minor") );
	c->addItem( QIcon("images/general/none.svg"), QObject::tr("C major") );
	c->addItem( QIcon("images/general/none.svg"), QObject::tr("a minor") );
	c->addItem( QIcon("images/accidental/accs1.svg"), QObject::tr("G major") );
	c->addItem( QIcon("images/accidental/accs1.svg"), QObject::tr("e minor") );
	c->addItem( QIcon("images/accidental/accs2.svg"), QObject::tr("D major") );
	c->addItem( QIcon("images/accidental/accs2.svg"), QObject::tr("b minor") );
	c->addItem( QIcon("images/accidental/accs3.svg"), QObject::tr("A major") );
	c->addItem( QIcon("images/accidental/accs3.svg"), QObject::tr("f-sharp minor") );
	c->addItem( QIcon("images/accidental/accs4.svg"), QObject::tr("E major") );
	c->addItem( QIcon("images/accidental/accs4.svg"), QObject::tr("c-sharp minor") );
	c->addItem( QIcon("images/accidental/accs5.svg"), QObject::tr("B major") );
	c->addItem( QIcon("images/accidental/accs5.svg"), QObject::tr("g-sharp minor") );
	c->addItem( QIcon("images/accidental/accs6.svg"), QObject::tr("F-sharp major") );
	c->addItem( QIcon("images/accidental/accs6.svg"), QObject::tr("d-sharp minor") );
	c->addItem( QIcon("images/accidental/accs7.svg"), QObject::tr("C-sharp major") );
	c->addItem( QIcon("images/accidental/accs7.svg"), QObject::tr("a-sharp minor") );
}

/*!
	This function returns the selected diatonic key dependent on the selected row.

	\sa CADrawableKeySignature::populateComboBox(), CADrawableKeySignature::populateComboBoxDirection()
 */
CADiatonicKey CADrawableKeySignature::comboBoxRowToDiatonicKey( int row ) {
	signed char accs = qRound((row-14.5) / 2);
	CADiatonicKey::CAGender gender = (row%2)==0 ? CADiatonicKey::Major : CADiatonicKey::Minor;

	return CADiatonicKey( accs, gender );
}

/*!
	Adds directions Up and Down with icons to the given combo box.

	\sa CADrawableKeySignature::comboBoxRowToDiatonicKey(), CADrawableKeySignature::populateComboBox()
 */
void CADrawableKeySignature::populateComboBoxDirection( QComboBox *c ) {
	c->addItem( QIcon("images/general/up.svg"), QObject::tr("Up") );
	c->addItem( QIcon("images/general/down.svg"), QObject::tr("Down") );
}

/*!
	Returns the item index in the combo box corresponding to the given diatonic key \a k.
 */
int CADrawableKeySignature::diatonicKeyToRow( CADiatonicKey k ) {
	return (k.numberOfAccs()+7)*2 + ((k.gender()==CADiatonicKey::Minor)?1:0);
}
