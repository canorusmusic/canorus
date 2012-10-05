/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef PROGRESSSTATUSBAR_H_
#define PROGRESSSTATUSBAR_H_

#include <QStatusBar>

class QLabel;
class QPushButton;
class QProgressBar;

class CAProgressStatusBar : public QStatusBar {
	Q_OBJECT

public:
	CAProgressStatusBar( QWidget *parent );
	~CAProgressStatusBar();

public slots:
	void setProgress( QString label, int value );
	void setProgress( int value );
	void setProgress( QString label );

signals:
	void cancelButtonClicked( bool );

private slots:
	void on_cancelButton_clicked( bool );

private:
	QLabel       *_progressLabel;
	QProgressBar *_progressBar;
	QPushButton  *_cancelButton;
};

#endif /* PROGRESSSTATUSBAR_H_ */
