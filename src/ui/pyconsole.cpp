/*!
	Copyright (c) 2006-2008, Stefan Sakalik, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <stdio.h>
#include <QKeyEvent>

#include "ui/pyconsole.h"
#include "canorus.h"


/*!
	\class CAPyConsole
	\brief Canorus console widget
	Class CAPyConsole is the GUI part of Canorus python-based command line interface.
	Utilizes CAPyConsoleInterface and pycli plugin to provide the functionality needed.

	Early version, still under development...
*/

// \todo document(QTextDocument) and _document(canorus document) -> similarity
// \todo pycli: interactive help won't work. More sys.std* overrides
// \todo command queuing, when busy
// \todo colors
// \todo event filtering
// CRLF on windows in qTextEdit ?
// \todo exit() terminates Canorus ?!? o_O

CAPyConsole::CAPyConsole( CADocument *doc, QWidget *parent) : QTextEdit(parent) {
	_parent = parent;

	setUndoRedoEnabled(false);
	setFontFamily("Courier");
	setFontPointSize(9.5);
	setPlainText("Not initialized\n");

	_iCurStart = _iCurNowOld = _iCurNow = textCursor().position();
	consoleCursor = new QTextCursor(document());
	consoleCursor->movePosition(QTextCursor::End);

	qStrFixed = toPlainText();
	qStrInput = "";

	connect(this, SIGNAL(thr_insertTextAtEnd(const QString &)), SLOT(insertTextAtEnd(const QString &)), Qt::QueuedConnection);
	connect(this, SIGNAL(thr_pluginInit()), SLOT(safePluginInit()), Qt::QueuedConnection);
	connect(this, SIGNAL(selectionChanged()), SLOT(selChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), SLOT(curPosChanged()));
	connect(this, SIGNAL(textChanged()), SLOT(txtChanged()));

	bCmdProcess = false;
	bufSend = "";

	bNoTxtChange = false;
}

/*!
	Slot for textChanged signal.

	The text is divided into two parts: fixed and input.
	User can't edit fixed part (from the beginning to ">>> " inclusive)
	Input is after ">>> ", so user is supposed to edit it.

	So, the text in the widget can obviously be changed:
	_allowed_
		1) user changes input -> save input into the variable (yes, after each keystroke)
		2) output from python arrives
	_not allowed_
		3) user attempts to change fixed text. It must be reverted into previous state
*/
void CAPyConsole::txtChanged(){
	if(bNoTxtChange)		// for example txtChanged() changes text sometimes, too
		return;

	// cursor is/was in not allowed position; revert
	if ((_iCurNow < _iCurStart) || (_iCurNowOld < _iCurStart)) {		
		//undo the last operation

		bNoTxtChange = true;
		setFontFamily("Courier");
		setFontPointSize(9.5);

		_iCurNow = _iCurNowOld;			// save cursor position (most of the times this works)
		setPlainText(qStrFixed + qStrInput);	// because here it changes

		QTextCursor newCur = textCursor();	//and move cursor back
		newCur.setPosition(_iCurNow = _iCurNowOld);
		setTextCursor(newCur);
		bNoTxtChange = false;

	} else {
		// input has changed, copy it
		consoleCursor->setPosition(_iCurStart);
		consoleCursor->movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
		qStrInput = consoleCursor->selectedText();
	}
	ensureCursorVisible();
}

/*!
	Appends text at the end of the documents; no extra newlines.
	WARNING: it also updates _iCur*; so >>> input information is lost
	emit thr_insertTextAtEnd, if accessing from different thread
*/
void CAPyConsole::insertTextAtEnd(const QString & text){
	bNoTxtChange = true;
	moveCursor(QTextCursor::End);
	textCursor().insertText(text);

	_iCurStart = _iCurNowOld = _iCurNow = textCursor().position();
	qStrFixed = toPlainText();
	qStrInput = "";
	bNoTxtChange = false;
}

/*!
	Tracks the position of the cursor.

	Saves old new cursor position.
	If old or new cursor pos is in the forbidden area, then txtChanged will revert the operation
	Undo/Redo mechanism is not used, because it's hard to control (or impossible?) Undo marks.
	Also we need to forbit user to undo
*/
void CAPyConsole::curPosChanged(){
	_iCurNowOld = _iCurNow;
	_iCurNow = textCursor().position();
}

/*!
	\todo: implementation
	User can with selected text overwrite non-overwritable text
*/
void CAPyConsole::selChanged(){

}


/*!
	when plugin initializes
	\todo: no more pyCLIs than one
	\todo: is this approach safe? (future)
*/ 
void CAPyConsole::safePluginInit(void){
	if(_parent != NULL)
		_parent->show();
}

// Qt is not thread-safe, use signals/slots
void CAPyConsole::plugin_init(void){
	emit thr_pluginInit(); 
}

/*!
	Pycli calls this function (through CAPyConsoleInterface) to get input from the console
	\todo: remove QString prompt; (do we need to know which prompt is used ps1/ps2?)
*/
QString CAPyConsole::buffered_input(QString prompt){
	emit thr_insertTextAtEnd(prompt);			//thread safe

	while(!bCmdProcess);	// wait for some input
	bCmdProcess = false;

	QString *str = new QString(bufSend);	//put contents of bufSend into buffer \todo: synch
	if (bufSend == "html") {
		std::cout << toHtml().toAscii().data() << std::endl; 
	}
	bufSend = "";
	return *str;
}

/*
	Pycli calls this function (through CAPyConsoleInterface) to send some output to console
*/
void CAPyConsole::buffered_output(QString bufInp, bool bStdErr){
	emit thr_insertTextAtEnd(bufInp);			//thread safe
}

/*
	Watches for some keyboard events:
	Enter pressed: run the command / queue
	\todo: these
	Escape sequences (at least ctrl+C)
	design
*/
void CAPyConsole::keyPressEvent (QKeyEvent * e) {
	//sometimes it's Key_Enter instead of Key_Return, weird
	if ((e->key() == Qt::Key_Return) || (e->key() == Qt::Key_Enter)){
		moveCursor(QTextCursor::End);
		bufSend = qStrInput;
		QTextEdit::keyPressEvent(e);
		insertTextAtEnd("");		//update all necessary pointers, and stuff
		bCmdProcess = true;
	} else
		QTextEdit::keyPressEvent(e);

}
