/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "ui/transposeview.h"
#include "ui/mainwin.h"
#include "scoreui/keysignatureui.h"
#include "core/undo.h"
#include "canorus.h"

#include "score/keysignature.h"
#include "core/transpose.h"
#include "score/sheet.h"
#include "layout/drawablekeysignature.h"
#include "layout/drawablemuselement.h"

CATransposeView::CATransposeView( CAMainWin *p )
: QDockWidget( p ) {
	setupUi( this );
	setupCustomUi();
}

CATransposeView::~CATransposeView() {
}

void CATransposeView::setupCustomUi() {
	// populate Key signatures
	CAKeySignatureUI::populateComboBox( uiKeySigFrom );
	CAKeySignatureUI::populateComboBox( uiKeySigTo );
	CAKeySignatureUI::populateComboBoxDirection( uiKeySigDir );

	// populate Intervals
	for (int i=1; i<9; i++) {
		// also triggers currentIndexChanged() and populates uiIntervalQuality
		uiIntervalQuantity->addItem( CAInterval::quantityToReadable(i) );
	}
	CAKeySignatureUI::populateComboBoxDirection( uiIntervalDir );

	connect( uiByKeySig, SIGNAL(toggled(bool)), this, SLOT(updateUi(bool)) );
	connect( uiByInterval, SIGNAL(toggled(bool)), this, SLOT(updateUi(bool)) );
	connect( uiBySemitones, SIGNAL(toggled(bool)), this, SLOT(updateUi(bool)) );
	connect( uiReinterpretAccidentals, SIGNAL(toggled(bool)), this, SLOT(updateUi(bool)) );
}

/*!
	Sets the KeySig1 key signature to the key signature of the first selected note.
 */
void CATransposeView::show() {
	CAScoreView *v = static_cast<CAMainWin*>(parent())->currentScoreView();

	if (v) {
		CAKeySignature *k = 0;

		if (v->selection().size()) {
			for (int i=0; !k && i<v->selection().size(); i++) {
				if ( v->selection()[i]->musElement() &&
					 v->selection()[i]->musElement()->isPlayable() ) {
					k = static_cast<CAPlayable*>(v->selection()[i]->musElement())->voice()->getKeySig( v->selection()[i]->musElement() );
				}
			}
		}

		// the key signature wasn't found yet - find the first key sig in the score
		for (int i=0; !k && i<v->sheet()->staffList().size(); i++) {
			if ( v->sheet()->staffList()[i]->voiceList().size() ) {
				k = v->sheet()->staffList()[i]->voiceList()[0]->getKeySig( 0 );
			}
		}

		if (k) { // key signature is placed
			uiKeySigFrom->setCurrentIndex(
				uiKeySigFrom->findData(
					CADiatonicKey::diatonicKeyToString(k->diatonicKey())
				)
			);
		} else { // set the key signature to empty (C-Major by default)
			uiKeySigFrom->setCurrentIndex(
				uiKeySigFrom->findData(
					CADiatonicKey::diatonicKeyToString(CADiatonicKey())
				)
			);
		}
	}

	QDockWidget::show();

	uiByInterval->toggle(); // select Transpose by interval by default
}

void CATransposeView::updateUi( bool ) {
	uiKeySigFrom->setEnabled( uiByKeySig->isChecked() );
	uiTo->setEnabled( uiByKeySig->isChecked() );
	uiKeySigTo->setEnabled( uiByKeySig->isChecked() );
	uiKeySigDir->setEnabled( uiByKeySig->isChecked() );

	uiIntervalQuality->setEnabled( uiByInterval->isChecked() );
	uiIntervalQuantity->setEnabled( uiByInterval->isChecked() );
	uiIntervalDir->setEnabled( uiByInterval->isChecked() );

	uiSemitones->setEnabled( uiBySemitones->isChecked() );

	uiReinterpretType->setEnabled( uiReinterpretAccidentals->isChecked() );
}

/*!
	Updates the quality options when quantity is changed.
	This function hides minor and major quality for prime, fourth, fifth and octave
	and hides perfect quality for second, third, sixth and seventh.

 */
void CATransposeView::on_uiIntervalQuantity_currentIndexChanged( int newIndex ) {
	uiIntervalQuality->clear();

	switch (newIndex) {
	case 0: // prime
	case 3: // fourth
	case 4: // fifth
	case 7: { // octave
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(-2) );
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(0) );
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(2) );
		uiIntervalQuality->setCurrentIndex( 1 ); // select Perfect
		break;
	}
	default: // other intervals (no perfect)
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(-2) );
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(-1) );
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(1) );
		uiIntervalQuality->addItem( CAInterval::qualityToReadable(2) );
		uiIntervalQuality->setCurrentIndex( 2 ); // select Perfect
		break;
	}
}

void CATransposeView::on_uiApply_clicked( QAbstractButton *b ) {
	if ( dynamic_cast<CAMainWin*>(parent()) &&
	     static_cast<CAMainWin*>(parent())->currentScoreView() ) {
		CACanorus::undo()->createUndoCommand( static_cast<CAMainWin*>(parent())->document(), tr("transposition", "undo") );

		CAScoreView *v = static_cast<CAMainWin*>(parent())->currentScoreView();
		CATranspose t;

		// get the music elements to transpose
		if ( v->selection().size() ) {
			for (int i=0; i<v->selection().size(); i++) {
				t.addMusElement( v->selection()[i]->musElement() );
			}
		} else
		if ( v->currentContext() ) {
			t.addContext( v->currentContext()->context() );
		} else {
			t.addSheet( v->sheet() );
		}

		// do the transpose dependent on the current transpose mode
		if ( uiByKeySig->isChecked() ) {
			t.transposeByKeySig( CADiatonicKey(uiKeySigFrom->currentData().toString()),
								 CADiatonicKey(uiKeySigTo->currentData().toString()),
					             uiKeySigDir->currentIndex()?(-1):1 );
		} else
		if ( uiByInterval->isChecked() ) {
			CAInterval i;
			if (uiIntervalQuality->count()==3) { // prime, fourth, fifth, octave
				i = CAInterval( (uiIntervalQuality->currentIndex()-1)*2,
		                        (uiIntervalQuantity->currentIndex()+1)*(uiIntervalDir->currentIndex()?(-1):1) );
			} else { // second, third, sixth, seventh
				i = CAInterval( qRound((uiIntervalQuality->currentIndex()-1.5)*(4/3.0)), //simple formula to distribute 0..3 -> -2..2
		                        (uiIntervalQuantity->currentIndex()+1)*(uiIntervalDir->currentIndex()?(-1):1) );
			}
			t.transposeByInterval( i );
		} else
		if ( uiBySemitones->isChecked() ) {
			t.transposeBySemitones( uiSemitones->value() );
		} else
		if ( uiReinterpretAccidentals->isChecked() ) {
			if (uiReinterpretType->currentIndex()==0) {
				// sharps to flats
				t.reinterpretAccidentals( 1 );
			} else
			if (uiReinterpretType->currentIndex()==1) {
				// flats to sharps
				t.reinterpretAccidentals( -1 );
			} else
			if (uiReinterpretType->currentIndex()==2) {
				// invert
				t.reinterpretAccidentals( 0 );
			}
		}

		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI( static_cast<CAMainWin*>(parent())->document(), v->sheet() );
	}
}
