/*!
	Copyright (c) 2018, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef JUMPTOVIEW_H_
#define JUMPTOVIEW_H_

#include <QDialog>

#include "ui_jumptoview.h"

class CAMainWin;

class CAJumpToView : public QDialog, private Ui::uiJumpToView {
	Q_OBJECT
public:
	CAJumpToView( CAMainWin *parent );
	virtual ~CAJumpToView();

public slots:
	void show();
	void accept();

private:
	void setupCustomUi();
};

#endif /* JUMPTOVIEW_H_ */
