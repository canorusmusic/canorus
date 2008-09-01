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
}

CAMidiRecorderView::~CAMidiRecorderView() {
}

void CAMidiRecorderView::setupCustomUi( QWidget *parent ) {
	uiPlay = new QAction( QIcon("images/playback/play.svg"), tr("Play"), this );
	uiPlay->setObjectName( "uiPlay" );
	addAction( uiPlay );
	uiPause = new QAction( QIcon("images/playback/pause.svg"), tr("Pause"), this );
	uiPause->setObjectName( "uiPause" );
	addAction( uiPause );
	uiStop = new QAction( QIcon("images/playback/stop.svg"), tr("Stop"), this );
	uiStop->setObjectName( "uiStop" );
	addAction( uiStop );
	uiRecord = new QAction( QIcon("images/playback/record.svg"), tr("Record"), this );
	uiRecord->setObjectName( "uiRecord" );
	addAction( uiRecord );
	_slider = new QSlider( Qt::Horizontal, this );
	_sliderAction = addWidget( _slider );
	_time = new QLabel( "0:50", this );
	_timeAction = addWidget( _time );

	QMetaObject::connectSlotsByName( this );

	uiPause->setVisible( false );
	uiStop->setEnabled( false );
	uiPlay->setEnabled( false );
	uiRecord->setEnabled( true );
	_status = Idle;
}

void CAMidiRecorderView::on_uiPlay_triggered(bool checked) {
	uiPlay->setVisible( false );
	uiPause->setVisible( true );
	_status = Playing;

	// TODO
}

void CAMidiRecorderView::on_uiPause_triggered(bool checked) {
	uiPause->setVisible( false );
	uiPlay->setVisible( true );

	// TODO
}

void CAMidiRecorderView::on_uiStop_triggered(bool checked) {
	uiPlay->setVisible( false );
	uiPause->setVisible( true );

	if ( _status==Recording ) {
		_midiRecorder->stopRecording();
	}

	_status = Idle;
}

void CAMidiRecorderView::on_uiRecord_triggered(bool checked) {
	uiPlay->setEnabled( false );
	uiRecord->setEnabled( false );
	uiStop->setEnabled( true );

	_midiRecorder->startRecording();
	_status = Recording;
}
