/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef MIDIRECORDERVIEW_H_
#define MIDIRECORDERVIEW_H_

#include <QTimer>
#include <QDockWidget>

#include "ui_midirecorder.h"

class QAction;
class QLabel;
class QSlider;
class QWidget;

class CAMidiRecorder;

class CAMidiRecorderView : public QDockWidget, private Ui::uiMidiRecorder {
	Q_OBJECT

public:
	CAMidiRecorderView( CAMidiRecorder *recorder, QWidget *parent=0 );
	virtual ~CAMidiRecorderView();

	void setMidiRecorder( CAMidiRecorder *r ) { _midiRecorder = r; }
	CAMidiRecorder *midiRecorder() { return _midiRecorder; }

	enum CARecorderStatus {
		Idle,
		Pause,
		Recording
	};

private slots:
	void on_uiRecord_clicked(bool);
	void on_uiPause_clicked(bool);
	void on_uiStop_clicked(bool);
	void onTimerTimeout();

private:
	void setupCustomUi();

	QTimer   *_timer;

	CAMidiRecorder *_midiRecorder;
	CARecorderStatus _status;
};

#endif /* MIDIRECORDERVIEW_H_ */
