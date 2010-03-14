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
		CADrawableKeySignature::populateComboBox( uiKeySig );
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
		uiKeySig->setCurrentIndex((
	_poMainWin->musElementFactory()->diatonicKeyNumberOfAccs()+7)*2 +
	  ((_poMainWin->musElementFactory()->diatonicKeyGender()==CADiatonicKey::Minor)?1:0) );
		uiKeySigToolBar->show();
	} else if ( _poMainWin->mode()==CAMainWin::EditMode && _poMainWin->currentScoreView() &&
	            _poMainWin->currentScoreView()->selection().size() &&
	            dynamic_cast<CAKeySignature*>(_poMainWin->currentScoreView()->selection().at(0)->musElement()) ) {
		CAScoreView *v = _poMainWin->currentScoreView();
		if (v && v->selection().size()) {
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(v->selection().at(0)->musElement());
			if (keySig) {
				uiKeySig->setCurrentIndex( CADrawableKeySignature::diatonicKeyToRow( keySig->diatonicKey() ) );
				uiKeySigToolBar->show();
			} else
				uiKeySigToolBar->hide();
		}
	} else
		uiKeySigToolBar->hide();
}

