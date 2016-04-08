/*!
	Copyright (c) 2015, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QDebug>

#include "ui/singleaction.h"
#include "ui/mainwin.h"
#include "actiondelegate.h"
#include "canorus.h"
#include "settings.h"

CAActionDelegate::CAActionDelegate(CAMainWin *mainWin)
{
    _mainWin = mainWin;
}

// Add a single action to the list of actions from Settings (copy action parameters)
void CAActionDelegate::addSingleAction(const QString &oCommandName, const QString &oDescription, const QAction &oAction)
{
    CASingleAction *pMainAction = new CASingleAction(_mainWin);
    if( pMainAction ) {
        //pMainAction->setAction( &oAction );
        pMainAction->newAction();
        pMainAction->fromQAction(oAction,*pMainAction);
        pMainAction->setCommandName(oCommandName);
        pMainAction->setDescription(oDescription);
        pMainAction->setShortCutAsString( oAction.shortcut().toString() );
        //pMainAction->setMidiKeySequence( oMidiKeySequence ); // ToDo
        CACanorus::settings()->addSingleAction(*pMainAction);
    }
}

// Update an action with the settings parameters of a single action (copy single action parameters)
void CAActionDelegate::updateSingleAction(CASingleAction &oSource, QAction &oAction)
{
    oAction.setText(oSource.getCommandName());
    oAction.setStatusTip(oSource.getDescription());
    oAction.setShortcut(oSource.getAction()->shortcut());
    //oAction->setMidiKeySequence( oSource.getMidiKeySequence ); // ToDo
    oAction.setObjectName( oSource.getAction()->objectName() );
    oAction.setActionGroup( oSource.getAction()->actionGroup() );
    oAction.setAutoRepeat( oSource.getAction()->autoRepeat() );
    oAction.setCheckable( oSource.getAction()->isCheckable() );
    oAction.setChecked( oSource.getAction()->isChecked() );
    oAction.setData( oSource.getAction()->data() );
}

// Add Actions from CAMainWin to the list of actions of the settings (initialization)
void CAActionDelegate::addWinActions(QWidget &widget)
{
    // Hard coded description could be changed to Status Tip Text (if available)
    //addSingleAction(_mainWin->uiQuit->text(),_mainWin->tr("Exit program"),*_mainWin->uiQuit);
    QList<QAction *> actionList = widget.actions();
    qWarning() << "Delegate: Adding " << actionList.size() << " actions ";
    QAction *actionEntry;
    foreach(actionEntry, actionList) {
        qWarning("Adding new action %s to shortcut list of size %d", actionEntry->text().toLatin1().constData(),widget.actions().size());
        addSingleAction( actionEntry->text(), actionEntry->toolTip(), *actionEntry);
    }
}

// Remove all CAMainWin actions from the list of (single) actions in settings (exit program)
void CAActionDelegate::removeMainWinActions()
{
    CASingleAction *poAction;
    while( 0 != CACanorus::settings()->getActionList().isEmpty() ) {
        poAction = CACanorus::settings()->getActionList().back();
        if( poAction ) {
            CACanorus::settings()->deleteSingleAction( poAction->getCommandName(), poAction );
            delete poAction;
            poAction = 0;
        }
    }
}

// Update all CAMainWin actions using the shortcuts (etc.) read from the settings
void CAActionDelegate::updateMainWinActions()
{
    CASingleAction *poResAction;
    CACanorus::settings()->getSingleAction(_mainWin->uiQuit->text(), poResAction);
    updateSingleAction(*poResAction,*_mainWin->uiQuit);
}
