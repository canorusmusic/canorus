/*!
	Copyright (c) 2006-2010, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef DRAWABLEKEYSIGNATURE_H_
#define DRAWABLEKEYSIGNATURE_H_

#include "layout/drawablemuselement.h"
#include "score/diatonickey.h"

class CADrawableStaff;
class CAKeySignature;
class CAClef;
class CADrawableAccidental;

class QComboBox;

class CADrawableKeySignature : public CADrawableMusElement {
	public:
		CADrawableKeySignature( CAKeySignature *keySig, CADrawableStaff *staff, CAClef *lastClef=0, CAKeySignature *lastKeySig=0 );
		~CADrawableKeySignature();

		inline CAKeySignature *keySignature() { return (CAKeySignature*)_musElement; }

		static void populateComboBox( QComboBox *c );
		static CADiatonicKey comboBoxRowToDiatonicKey( int row );
		static int diatonicKeyToRow( CADiatonicKey k );
		static void populateComboBoxDirection( QComboBox *c );

	private:
		QList<CADrawableAccidental*> _drawableAccidentalList;	///List of actual drawable accidentals
};

#endif /*DRAWABLEKEYSIGNATURE_H_*/
