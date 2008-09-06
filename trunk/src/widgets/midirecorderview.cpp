/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "widgets/midirecorderview.h"
#include "core/midirecorder.h"

#include "canorus.h"

#include <QAction>
#include <QDir>

CAMidiRecorderView::CAMidiRecorderView( CAMidiRecorder *r, QWidget *parent )
 : QToolBar( tr("Midi recorder"), parent ), _midiRecorder(r) {
	// Locate resources (images, icons)
	QString currentPath = QDir::currentPath();

	QList<QString> resourcesLocations = CACanorus::locateResourceDir(QString("images"));
	if (!resourcesLocations.size()) // when Canorus not installed, search the source path
		resourcesLocations = CACanorus::locateResourceDir(QString("ui/images"));

	QDir::setCurrent( resourcesLocations[0] );

	setupCustomUi( parent );

	QDir::setCurrent( currentPath );

	_status = Idle;
}

CAMidiRecorderView::~CAMidiRecorderView() {
}

void CAMidiRecorderView::setupCustomUi( QWidget *parent ) {
	uiRecord = new QAction( QIcon("images/playback/record.svg"), tr("Record"), this );
	uiRecord->setObjectName( "uiRecord" );
	addAction( uiRecord );
	uiPause = new QAction( QIcon("images/playback/pause.svg"), tr("Pause"), this );
	uiPause->setObjectName( "uiPause" );
	addAction( uiPause );
	uiStop = new QAction( QIcon("images/playback/stop.svg"), tr("Stop"), this );
	uiStop->setObjectName( "uiStop" );
	addAction( uiStop );
	_time = new QLabel( "0:00", this );
	_timeAction = addWidget( _time );

	_timer = new QTimer();
	_timer->setInterval(1000);
	_timer->start();

	connect( _timer, SIGNAL(timeout()), this, SLOT(onTimerTimeout()) );

	QMetaObject::connectSlotsByName( this );

	uiRecord->setEnabled( true );
	uiPause->setVisible( false );
	uiStop->setEnabled( false );
}

void CAMidiRecorderView::onTimerTimeout() {
	if (_midiRecorder) {
		if ( _time->text().isEmpty() || _status != Pause ) {
			_time->setText( QString("%1:%2").arg((_midiRecorder->curTime()/1000)/60).arg((_midiRecorder->curTime()/1000) % 60, 2, 10, QChar('0')) );
		} else {
			_time->setText("");
		}
	}
}

void CAMidiRecorderView::on_uiPause_triggered(bool checked) {
	uiPause->setVisible( false );
	uiRecord->setVisible( true );

	_midiRecorder->pauseRecording();
	_status = Pause;
}

void CAMidiRecorderView::on_uiStop_triggered(bool checked) {
	uiRecord->setVisible( true );
	uiPause->setVisible( false );
	uiStop->setEnabled( false );

	_midiRecorder->stopRecording();
	_status = Idle;
}

void CAMidiRecorderView::on_uiRecord_triggered(bool checked) {
	uiRecord->setVisible( false );
	uiPause->setVisible( true );
	uiStop->setEnabled( true );

	_midiRecorder->startRecording();
	_status = Recording;
}
