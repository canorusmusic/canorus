/*!
	Copyright (c) 2015, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef ACTIONDELEGATE_H_
#define ACTIONDELEGATE_H_

#include <QString>
#include <QAction>

// Helper methods to reduce code ballast in mainwin class
// Keyboard (Midi) Shortcuts that can be changed dynamically

class CAMainWin;
class CASingleAction;

class CAActionDelegate
{
public:
    CAActionDelegate(CAMainWin *mainWin);

    void addWinActions(QWidget &widget);
    void removeMainWinActions();
    void updateMainWinActions();

protected:
    void addSingleAction(const QString &oCommandName, const QString &oDescription, const QAction &oAction);
    void updateSingleAction(CASingleAction &oSource, QAction &oAction);

private:
    CAMainWin *_mainWin;
};

#endif // ACTIONDELEGATE_H_


