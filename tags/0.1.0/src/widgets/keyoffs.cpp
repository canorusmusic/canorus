/** @file widgets/keyoffs.cpp
 *
 * This program is free software; you can redistribute it and/or modify it   
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.                       
 *                                                                           
 * This program is distributed in the hope that it will be useful, but       
 * WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  
 * Public License for more details.                                          
 *                                                                           
 * You should have received a copy of the GNU General Public License along   
 * with this program; (See "LICENSE.GPL"). If not, write to the Free         
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        
 * 02111-1307, USA.                                                          
 *                                                                           
 *---------------------------------------------------------------------------
 *                                                                           
 * Copyright (c) 2006, Reinhard Katzmann, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

#include <QButtonGroup>
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include <iostream>
#include "keyoffs.h"
//#include "keysig.h"

#define KEY_OFFS_LEFT_DIST 5
#define KEY_OFFS_BUTTONWIDTH 20

CAKeyOffs::CAKeyOffs(const char *pcNoteName, int iBuNr, QWidget *poParent, const char *pcName) 
{
	miMargin      = 5;
	miSpace       = 4;
	// Create the button group for the radio button's actions
	mpoBGroup = new QButtonGroup(poParent);
	mpoBGroup->setObjectName( pcName );
	//mpoBGroup->setTitle(pcNoteName);
	// Size policy: Expanding / Expanding
	QSizePolicy oSizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
	oSizePolicy.setHorizontalStretch(0);
	oSizePolicy.setVerticalStretch(0);
	QSizePolicy oBMSPol = poParent->sizePolicy();
	oSizePolicy.setHeightForWidth( oBMSPol.hasHeightForWidth() );
	// Layout for visual group box
	mpoMLayout = new QGridLayout( 0 );
	mpoMLayout->setSpacing( 0 );
	mpoMLayout->setMargin( 0 );
	poParent->setSizePolicy( oSizePolicy );
	// Visual group box for the button menu
	mpoBBox   = new QGroupBox( "", poParent );
	mpoBBox->setFlat(true);
	oSizePolicy.setHeightForWidth(mpoBBox->sizePolicy().hasHeightForWidth());
	mpoBBox->setSizePolicy( oSizePolicy );
	// Layout for key offs
	mpoBLayout = new QGridLayout( mpoBBox );
	mpoBLayout->setSpacing( miSpace );
	mpoBLayout->setMargin( miMargin );
	// Actual positions of the buttons in the button menu layout
	//mpoBBox->setBackgroundRole( QPalette::Button );
	//mpoBBox->setAutoFillBackground( true );

	// Create the radio buttons themself
	mpoSharpButton = new QRadioButton("sharp",   mpoBBox);
	mpoFlatButton  = new QRadioButton("flat",    mpoBBox);
	mpoNaturButton = new QRadioButton("natural", mpoBBox);
	// Add radio buttons to abstract container
	mpoBGroup->addButton(mpoSharpButton, CKO_Sharp);
	mpoBGroup->addButton(mpoFlatButton,  CKO_Flat);
	mpoBGroup->addButton(mpoNaturButton, CKO_Natural);
	// Mutual exclusive button
	mpoBGroup->setExclusive(true);
	miBuNr = iBuNr;
	//keysigObj_ = 0;
	// Add actions for the radio buttons
	connect(mpoSharpButton, SIGNAL(toggled(bool)), this, SLOT(updateSharp(bool)));
	connect(mpoFlatButton,  SIGNAL(toggled(bool)), this, SLOT(updateFlat(bool)));
	connect(mpoNaturButton, SIGNAL(toggled(bool)), this, SLOT(updateNatural(bool)));
}

/*void CAKeyOffs::setKeysigObj(NKeySig *keysig_obj) 
{
	keysigObj_ = keysig_obj;
}*/

void CAKeyOffs::setOffs(CKO_Type eKind) 
{
	switch (eKind) 
	{
		case CKO_Sharp: mpoSharpButton->setChecked(true); break;
		case CKO_Flat:  mpoFlatButton->setChecked(true);  break;
		default:
		case CKO_Natural: mpoNaturButton->setChecked(true); break;
	}
}

void CAKeyOffs::updateSharp(bool bOn) 
{
	if (!bOn) return;
	//keysigObj_->setAccent(buNr_, PROP_SHARP);
}

void CAKeyOffs::updateFlat(bool bOn) {
	if (!bOn) return;
	//keysigObj_->setAccent(buNr_, PROP_FLAT);
}

void CAKeyOffs::updateNatural(bool bOn) {
	if (!bOn) return;
	//keysigObj_->setAccent(buNr_, PROP_NATUR);
}

void CAKeyOffs::setGeometry(int iXpos, int iYpos, int iWidth, int iHeight) 
{
	int iNewHeight;
	miXpos = iXpos; 
	miYpos = iYpos;
	iNewHeight = (iHeight - KEY_OFFS_UP_DIST-KEY_OFFS_BOTTOM_DIST) / 3;
	mpoBBox->setGeometry(iXpos, iYpos, iWidth, iHeight);
	mpoSharpButton->setGeometry(iXpos+KEY_OFFS_LEFT_DIST, iYpos + KEY_OFFS_UP_DIST, 
				     iWidth-2*KEY_OFFS_LEFT_DIST, 
				     (iHeight - KEY_OFFS_UP_DIST - KEY_OFFS_BOTTOM_DIST) / 3);
	mpoFlatButton->setGeometry(iXpos+KEY_OFFS_LEFT_DIST, 
				    iYpos + KEY_OFFS_UP_DIST + iNewHeight ,
				    iWidth-2*KEY_OFFS_LEFT_DIST, 
				    (iHeight -  KEY_OFFS_UP_DIST - KEY_OFFS_BOTTOM_DIST) / 3);
	mpoNaturButton->setGeometry(iXpos+KEY_OFFS_LEFT_DIST, 
				     iYpos + KEY_OFFS_UP_DIST + 2*iNewHeight,
				     iWidth-2*KEY_OFFS_LEFT_DIST, 
				     (iHeight - KEY_OFFS_UP_DIST - KEY_OFFS_BOTTOM_DIST) / 3);
}
