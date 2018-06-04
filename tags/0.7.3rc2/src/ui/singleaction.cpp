/*!
	Copyright (c) 2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QString>

#include "singleaction.h"

CASingleAction::CASingleAction( QObject *parent )
 : _pAction(0)
{
    _bMidiShortCutCombined = false;
}

CASingleAction::~CASingleAction()
{
    if(m_localCreated) {
        delete _pAction;
    }
    _pAction = 0;
}

void CASingleAction::setCommandName( QString oCommandName )
{
    if( !oCommandName.isEmpty() )
	{
        _oCommandName = oCommandName;
        if(_pAction) {
            _pAction->setText( oCommandName );
        }
        _oCommandNameNoAmpersand = _oCommandName;
        _oCommandNameNoAmpersand.remove("&");
	}
}

void CASingleAction::setDescription( QString oDescription )
{
	if( !oDescription.isEmpty() )
	{
		_oDescription = oDescription;
        if(_pAction) {
            _pAction->setToolTip( oDescription );
        }
	}
}

void CASingleAction::setShortCutAsString( QString oShortCut )
{
	if( !oShortCut.isEmpty() )
	{
		_oShortCut = oShortCut;
        if(_pAction) {
            _pAction->setShortcut( oShortCut );
        }
        //_oSysShortCut = shortcut();
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

//void CASingleAction::setAction(QAction *pAction)
//{
//    if(pAction == 0)
//    {
//        qWarning("Not overwriting action with Null-Pointer");
//        return;
//    }
//    _pAction = pAction;
//}

QAction *CASingleAction::newAction(QObject *parent)
{
    if(_pAction && m_localCreated) {
        delete _pAction;
    }
    m_localCreated = true;
    _pAction = new QAction(getCommandName(),parent);
    _pAction->setText( getCommandName() );
    _pAction->setShortcut( getShortCutAsString() );
    _pAction->setToolTip( getDescription() );
    return _pAction;
}

void CASingleAction::fromQAction(const QAction &action, CASingleAction &sAction)
{
    sAction.getAction()->setActionGroup(action.actionGroup());
    sAction.getAction()->setAutoRepeat(action.autoRepeat());
    sAction.getAction()->setCheckable(action.isCheckable());
    sAction.getAction()->setChecked(action.isChecked());
    sAction.getAction()->setData(action.data());
    sAction.getAction()->setEnabled(action.isEnabled());
    sAction.getAction()->setFont(action.font());
    sAction.getAction()->setIcon(action.icon());
    sAction.getAction()->setIconText(action.iconText());
    sAction.getAction()->setIconVisibleInMenu(action.isIconVisibleInMenu());
    sAction.getAction()->setMenu(action.menu());
    sAction.getAction()->setMenuRole(action.menuRole());
    sAction.getAction()->setObjectName(action.objectName());
    sAction.getAction()->setParent(action.parent());
    sAction.getAction()->setPriority(action.priority());
    sAction.getAction()->setSeparator(action.isSeparator());
    sAction.getAction()->setStatusTip(action.statusTip());
    sAction.getAction()->setShortcut(action.shortcut());
    sAction.getAction()->setShortcutContext(action.shortcutContext());
    sAction.getAction()->setShortcuts(action.shortcuts());
    sAction.getAction()->setStatusTip(action.statusTip());
    sAction.getAction()->setText(action.text());
    sAction.getAction()->setToolTip(action.toolTip());
    sAction.getAction()->setVisible(action.isVisible());
    sAction.getAction()->setWhatsThis(action.whatsThis());
}
