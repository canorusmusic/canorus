/*!
    Copyright (c) 2015, Reinhard Katzmann, Matevž Jekovec, Canorus development team
    All Rights Reserved. See AUTHORS for a complete list of authors.

    Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef ACTIONSTORAGE_H_
#define ACTIONSTORAGE_H_

#include <QString>
#include <QAction>

// Helper class to reduce code ballast in mainwin class
// Action instances from childs in CAMainWin are stored here for access
//   their default keyboard information (like shortcut)

class CAMainWin;
class CAActionDelegate;
class CASingleAction;

class CAActionStorage
{
public:
    CAActionStorage();
    ~CAActionStorage();

    void storeActionsFromMainWindow(CAMainWin &mainWin);
    void storeAction(QAction *action);
    void addWinActions();

protected:
    QWidget           _actionWidget;
    CAActionDelegate *_actionDelegate; // Requires parts of main win, so needs to be a member
};

#endif // ACTIONSTORAGE_H_
