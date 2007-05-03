/*! 
 * Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QTextStream>

#include "core/canorusml.h"
#include "widgets/sourceviewport.h"
#include "core/document.h"
#include "core/voice.h"

#include "export/lilypondexport.h"
#include "import/lilypondimport.h"

/*!
	\class CASourceViewPort
	\brief Widget that shows the current score source in various syntax
	
	This widget is a viewport which shows in the main text area the syntax of the current score (or voice, staff).
	It includes 2 buttons for committing the changes to the score and reverting any changes back from the score.
	
	\sa CAScoreViewPort
*/

/*!
	Constructor for CanorusML syntax - requires the whole document.
	
	\todo This should be merged in the future with other formats.
*/
CASourceViewPort::CASourceViewPort(CADocument *doc, QWidget *parent)
 : CAViewPort(parent) {
 	_viewPortType = CAViewPort::SourceViewPort;
 	_document = doc;
 	_voice = 0;
  	_lyricsContext = 0;
 	
 	setupUI();
}

/*!
	Constructor for LilyPond syntax - requires the current voice.
	
	\todo This should be merged in the future with other formats.
*/
CASourceViewPort::CASourceViewPort(CAVoice *voice, QWidget *parent)
 : CAViewPort(parent) {
 	_viewPortType = CAViewPort::SourceViewPort;
 	_document = 0;
 	_voice = voice;
 	_lyricsContext = 0;
 	
 	setupUI();
}

/*!
	Constructor for LilyPond syntax - requires the current lyrics context to show the lyrics.
	
	\todo This should be merged in the future with other formats.
*/
CASourceViewPort::CASourceViewPort(CALyricsContext *lc, QWidget *parent)
 : CAViewPort(parent) {
 	_viewPortType = CAViewPort::SourceViewPort;
 	_document = 0;
 	_voice = 0;
  	_lyricsContext = lc;
 	
 	setupUI();
}

void CASourceViewPort::setupUI() {
	_layout = new QGridLayout(this);
	_layout->addWidget(_textEdit = new QTextEdit(0));
	_layout->addWidget(_commit = new QPushButton("Commit changes"));
	_layout->addWidget(_revert = new QPushButton("Revert changes"));
	
	connect(_commit, SIGNAL(clicked()), this, SLOT(on_commit_clicked()));
	connect(_revert, SIGNAL(clicked()), this, SLOT(rebuild()));
	
	rebuild();
}

CASourceViewPort::~CASourceViewPort() {
	_textEdit->disconnect();
	_commit->disconnect();
	_revert->disconnect();
	_layout->disconnect();
	
	delete _textEdit;
	delete _commit;
	delete _revert;
	delete _layout;
}

void CASourceViewPort::on_commit_clicked() {
	emit CACommit(this, _textEdit->toPlainText());
}

void CASourceViewPort::mousePressEvent(QMouseEvent *e) {
	emit CAMousePressEvent( e, QPoint(0,0), this);
}

CASourceViewPort *CASourceViewPort::clone() {
	CASourceViewPort *v;
	if (_document)
		v = new CASourceViewPort(_document, _parent);
	else if (_voice)
		v = new CASourceViewPort(_voice, _parent);
	
	return v;
}

CASourceViewPort *CASourceViewPort::clone(QWidget *parent) {
	CASourceViewPort *v;
	if (_document)
		v = new CASourceViewPort(_document, parent);
	else if (_voice)
		v = new CASourceViewPort(_voice, parent);
	
	return v;
}

/*!
	Generates the score source from the current score and fill the text area with it.
*/ 
void CASourceViewPort::rebuild() {
	_textEdit->clear();
	
	QString *value = new QString();
	QTextStream stream(value);
	
	// CanorusML
	if (document()) {
		CACanorusML::saveDocument(document(), stream);
	} else
	// LilyPond
	if (voice()) {
		CALilyPondExport(voice(), &stream);
	} else
	if (lyricsContext()) {
		CALilyPondExport(lyricsContext(), &stream);
	}
	
	_textEdit->insertPlainText(*value);
	
	delete value;
}
