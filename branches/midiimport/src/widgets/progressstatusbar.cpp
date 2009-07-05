/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QLabel>
#include <QProgressBar>
#include <QPushButton>

#include "widgets/progressstatusbar.h"

CAProgressStatusBar::CAProgressStatusBar( QWidget *parent )
 : QStatusBar(parent),
   _progressLabel(new QLabel("", this)),
   _progressBar(new QProgressBar(this)),
   _cancelButton(new QPushButton(tr("Cancel"), this)) {
	addWidget( _progressLabel );
	addWidget( _progressBar );
	addWidget( _cancelButton );

	connect( _cancelButton, SIGNAL(clicked(bool)), this, SLOT(on_cancelButton_clicked(bool)) );
}

CAProgressStatusBar::~CAProgressStatusBar() {
	delete _progressLabel;
	delete _progressBar;
	delete _cancelButton;
}

void CAProgressStatusBar::on_cancelButton_clicked( bool c ) {
	_cancelButton->setEnabled(false);

	emit( cancelButtonClicked(c) );
}

void CAProgressStatusBar::setProgress( QString label, int value ) {
	_progressLabel->setText( label );
	_progressBar->setValue( value );
}

void CAProgressStatusBar::setProgress( int value ) {
	_progressBar->setValue( value );
}

void CAProgressStatusBar::setProgress( QString label ) {
	_progressLabel->setText( label );
}
