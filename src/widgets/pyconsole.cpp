/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "scripting/swigpython.h" // Must be included first (includes Python.h).
#include "widgets/pyconsole.h"
#include "canorus.h"

#include <stdio.h>
#include <QKeyEvent>


/*!
	\class CAPyConsole
	\brief Canorus console widget
	Class CAPyConsole is the GUI part of Canorus python-based command line interface.
	Utilizes CAPyConsoleInterface and pycli plugin to provide the functionality needed.
	\warning exit() terminates whole Canorus
	\warning trying to execute more than one pyCLI (from CLI menu) => undefined behaviour

	Still under development (beta)
*/


// --python shell emulation; script interaction--
// \todo pycli: interactive help won't work. More sys.std* overrides?
// \todo command queuing, when busy
// \todo [CRLF on windows] in qTextEdit & python
// \todo (design) remove unnecessary _fmtNormal changes
// \todo signals, interrupts, exit() exits Canorus, maybe ExternProgram / separate process

// --Behaviour, internal commands (at the bottom of source)--
// \todo redesign 1: fragility, unstability (near future)
// \todo redesign 2: CAPyConsole is no more TextWidget (far future)
// \todo design: history <-> text edition <-> text changed signal <-> text Revert
// \todo design: specify behaviour
//
// \todo maybe TABS? -> anyone wants this?
//
// \todo working -> internal commands (???)
//
// Internal commands: are entered to python shell
// 			looks like this "/commandx", have / at the beginning
// 			are interpreted in canorus not python
//
// /todo autoindentation, internal variable (like in idle), test it!

CAPyConsole::CAPyConsole( CADocument *doc, QWidget *parent) : QTextEdit(parent) {
	_parent = parent;

	histIndex = -1;
	bufSend = "";
	strInput = "";
	bNoTxtChange = false;

	setUndoRedoEnabled(false);
	setFontFamily("Courier");
	setFontPointSize(9);

	_fmtStderr = _fmtStdout = _fmtNormal = currentCharFormat();
	_fmtStdout.setForeground(QColor(0,0,255));
	_fmtStderr.setForeground(QColor(255,0,0));

	// this must be set for txtChanged [bNoTxtChange=false] not to crash
	consoleCursor = new QTextCursor(document());
	consoleCursor->movePosition(QTextCursor::End);

	// slots
	qRegisterMetaType<TxtType>("TxtType");
	connect(this, SIGNAL(textChanged()), SLOT(txtChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), SLOT(posChanged()));
	connect(this, SIGNAL(selectionChanged()), SLOT(selChanged()));
	connect(this, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), SLOT(fmtChanged()));

	// thread
	connect(this, SIGNAL(thr_insertTextAtEnd(const QString &, TxtType)), SLOT(insertTextAtEnd(const QString &, TxtType)), Qt::QueuedConnection);
	connect(this, SIGNAL(thr_pluginInit()), SLOT(safePluginInit()), Qt::QueuedConnection);

	thrWaitMut = new QMutex();
	thrWait = new QWaitCondition();
}

// --------------------------------
// ----------FUNCTIONS-------------
// --------------------------------

/*!
	Revert text to previous state (like undo)
*/
void CAPyConsole::txtRevert() {
	bNoTxtChange = true;
	clear();
	QList<TxtFragment*>::iterator i;
	for(i = txtFixed.begin(); i != txtFixed.end(); i++) {
		switch((*i)->type) {
		case txtNormal:
			setCurrentCharFormat(_fmtNormal);
			break;

		case txtStdout:
			setCurrentCharFormat(_fmtStdout);
			break;

		case txtStderr:
			setCurrentCharFormat(_fmtStderr);
			break;
		}
		insertPlainText((*i)->text);
	}
	setCurrentCharFormat(_fmtNormal);
	insertPlainText(strInput);
	bNoTxtChange = false;
}

// --------------------------------
// -------------SLOTS--------------
// --------------------------------

/*!
	Appends text at the end of the documents; no extra newlines.
	emit thr_insertTextAtEnd, if accessed from thread (it is slot too)
	WARNING: it also updates cursor positions => user input is lost
*/
void CAPyConsole::insertTextAtEnd(const QString & text, TxtType txtType) {
	bNoTxtChange = true;

	// backup
	TxtFragment *tf = new TxtFragment;
	tf->text = text;
	tf->type = txtType;
	txtFixed += tf;

	moveCursor(QTextCursor::End);
	switch(txtType) {
	case txtNormal:
		setCurrentCharFormat(_fmtNormal);
		break;

	case txtStdout:
		setCurrentCharFormat(_fmtStdout);
		break;

	case txtStderr:
		setCurrentCharFormat(_fmtStderr);
		break;
	}
	insertPlainText(text);
	setCurrentCharFormat(_fmtNormal);

	_iCurStart = _iCurNowOld = _iCurNow = textCursor().position();
	strInput = "";
	bNoTxtChange = false;
}

/*!
	Slot for textChanged signal.

	Input is after '>>> ', user can edit it: strInput
	Text before '>>> ' inclusive -> is not editable: txtFixed

	So, the text in the widget might be changed:
	Allowed change:
		1) user changes input -> save input into the variable (yes, after each keystroke)
		2) output from python arrives
	Not allowed change:
		3) user attempts to change fixed text. It must be reverted into previous state
*/
void CAPyConsole::txtChanged() {
	if(bNoTxtChange)		// for example txtChanged() changes text sometimes, too
		return;

	// cursor is/was in not allowed position; revert
	if ((_iCurNow < _iCurStart) || (_iCurNowOld < _iCurStart)) {
		bNoTxtChange = true;

		int icno = _iCurNowOld;	// save cursor pos. before text was modified
		txtRevert();

		_iCurNowOld = _iCurNow = icno;
		newCur = textCursor();
		newCur.setPosition(_iCurNow);
		setTextCursor(newCur);
		setCurrentCharFormat(_fmtNormal);

		bNoTxtChange = false;

	} else {
		// input has changed, copy it
		consoleCursor->setPosition(_iCurStart);
		consoleCursor->movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
		strInput = consoleCursor->selectedText();
	}
	ensureCursorVisible();
}

/*!
	Tracks the position of the cursor.

	Saves old new cursor position.
	If old or new cursor pos is in the forbidden area, then txtChanged will revert the operation
	Undo/Redo mechanism is not used, because it's hard to control (or impossible?) Undo marks.
	Also we need to forbit user to undo
*/
void CAPyConsole::posChanged() {
	_iCurNowOld = _iCurNow;
	_iCurNow = textCursor().position();

	if(_iCurNow == _iCurStart)		// TextCursor can change format
		setCurrentCharFormat(_fmtNormal);
}

/*!
	\todo: implementation
	User can with selected text overwrite non-overwritable text
*/
void CAPyConsole::selChanged() {

}

/*!
	\todo: implementation maybe?
*/
void CAPyConsole::fmtChanged() {
	
}


// --------------------------------
// --------THREAD-RELATED----------
// --------------------------------

/*!
	when plugin initializes
	\todo: no more pyCLIs than one
	\todo: is this approach safe? (future)
*/ 
void CAPyConsole::safePluginInit(void) {
	if(_parent != NULL)
		_parent->show();
	setFocus(Qt::OtherFocusReason);
}

/*!
	Emit signal thr_pluginInit -> slot safePluginInit (synchronized)
*/
void CAPyConsole::plugin_init(void) {
	emit thr_pluginInit(); 
}

/*!
	Pycli calls this function (through CAPyConsoleInterface) to get input from the console
	\todo: remove QString prompt; (do we need to know which prompt is used ps1/ps2?)
*/
QString CAPyConsole::buffered_input(QString prompt) {
	//prompt = "~";
	emit thr_insertTextAtEnd(prompt, txtNormal);

	thrWaitMut->lock();
	thrWait->wait(thrWaitMut);

	QString *str = new QString(bufSend);	//put contents of bufSend into buffer \todo: synch
	if (bufSend == "html") {
		std::cout << toHtml().toAscii().data() << std::endl; 
	}
	bufSend = "";
	
	thrWaitMut->unlock();

	return *str;
}

/*
	Pycli calls this function (through CAPyConsoleInterface) to send some output to console
*/
void CAPyConsole::buffered_output(QString bufInp, bool bStderr) {
	if(bStderr)
		emit thr_insertTextAtEnd(bufInp, txtStderr);
	else
		emit thr_insertTextAtEnd(bufInp, txtStdout);
}

// --------------------------------
// -------EVENTS,BEHAVIOUR---------
// --------------------------------

// history, hist* functions take care of strInput

void CAPyConsole::histAdd() {
	// dont add "" to history
	if(strInput == "")
		return;

	// dont add item, that's already at the end
	if(!histList.isEmpty())
		if(strInput == histList[0])
			return;

	histIndex = -1;
	histList.prepend(strInput);
}

void CAPyConsole::histGet(bool prev) {
	// must do it twice, because of rules
	moveCursor(QTextCursor::End);

	if (histList.isEmpty())
		return;

	// error check (just for debugging)
	if (histIndex < -1){
		std::cout << "ERROR!: PyConsole->histGet index below -1" << std::endl;
		histIndex = -1;
		
	} else if (histIndex >= histList.size()) {
		std::cout << "ERROR!: PyConsole->histGet index above MAX" << std::endl;
		histIndex = histList.size();
	}
		
	if (histIndex == -1)
		histOldInput = strInput;
	else
		histList[histIndex] = strInput;

	consoleCursor->setPosition(_iCurStart);
	consoleCursor->movePosition(QTextCursor::End,QTextCursor::KeepAnchor);

	if(prev)
		histIndex++;
	else
		histIndex--;

	if(histIndex < 0) {
		histIndex = -1;
		consoleCursor->insertText(strInput = histOldInput);
		return;

	} else if (histIndex >= histList.size())
		histIndex = histList.size() - 1;

	consoleCursor->setCharFormat(_fmtNormal);
	consoleCursor->insertText(strInput = histList[histIndex]);
}

/*
	Watches for some keyboard events:
	Enter pressed: run the command
	\todo queue
*/
void CAPyConsole::keyPressEvent (QKeyEvent * e) {
	bool defCase = false;
	//sometimes it's Key_Enter instead of Key_Return, weird
	switch (e->key()) {
	case Qt::Key_Return: 
	case Qt::Key_Enter:
		moveCursor(QTextCursor::End);
		histAdd();
		bufSend = strInput;

		tf = new TxtFragment;
		tf->text = strInput + "\n";
		tf->type = txtNormal;
		txtFixed += tf;

		QTextEdit::keyPressEvent(e);
		insertTextAtEnd("");		//update all necessary pointers, and stuff
		thrWait->wakeOne();
		break;

	case Qt::Key_Up:
		bNoTxtChange = true;
		histGet(true);
		bNoTxtChange = false;
		break;

	case Qt::Key_Down:
		bNoTxtChange = true;
		histGet(false);
		bNoTxtChange = false;
		break;

	case Qt::Key_Left:
		if(_iCurNow <= _iCurStart) {
			_iCurNowOld = _iCurNow = _iCurStart;
			newCur = textCursor();
			newCur.setPosition(_iCurNow);
			setTextCursor(newCur);
			setCurrentCharFormat(_fmtNormal); }
		else
			defCase = true;
		break;

	case Qt::Key_Home:
		_iCurNowOld = _iCurNow = _iCurStart;
		newCur = textCursor();
		newCur.setPosition(_iCurNow);
		setTextCursor(newCur);
		setCurrentCharFormat(_fmtNormal);
		break;

	case Qt::Key_Backspace:
		if(_iCurNow > _iCurStart)
			defCase = true;	
		break;
	
	default:
		defCase = true;
		break;

	} if(defCase) {			// enhanced default: (switch)
		if(_iCurNow == _iCurStart)
			setCurrentCharFormat(_fmtNormal);

		QTextEdit::keyPressEvent(e);
	}
}

// --------------------------------
// -------INTERNAL-COMMANDS--------
// --------------------------------
// internal commands starts with "/"

