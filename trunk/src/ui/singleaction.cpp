/*!
	Copyright (c) 2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>

#include "singleaction.h"


CASingleAction::CASingleAction( QObject *parent )
 : QAction( parent )
{
}

CASingleAction::~CASingleAction()
{
}

void CASingleAction::setCommand( QString oCommand )
{
	if( !oCommand.isEmpty() )
	{
		_oCommand = oCommand;
		setText( oCommand );
	}
}

void CASingleAction::setContext( QString oContext )
{
	if( !oContext.isEmpty() )
		_oContext = oContext;
}

void CASingleAction::setDescription( QString oDescription )
{
	if( !oDescription.isEmpty() )
	{
		_oDescription = oDescription;
		setToolTip( oDescription );
	}
}

void CASingleAction::setShortCut( QString oShortCut )
{
	if( !oShortCut.isEmpty() )
	{
		_oShortCut = oShortCut;
		QAction::setShortcut( oShortCut );
	}
}

void CASingleAction::setMidiCommand( QString oMidiCommand )
{
	if( !oMidiCommand.isEmpty() )
		_oMidiCommand = oMidiCommand;
}
