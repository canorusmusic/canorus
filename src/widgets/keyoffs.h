/** @file widgets/keyoffs.h
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

#ifndef KEYOFFS_H

#define KEYOFFS_H

#include <QObject>

#define KEY_OFFS_UP_DIST 20
#define KEY_OFFS_BOTTOM_DIST 0

typedef enum 
{
	CKO_Sharp	= 0,
	CKO_Flat,
	CKO_Natural
} CKO_Type;

class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QGridLayout;

// Instance of this class can be used to create a key
// offset widget for the key signature perspective
class CAKeyOffs : public QObject
{
	Q_OBJECT
public:
	/**
	 * Constructs the Key Offs
	 * 
	 * @param pcNoteName    Name of the note displayed above the Key Offs
	 * @param iBuNr         parent widget
	 * @param poParent      parent widget
	 * @param pcName        Name of the Key Offs
	 *
	 */
	CAKeyOffs(const char *pcNoteName, int iBuNr, QWidget *poParent, const char *pcName);
	//void setKeysigObj(NKeySig *keysig_obj);
	/**
	 * Sets the geometry of the Key Offs
	 * 
	 * @param iXPos    New X position
	 * @param iYPos    New Y position
	 * @param iWidth   New Width
	 * @param iHeight  New Height
	 *
	 */
	void setGeometry(int iXpos, int iYpos, int iWidth, int iHeight);
	void setOffs(CKO_Type eKind);
protected slots:
	/**
	 * Enables the sharp radio button (and switches off all others)
	 * 
	 * @param bool bOn  'true' to switch on the sharp radio button
	 *
	 */
	void updateSharp(bool bOn);
	/**
	 * Enables the flat radio button (and switches off all others)
	 * 
	 * @param bool bOn  'true' to switch on the flat radio button
	 *
	 */
	void updateFlat(bool bOn);
	/**
	 * Enables the natural radio button (and switches off all others)
	 * 
	 * @param bool bOn  'true' to switch on the natural radio button
	 *
	 */
	void updateNatural(bool bOn);
private:
	int miXpos, miYpos;             // Position of the Key Offs
	QButtonGroup *mpoBGroup;        // Button group with actions
	QGroupBox    *mpoBBox;		// Group box containing radio buttons
	QGridLayout  *mpoMLayout;       /// Layout for the group box
	QGridLayout  *mpoBLayout;       /// Layout for the button menu
	QRadioButton *mpoSharpButton;   // Sharp radio button
	QRadioButton *mpoFlatButton;    // Flat radio button
	QRadioButton *mpoNaturButton;   // Natural radio Button
	//NKeySig *keysigObj_;
	int           miBuNr;           // Button number
	int           miSpace;          // Space between buttons
	int           miMargin;         // Margin of layout
};


#endif // KEYOFFS_H
