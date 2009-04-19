/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QTextStream>
#include <QMouseEvent>

#include "export/canorusmlexport.h"
#include "widgets/sourceview.h"
#include "score/document.h"
#include "score/voice.h"

#include "export/lilypondexport.h"
#include "import/lilypondimport.h"

class CASourceView::CATextEdit : public QTextEdit {
	public:
		CATextEdit(CASourceView* v) : QTextEdit(v), _view(v) {}
	protected:
		void focusInEvent(QFocusEvent* event) {
			QTextEdit::focusInEvent(event);
			QMouseEvent fake(QEvent::MouseButtonPress, QCursor::pos(), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
			_view->mousePressEvent(&fake);
		}
	private:
		CASourceView* _view;
};

/*!
	\class CASourceView
	\brief Widget that shows the current score source in various syntax

	This widget is a view which shows in the main text area the syntax of the current score (or voice, staff).
	It includes 2 buttons for committing the changes to the score and reverting any changes back from the score.

	\sa CAScoreView
*/

/*!
	Constructor for CanorusML syntax - requires the whole document.

	\todo This should be merged in the future with other formats.
*/
CASourceView::CASourceView(CADocument *doc, QWidget *parent)
 : CAView(parent) {
 	setViewType( SourceView );
 	setSourceViewType( CanorusML );
 	_document = doc;
 	_voice = 0;
  	_lyricsContext = 0;

 	setupUI();
}

/*!
	Constructor for LilyPond syntax - requires the current voice.

	\todo This should be merged in the future with other formats.
*/
CASourceView::CASourceView(CAVoice *voice, QWidget *parent)
 : CAView(parent) {
 	setViewType( SourceView );
 	setSourceViewType( LilyPond );
 	_document = 0;
 	_voice = voice;
 	_lyricsContext = 0;

 	setupUI();
}

/*!
	Constructor for LilyPond syntax - requires the current lyrics context to show the lyrics.

	\todo This should be merged in the future with other formats.
*/
CASourceView::CASourceView(CALyricsContext *lc, QWidget *parent)
 : CAView(parent) {
 	setViewType( SourceView );
  	setSourceViewType( LilyPond );
 	_document = 0;
 	_voice = 0;
  	_lyricsContext = lc;

 	setupUI();
}

void CASourceView::setupUI() {
	_layout = new QGridLayout(this);
	_layout->addWidget(_textEdit = new CATextEdit(this));
	_layout->addWidget(_commit = new QPushButton(tr("Commit changes")));
	_layout->addWidget(_revert = new QPushButton(tr("Revert changes")));

	connect(_commit, SIGNAL(clicked()), this, SLOT(on_commit_clicked()));
	connect(_revert, SIGNAL(clicked()), this, SLOT(rebuild()));

	rebuild();
}

CASourceView::~CASourceView() {
	_textEdit->disconnect();
	_commit->disconnect();
	_revert->disconnect();
	_layout->disconnect();

	delete _textEdit;
	delete _commit;
	delete _revert;
	delete _layout;
}

void CASourceView::on_commit_clicked() {
	emit CACommit( _textEdit->toPlainText() );
}

CASourceView *CASourceView::clone() {
	CASourceView *v;
	if ( document() )
		v = new CASourceView( document(), static_cast<QWidget*>(parent()) );
	else if ( voice() )
		v = new CASourceView( voice(), static_cast<QWidget*>(parent()) );
	else if ( lyricsContext() )
		v = new CASourceView( lyricsContext(), static_cast<QWidget*>(parent()) );

	return v;
}

CASourceView *CASourceView::clone(QWidget *parent) {
	CASourceView *v;
	if ( document() )
		v = new CASourceView( document(), parent );
	else if ( voice() )
		v = new CASourceView( voice(), parent );
	else if ( lyricsContext() )
		v = new CASourceView( lyricsContext(), parent );

	return v;
}

/*!
	Generates the score source from the current score and fill the text area with it.
*/
void CASourceView::rebuild() {
	_textEdit->clear();

	QString *value = new QString();
	QTextStream stream(value);

	// CanorusML
	if ( document() ) {
		CACanorusMLExport save( &stream );
		save.exportDocument( document() );
		save.wait();
	} else {
		CALilyPondExport le( &stream );
		// LilyPond
		if (voice()) {
			le.exportVoice( voice() );
			le.wait();
		} else
		if (lyricsContext()) {
			le.exportLyricsContext( lyricsContext() );
			le.wait();
		}
	}

	_textEdit->insertPlainText(*value);

	delete value;
}
