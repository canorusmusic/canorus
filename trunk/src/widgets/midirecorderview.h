/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDIRECORDERVIEW_H_
#define MIDIRECORDERVIEW_H_

#include <QToolBar>

class QAction;
class QLabel;
class QSlider;
class QWidget;

class CAMidiRecorder;

class CAMidiRecorderView : public QToolBar {
	Q_OBJECT

public:
	CAMidiRecorderView( CAMidiRecorder *recorder, QWidget *parent=0 );
	virtual ~CAMidiRecorderView();

	enum CARecorderStatus {
		Idle,
		Playing,
		Recording
	};

private slots:
	void on_uiPlay_triggered(bool);
	void on_uiStop_triggered(bool);
	void on_uiPause_triggered(bool);
	void on_uiRecord_triggered(bool);

private:
	void setupCustomUi( QWidget *parent );

	QAction *uiPlay;
	QAction *uiPause;
	QAction *uiStop;
	QAction *uiRecord;
	QSlider *_slider;       // progress
	QAction *_sliderAction;
	QLabel  *_time;
	QAction *_timeAction;

	CAMidiRecorder *_midiRecorder;
	CARecorderStatus _status;
};

#endif /* MIDIRECORDERVIEW_H_ */
