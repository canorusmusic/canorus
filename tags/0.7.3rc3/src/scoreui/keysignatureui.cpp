/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QMessageBox>

#include "ui/mainwin.h"
#include "core/muselementfactory.h"
#include "layout/drawablekeysignature.h"
#include "scoreui/keysignatureui.h"
#include "scorectl/keysignaturectl.h"
#include "canorus.h"

/*! 
	\class CAKeySignatureUI
	\brief Keysignature user interface objects creation

	This class creates the key signature user interface parts
	of the main window (toolbar, combobox)
*/

/*!
	Default Construktor
	Besides standard initialization it creates it's ui objects.
	Also the control object is created here.
	Inbetween the combobox (for selection) gets populated
	Finally the Toolbar is added to the main win Toolbar and hidden
*/
CAKeySignatureUI::CAKeySignatureUI( CAMainWin *poMainWin, const QString &oHash )
{
	setObjectName("oKeySignatureUI");
	_poMainWin = poMainWin;
	_poKeySignatureCtl = new CAKeySignatureCtl( poMainWin, oHash );
	uiKeySigToolBar = new QToolBar( tr("Key Signature ToolBar"), poMainWin );
	uiKeySig = new QComboBox( poMainWin );
		uiKeySig->setObjectName("uiKeySig");
		CAKeySignatureUI::populateComboBox( uiKeySig );
	if( poMainWin == 0 )
		qCritical("KeySignatureUI: No mainwindow instance available!");
	// KeySig Toolbar
	uiKeySigToolBar->addWidget( uiKeySig );
	poMainWin->addToolBar(Qt::TopToolBarArea, uiKeySigToolBar);
	_poKeySignatureCtl->setupActions();
	uiKeySigToolBar->hide();
}

// Destructor
CAKeySignatureUI::~CAKeySignatureUI()
{
	if( uiKeySigToolBar ) {
		delete uiKeySigToolBar;
	}
	if( uiKeySig ) {
		delete uiKeySig;
	}
	uiKeySigToolBar = 0;
	uiKeySig = 0;
}

/*!
	Shows/Hides the key signature properties tool bar according to the current state.
*/
void CAKeySignatureUI::updateKeySigToolBar() {
	if (_poMainWin->isInsertKeySigChecked() && _poMainWin->mode()==CAMainWin::InsertMode) {
		uiKeySig->setCurrentIndex(
			uiKeySig->findData(
				CADiatonicKey::diatonicKeyToString(CADiatonicKey(_poMainWin->musElementFactory()->diatonicKeyNumberOfAccs(), _poMainWin->musElementFactory()->diatonicKeyGender() ))
			)
		);
		uiKeySigToolBar->show();
	} else if ( _poMainWin->mode()==CAMainWin::EditMode && _poMainWin->currentScoreView() &&
	            _poMainWin->currentScoreView()->selection().size() &&
	            dynamic_cast<CAKeySignature*>(_poMainWin->currentScoreView()->selection().at(0)->musElement()) ) {
		CAScoreView *v = _poMainWin->currentScoreView();
		if (v && v->selection().size()) {
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(v->selection().at(0)->musElement());
			if (keySig) {
				uiKeySig->setCurrentIndex( uiKeySig->findData( CADiatonicKey::diatonicKeyToString(keySig->diatonicKey()) ) );
				uiKeySigToolBar->show();
			} else
				uiKeySigToolBar->hide();
		}
	} else
		uiKeySigToolBar->hide();
}

/*!
	This function adds key signatures to the given combobox in order
	major-minor-major-... from most flats to most sharps.

	This function usually called when initializing the main window.

	\sa CADrawableKeySignature::comboBoxRowToDiatonicKey(), CADrawableKeySignature::populateComboBoxDirection()
 */
void CAKeySignatureUI::populateComboBox( QComboBox *c ) {
	c->addItem( QIcon("images:general/none.svg"), QObject::tr("C major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(0, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs7.svg"), QObject::tr("C-sharp major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(7, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-7.svg"), QObject::tr("C-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-7, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-3.svg"), QObject::tr("c minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-3, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs4.svg"), QObject::tr("c-sharp minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(4, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs2.svg"), QObject::tr("D major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(2, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-5.svg"), QObject::tr("D-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-5, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-1.svg"), QObject::tr("d minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-1, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs6.svg"), QObject::tr("d-sharp minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(6, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs4.svg"), QObject::tr("E major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(4, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-3.svg"), QObject::tr("E-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-3, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs1.svg"), QObject::tr("e minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(1, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs-6.svg"), QObject::tr("e-flat minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-6, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs-1.svg"), QObject::tr("F major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-1, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs6.svg"), QObject::tr("F-sharp major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(6, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-4.svg"), QObject::tr("f minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-4, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs3.svg"), QObject::tr("f-sharp minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(3, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs1.svg"), QObject::tr("G major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(1, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-6.svg"), QObject::tr("G-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-6, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-2.svg"), QObject::tr("g minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-2, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs5.svg"), QObject::tr("g-sharp minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(5, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs3.svg"), QObject::tr("A major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(3, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-4.svg"), QObject::tr("A-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-4, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:general/none.svg"), QObject::tr("a minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(0, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs7.svg"), QObject::tr("a-sharp minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(7, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs-7.svg"), QObject::tr("a-flat minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-7, CADiatonicKey::Minor)) );

	c->addItem( QIcon("images:accidental/accs5.svg"), QObject::tr("B major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(5, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs-2.svg"), QObject::tr("B-flat major"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-2, CADiatonicKey::Major)) );
	c->addItem( QIcon("images:accidental/accs2.svg"), QObject::tr("b minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(2, CADiatonicKey::Minor)) );
	c->addItem( QIcon("images:accidental/accs-5.svg"), QObject::tr("b-flat minor"), CADiatonicKey::diatonicKeyToString(CADiatonicKey(-5, CADiatonicKey::Minor)) );
}

/*!
	Adds directions Up and Down with icons to the given combo box.

	\sa CADrawableKeySignature::comboBoxRowToDiatonicKey(), CADrawableKeySignature::populateComboBox()
 */
void CAKeySignatureUI::populateComboBoxDirection( QComboBox *c ) {
	c->addItem( QIcon("images:general/up.svg"), QObject::tr("Up") );
	c->addItem( QIcon("images:general/down.svg"), QObject::tr("Down") );
}
