/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef TRANSPOSEVIEW_H_
#define TRANSPOSEVIEW_H_

#include "ui_transposeview.h"

class CAMainWin;
class QAbstractButton;

class CATransposeView : public QDockWidget, private Ui::uiTransposeView {
	Q_OBJECT
public:
	CATransposeView( CAMainWin *parent );
	virtual ~CATransposeView();

public slots:
	void show();

private slots:
	void updateUi( bool );

	void on_uiApply_clicked( QAbstractButton *b );
	void on_uiIntervalQuantity_currentIndexChanged(int);

private:
	void setupCustomUi();
	void updateKeySig1();
};

#endif /* TRANSPOSEVIEW_H_ */
