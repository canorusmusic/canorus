/*!
	Copyright (c) 2006, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef SOURCEVIEW_H_
#define SOURCEVIEW_H_

#include <QTextEdit>

#include "widgets/view.h"

class QPushButton;
class QGridLayout;

class CADocument;
class CAVoice;
class CALyricsContext;

class CASourceView : public CAView {
	Q_OBJECT

	enum CASourceViewType {
		LilyPond,
		CanorusML
	};

public:
	CASourceView(CADocument *doc, QWidget *parent=0);
	CASourceView(CAVoice *voice, QWidget *parent=0);
	CASourceView(CALyricsContext *lc, QWidget *parent=0);
	virtual ~CASourceView();

	CASourceView *clone();
	CASourceView *clone(QWidget *parent);

	inline CASourceViewType sourceViewType() { return _sourceViewType; }
	inline void setSourceViewType( CASourceViewType t ) { _sourceViewType = t; }

	inline CADocument *document() { return _document; };
	inline CAVoice *voice() { return _voice; }
	inline CALyricsContext *lyricsContext() { return _lyricsContext; }
	inline void setDocument( CADocument *doc ) { _document = doc; }
	inline void setVoice( CAVoice *voice ) { _voice = voice; }
	inline void setLyricsContext( CALyricsContext *c ) { _lyricsContext = c; }

	inline void selectAll() { _textEdit->selectAll(); }
signals:
	void CACommit( QString documentString );

public slots:
	void rebuild();

private slots:
	void on_commit_clicked();

private:
	void setupUI();

	class CATextEdit;
	friend class CASourceView::CATextEdit;

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
	CASourceViewType _sourceViewType;
	CADocument *_document;
	CAVoice *_voice;
	CALyricsContext *_lyricsContext;
};

#endif /* SOURCEVIEW_H_ */
