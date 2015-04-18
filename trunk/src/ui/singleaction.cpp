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
    _bMidiShortCutCombined = false;
}

CASingleAction::~CASingleAction()
{
}

void CASingleAction::setCommandName( QString oCommandName )
{
    if( !oCommandName.isEmpty() )
	{
        _oCommandName = oCommandName;
        setText( oCommandName );
	}
}

void CASingleAction::setDescription( QString oDescription )
{
	if( !oDescription.isEmpty() )
	{
		_oDescription = oDescription;
		setToolTip( oDescription );
	}
}

void CASingleAction::setShortCutAsString( QString oShortCut )
{
	if( !oShortCut.isEmpty() )
	{
		_oShortCut = oShortCut;
		QAction::setShortcut( oShortCut );
        _oSysShortCut = shortcut();
	}
}

void CASingleAction::setMidiKeySequence( QString oMidiKeySequence, bool combined )
{
    if( !oMidiKeySequence.isEmpty() )
    {
        _oMidiKeySequence = oMidiKeySequence;
        _bMidiShortCutCombined = combined;
        QStringList mksList = oMidiKeySequence.split(" ");
        QString le;
        _oMidiKeyParameters.clear();
        foreach(le,mksList)
        {
            _oMidiKeyParameters.push_back(le.toInt());
        }
    }
}
