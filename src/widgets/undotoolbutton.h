/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef UNDOTOOLBUTTON_H_
#define UNDOTOOLBUTTON_H_

#include "widgets/toolbutton.h"

#include <QListWidget>

class QUndoStack;

class CAUndoToolButton : public CAToolButton {
    Q_OBJECT
public:
    enum CAUndoToolButtonType {
        Undo,
        Redo
    };

    CAUndoToolButton(QIcon icon, CAUndoToolButtonType t, QWidget* parent);
    ~CAUndoToolButton();
    void setDefaultAction(QAction*);

    inline CAUndoToolButtonType undoType() { return _type; }
    inline void setUndoType(CAUndoToolButtonType type) { _type = type; }
    void showButtons();

public slots:
    void onListWidgetItemClicked(QListWidgetItem*);
    void onListWidgetItemEntered(QListWidgetItem*);

protected:
    void wheelEvent(QWheelEvent*);

private:
    QListWidget* _listWidget;
    CAUndoToolButtonType _type;
    QIcon _icon;
};

#endif /* UNDOTOOLBUTTON_H_ */
