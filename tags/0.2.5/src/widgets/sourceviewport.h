/** @file widgets/sourceviewport.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef SOURCEVIEWPORT_H_
#define SOURCEVIEWPORT_H_

#include "widgets/viewport.h"

class QTextEdit;
class QPushButton;
class QGridLayout;

class CADocument;
class CAVoice;

class CASourceViewPort : public CAViewPort {
	Q_OBJECT
	
public:
	CASourceViewPort(CADocument *doc, QWidget *parent=0);
	CASourceViewPort(CAVoice *voice, QWidget *parent=0);
	virtual ~CASourceViewPort();
	
	CASourceViewPort *clone();
	CASourceViewPort *clone(QWidget *parent);
	
	inline CADocument *document() { return _document; };
	inline CAVoice *voice() { return _voice; }
	
signals:
	void CACommit(CASourceViewPort *v, QString documentString);

private slots:
	void on_commit_clicked();
	void rebuild();
	
private:
	void mousePressEvent( QMouseEvent* );
	void setupUI();
	
	////////////////////////////////////////////////
	//Widgets
	////////////////////////////////////////////////
	QTextEdit *_textEdit;
	QPushButton *_commit;
	QPushButton *_revert;
	QGridLayout *_layout;
	
	////////////////////////////////////////////////
	//Properties
	////////////////////////////////////////////////
	CADocument *_document;
	CAVoice *_voice;
};

#endif /*SOURCEVIEWPORT_H_*/
