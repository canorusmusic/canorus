/** @file widgets/sourceviewport.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QTextStream>
#include <iostream>	//DEBUG
#include "core/canorusml.h"
#include "widgets/sourceviewport.h"
#include "core/document.h"

CASourceViewPort::CASourceViewPort(CADocument *doc, QWidget *parent)
 : CAViewPort(parent) {
 	_viewPortType = CAViewPort::SourceViewPort;
 	
 	_document = doc;
 	
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
	emit CACommit(_textEdit->toPlainText());
}

CASourceViewPort *CASourceViewPort::clone() {
	CASourceViewPort *v = new CASourceViewPort(_document, _parent);
	
	return v;
}

CASourceViewPort *CASourceViewPort::clone(QWidget *parent) {
	CASourceViewPort *v = new CASourceViewPort(_document, parent);
	
	return v;
}

void CASourceViewPort::rebuild() {
	_textEdit->clear();
	
	QString *value = new QString();
	QTextStream stream(value);
	CACanorusML::saveDocument(stream, _document);
	
	_textEdit->insertPlainText(*value);
	
	delete value;
}
