/*!
        Copyright (c) 2006-2010, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

// Includes
#include <QMessageBox>

#include "ui/mainwin.h"
#include "core/muselementfactory.h"
#include "core/undo.h"
#include "layout/drawablekeysignature.h"
#include "scorectl/keysignaturectl.h"
#include "score/keysignature.h"
#include "score/sheet.h"
#include "canorus.h"

/*! 
	\class CAKeySignatureCtl
	\brief Keysignature ctl for user interface actions

	Instances of this class can be used for user interactions
	of placing key signatures into the score	
*/

/*!
	Default Construktor
	Besides standard initialization it creates a dummy object.
	Though not implemented we assume that the dummy object has at
	least one signal called dummyToggle.
	We connect this signal manually to our slot myToggle.
*/
CAKeySignatureCtl::CAKeySignatureCtl( CAMainWin *poMainWin, const QString &oHash )
 : _oHash( oHash )
{
	setObjectName("oDummyCtl");
	_poMainWin = poMainWin;
	if( poMainWin == 0 )
		qCritical("DummyCtl: No mainwindow instance available!");
}

void CAKeySignatureCtl::setupActions()
{
	CACanorus::connectSlotsByName(_poMainWin, this);
	//connect( _poDummy, SIGNAL( dummyToggle( int ) ), this, SLOT( myToggle( int ) ) );
}

// Destructor
CAKeySignatureCtl::~CAKeySignatureCtl()
{
}

/*!
	Changes the number of accidentals.
*/
void CAKeySignatureCtl::on_uiKeySig_activated( int row ) {
        
	CADiatonicKey key(static_cast<QComboBox*>(sender())->itemData(row).toString());

	if (_poMainWin->mode()==CAMainWin::InsertMode) {
		_poMainWin->musElementFactory()->setDiatonicKeyNumberOfAccs( key.numberOfAccs() );
		_poMainWin->musElementFactory()->setDiatonicKeyGender( key.gender() );
	} else
	if ( _poMainWin->mode()==CAMainWin::EditMode && _poMainWin->currentScoreView() && 
	     _poMainWin->currentScoreView()->selection().size() ) {
		QList<CADrawableMusElement*> list = _poMainWin->currentScoreView()->selection();
		CACanorus::undo()->createUndoCommand( _poMainWin->document(), tr("change key signature", "undo") );

		for ( int i=0; i<list.size(); i++ ) {
			CAKeySignature *keySig = dynamic_cast<CAKeySignature*>(list[i]->musElement());
			CAFunctionMark *fm = dynamic_cast<CAFunctionMark*>(list[i]->musElement());

			if ( keySig ) {
				keySig->setDiatonicKey( key );
			}

			if ( fm ) {
				fm->setKey( CADiatonicKey::diatonicKeyToString( key ) );
			}
		}

		CACanorus::undo()->pushUndoCommand();
		CACanorus::rebuildUI(_poMainWin->document(), _poMainWin->currentSheet());
	}
}

// Changes the toolbar entries for key signature input
void CAKeySignatureCtl::on_uiInsertKeySig_toggled(bool checked) {
	if (checked) {
		_poMainWin->musElementFactory()->setMusElementType( CAMusElement::KeySignature );
		_poMainWin->setMode( CAMainWin::InsertMode, _oHash );
	}
}
