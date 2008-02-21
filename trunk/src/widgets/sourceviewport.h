/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SOURCEVIEWPORT_H_
#define SOURCEVIEWPORT_H_

#include <QTextEdit>

#include "widgets/viewport.h"

class QPushButton;
class QGridLayout;

class CADocument;
class CAVoice;
class CALyricsContext;

class CASourceViewPort : public CAViewPort {
	Q_OBJECT
	
	enum CASourceViewPortType {
		LilyPond,
		CanorusML
	};
	
public:
	CASourceViewPort(CADocument *doc, QWidget *parent=0);
	CASourceViewPort(CAVoice *voice, QWidget *parent=0);
	CASourceViewPort(CALyricsContext *lc, QWidget *parent=0);
	virtual ~CASourceViewPort();
	
	CASourceViewPort *clone();
	CASourceViewPort *clone(QWidget *parent);
	
	inline CASourceViewPortType sourceViewPortType() { return _sourceViewPortType; }
	inline void setSourceViewPortType( CASourceViewPortType t ) { _sourceViewPortType = t; }
	
	inline CADocument *document() { return _document; };
	inline CAVoice *voice() { return _voice; }
	inline CALyricsContext *lyricsContext() { return _lyricsContext; }
	inline void setDocument( CADocument *doc ) { _document = doc; }
	inline void setVoice( CAVoice *voice ) { _voice = voice; }
	inline void setLyricsContext( CALyricsContext *c ) { _lyricsContext = c; }
	
	inline void selectAll() { _textEdit->selectAll(); }
signals:
	void CACommit( QString documentString, CASourceViewPort *v );
	
public slots:
	void rebuild();
	
private slots:
	void on_commit_clicked();
	
private:
	void setupUI();
	
	/////////////
	// Widgets //
	/////////////
	QTextEdit *_textEdit;
	QPushButton *_commit;
	QPushButton *_revert;
	QGridLayout *_layout;
	
	////////////////
	// Properties //
	////////////////
	CASourceViewPortType _sourceViewPortType;
	CADocument *_document;
	CAVoice *_voice;
	CALyricsContext *_lyricsContext;
};

#endif /* SOURCEVIEWPORT_H_ */
