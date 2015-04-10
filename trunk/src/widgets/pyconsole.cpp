/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/


#include "scripting/swigpython.h" // Must be included first (includes Python.h).
#include "interface/plugin.h"
#include "interface/pluginmanager.h"
#include "widgets/pyconsole.h"
#include "canorus.h"

#include <stdio.h>
#include <QKeyEvent>
#include <qwaitcondition.h>

#ifdef USE_PYTHON
/*!
	\class CAPyConsole
	\brief Canorus console widget
	Class CAPyConsole is the GUI part of Canorus python-based command line interface.
	Utilizes CAPyConsoleInterface and pycli plugin to provide the functionality needed.
	\warning exit() terminates whole Canorus
	\warning trying to execute more than one pyCLI (from CLI menu) => undefined behaviour

	When plugin tries to interact with this; it calls asyncInput
        -> when plugin is terminated, mutexes must be cleared

	Still under development (beta)
*/


// --python shell emulation; script interaction--
// document as parameter won't work (solved like in pluginaction.cpp)
// \todo pycli: interactive help won't work. More sys.std* overrides?
// \todo command queuing, when busy
// \todo [CRLF on windows] in QTextEdit & python
// \todo (design) remove unnecessary _fmtNormal changes
// \todo processes

// --Behaviour, internal commands (at the bottom of source)--
// \todo redesign 2: CAPyConsole TextWidget (future)
// \todo design: history <-> text edition <-> text changed signal <-> text Revert
// \todo design: specify behaviour
//
// \todo maybe TABS? -> anyone wants this?
//
//
// Internal commands: are entered to python shell
// 			looks like this "/commandx", have / at the beginning
// 			are interpreted in canorus not python
//     "/callscript <script>", "/entryfunc", "entryfunc <function>"
//
// /todo autoindentation, internal variable (like in idle), test it!

CAPyConsole::CAPyConsole( CADocument *doc, QWidget *parent) : QTextEdit(parent) {
	_parent = parent;
	_canorusDoc = doc;
	_strEntryFunc = "main";

	_histIndex = -1;
	_bufSend = "";
	_strInput = "";
	_bIgnTxtChange = false;
	_iCurStart = 0;

	setUndoRedoEnabled(false);
	setFontFamily("Courier 10 Pitch");
	setFontPointSize(9);

	_fmtStderr = _fmtStdout = _fmtNormal = currentCharFormat();
	_fmtStdout.setForeground(QColor(0,0,255));
	_fmtStderr.setForeground(QColor(255,0,0));

	// this must be set for on_txtChanged [_bIgnTxtChange=false] not to crash
	_curInput = QTextCursor(document());
	_curInput.movePosition(QTextCursor::End);

	// slots
	qRegisterMetaType<TxtType>("TxtType");
	connect(this, SIGNAL(textChanged()), SLOT(on_txtChanged()));
	connect(this, SIGNAL(cursorPositionChanged()), SLOT(on_posChanged()));
	connect(this, SIGNAL(selectionChanged()), SLOT(on_selChanged()));
	connect(this, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), SLOT(on_fmtChanged()));

	// thread
	connect(this, SIGNAL(sig_txtAppend(const QString &, TxtType)), SLOT(txtAppend(const QString &, TxtType)), Qt::QueuedConnection);
	connect(this, SIGNAL(sig_syncPluginInit()), SLOT(syncPluginInit()), Qt::QueuedConnection);

	_thrWaitMut = new QMutex();
	_thrWait = new QWaitCondition();

	_thrIntrWaitMut = new QMutex();
	_thrIntrWait = new QWaitCondition();
}

// --------------------------------
// ----------FUNCTIONS-------------
// --------------------------------

/*!
	Revert text to previous state (like undo)
*/
void CAPyConsole::txtRevert() {
	_bIgnTxtChange = true;
	clear();
	QList<TxtFragment*>::iterator i;
	for(i = _txtFixed.begin(); i != _txtFixed.end(); i++) {
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
	insertPlainText(_strInput);
	_bIgnTxtChange = false;
}

/*!
    Get console input text. In most cases call txtGetInput()
    if bReadText is true, input will be reconstructed from text [used in on_txtChanged]
        otherwise, _strInput will be returned (default)
*/
QString CAPyConsole::txtGetInput(bool bReadText) {
    if(bReadText){
        _curInput.setPosition(_iCurStart);
        _curInput.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
        return _curInput.selectedText();
    } else
        return _strInput;
}

/*!
    Set console input text. In most cases call txtSetInput(QString bUpdateText)
    if bUpdateText is false,_strInput will be just set, [used in on_txtChanged]
        otherwise, text in console will be updated
*/
void CAPyConsole::txtSetInput(QString input, bool bUpdateText) {
    _strInput = input;
    if(bUpdateText) {
        bool ignWasSet = _bIgnTxtChange;

        _bIgnTxtChange = true;
        _curInput.setPosition(_iCurStart);
        _curInput.movePosition(QTextCursor::End,QTextCursor::KeepAnchor);
        _curInput.insertText(_strInput);
        _bIgnTxtChange = ignWasSet;
    }
}

// --------------------------------
// -------------SLOTS--------------
// --------------------------------

/*!
	Appends text at the end of the documents; no extra newlines.
	emit sig_txtAppend, if accessed from thread (it is slot too)
	WARNING: it also updates cursor positions => user input is lost
*/
void CAPyConsole::txtAppend(const QString & text, TxtType txtType) {
	_bIgnTxtChange = true;

	// backup
	_tf = new TxtFragment;
	_tf->text = text;
	_tf->type = txtType;
	_txtFixed += _tf;

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
	_strInput = "";
	ensureCursorVisible();

	_bIgnTxtChange = false;
}

/*!
	Slot for textChanged signal.

	Input is after '>>> ', user can edit it: _strInput
	Text before '>>> ' inclusive -> is not editable: _txtFixed

	So, the text in the widget might be changed:
	Allowed change:
		1) user changes input -> save input into the variable (yes, after each keystroke)
		2) output from python arrives
	Not allowed change:
		3) user attempts to change fixed text. It must be reverted into previous state
*/
void CAPyConsole::on_txtChanged() {
	if(_bIgnTxtChange)		// for example on_txtChanged() changes text sometimes, too
		return;

	// cursor is/was in not allowed position; revert
	if ((_iCurNow < _iCurStart) || (_iCurNowOld < _iCurStart)) {
		_bIgnTxtChange = true;

		int icno = _iCurNowOld;	// save cursor pos. before text was modified
		txtRevert();

		_iCurNowOld = _iCurNow = icno;
		_curNew = textCursor();
		_curNew.setPosition(_iCurNow);
		setTextCursor(_curNew);
		setCurrentCharFormat(_fmtNormal);

		_bIgnTxtChange = false;

	} else {
		// input was changed, update it -> special setInput and getInput calls, see doc
		txtSetInput(txtGetInput(true),false);
	}
	ensureCursorVisible();
}

/*!
	Tracks the position of the cursor.

	Saves old new cursor position.
	If old or new cursor pos is in the forbidden area, then on_txtChanged will revert the operation
	Undo/Redo mechanism is not used, because it's hard to control (or impossible?) Undo marks.
	Also we need to forbit user to undo
*/
void CAPyConsole::on_posChanged() {
	_iCurNowOld = _iCurNow;
	_iCurNow = textCursor().position();

	if(_iCurNow == _iCurStart)		// TextCursor can change format
		setCurrentCharFormat(_fmtNormal);
}

/*!
	\todo: implementation
	User can with selected text overwrite non-overwritable text
*/
void CAPyConsole::on_selChanged() {

}

/*!
	\todo: implementation maybe?
*/
void CAPyConsole::on_fmtChanged() {

}


// --------------------------------
// --------THREAD-RELATED----------
// --------------------------------

/*!
	when plugin initializes
	\todo: no more pyCLIs than one
	\todo: is this approach safe? (future)
*/
void CAPyConsole::syncPluginInit(void) {
	if(_parent != NULL)
		_parent->show();
	setFocus(Qt::OtherFocusReason);
}

/*!
	Emit signal sig_syncPluginInit -> slot syncPluginInit (synchronized)
*/
void CAPyConsole::asyncPluginInit(void) {
	emit sig_syncPluginInit();
}

/*!
	Pycli calls this function (through CAPyConsoleInterface) to get input from the console
	\todo: remove QString prompt; (do we need to know which prompt is used ps1/ps2?)
*/
QString CAPyConsole::asyncBufferedInput(QString prompt) {
    
	emit sig_txtAppend(prompt, txtNormal);

// blocking operation;
PyThreadState_Swap(CASwigPython::mainThreadState);
PyEval_ReleaseLock();

//Py_BEGIN_ALLOW_THREADS
	_thrWaitMut->lock();
	_thrWait->wait(_thrWaitMut);
//Py_END_ALLOW_THREADS

PyEval_AcquireLock();
PyThreadState_Swap(CASwigPython::pycliThreadState);

	QString *str = new QString(_bufSend);	//put contents of _bufSend into buffer \todo: synch
	if (_bufSend == "html") {
		std::cout << toHtml().toLatin1().data() << std::endl;
	}
	_bufSend = "";

	_thrWaitMut->unlock();

	return *str;
}

/*
	Pycli calls this function (through CAPyConsoleInterface) to send some output to console
*/
void CAPyConsole::asyncBufferedOutput(QString bufInp, bool bStdErr) {
		emit sig_txtAppend(bufInp, bStdErr?txtStderr:txtStdout);
}


void CAPyConsole::asyncKeyboardInterrupt() {
Py_BEGIN_ALLOW_THREADS
    _thrIntrWaitMut->lock();
    _thrIntrWait->wait(_thrIntrWaitMut);
    _thrIntrWaitMut->unlock();
Py_END_ALLOW_THREADS
}


// --------------------------------
// -------EVENTS,BEHAVIOUR---------
// --------------------------------

// history, hist* functions take care of _strInput

void CAPyConsole::histAdd() {
	// dont add "" to history
	if(_strInput == "")
		return;

	// dont add item, that's already at the end
	if(!_histList.isEmpty())
		if(_strInput == _histList[0])
			return;

	_histIndex = -1;
	_histList.prepend(_strInput);
}

void CAPyConsole::histGet(HistLay histLay) {
	moveCursor(QTextCursor::End);

	if (_histList.isEmpty())
		return;

	// error check (just for debugging)
	if (_histIndex < -1){
		std::cout << "ERROR!: PyConsole->histGet index below -1" << std::endl;
		_histIndex = -1;

	} else if (_histIndex >= _histList.size()) {
		std::cout << "ERROR!: PyConsole->histGet index above MAX" << std::endl;
		_histIndex = _histList.size();
	}

	if (_histIndex == -1)
		_histOldInput = txtGetInput();
	else
		_histList[_histIndex] = txtGetInput();

	if(histLay == histPrev)
		_histIndex++;
	else    // must be histNext
		_histIndex--;

	if(_histIndex < 0) {
		_histIndex = -1;
		txtSetInput(_histOldInput);
		return;

	} else if (_histIndex >= _histList.size())
		_histIndex = _histList.size() - 1;

	txtSetInput(_histList[_histIndex]);
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
		histAdd();
		_bufSend = txtGetInput();

		_tf = new TxtFragment;
		_tf->text = _strInput + "\n";
		_tf->type = txtNormal;
		_txtFixed += _tf;

		txtAppend("\n");		//update all necessary pointers, and stuff
		if (!cmdIntern(_bufSend))
            _thrWait->wakeOne();

		break;

	case Qt::Key_Up:
		histGet(histPrev);
		break;

	case Qt::Key_Down:
		histGet(histNext);
		break;

	case Qt::Key_Left:
		if(_iCurNow <= _iCurStart) {
			_iCurNowOld = _iCurNow = _iCurStart;
			_curNew = textCursor();
			_curNew.setPosition(_iCurNow);
			setTextCursor(_curNew);
			setCurrentCharFormat(_fmtNormal); }
		else
			defCase = true;
		break;

	case Qt::Key_Home:
		_iCurNowOld = _iCurNow = _iCurStart;
		_curNew = textCursor();
		_curNew.setPosition(_iCurNow);
		setTextCursor(_curNew);
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
// internal commands start with "/"
// none of this works, because of some threading issues

bool CAPyConsole::cmdIntern(QString strCmd) {
    if (!strCmd.startsWith("/"))
        return false;
    
    if (strCmd == "/i") {
        txtAppend("[Can't reset PyCLI]\n");
        /*
            txtAppend("[Resetting PyCLI...]");
            _bufSend = "";

            CASwigPython::terminatePycli();
            _thrIntrWait->wakeOne();

            CAPlugin *plug;
            if(plug = CAPluginManager::getPluginByName("pyCLI")){
                plug->callActionByName("pycli", 0, _canorusDoc);
            }
        */
        return true;
    }
    else if (strCmd.startsWith("/callscript ")) {

        if(!QFile::exists("scripts:" + strCmd.mid(12))) {
            txtAppend("Script not found\n",txtStderr);
            txtAppend(">>> ",txtNormal);
            return true;
        }
        
        QList<PyObject*> argsPython;
        
        QObject *curObject = this;
        while (dynamic_cast<CAMainWin*>(curObject)==0 && curObject!=0) // find the parent which is mainwindow
            curObject = curObject->parent();
        
        argsPython << CASwigPython::toPythonObject(static_cast<CAMainWin*>(curObject)->document(), CASwigPython::Document);        

//		if (strCmd.length() != 12) {		// entryfunc specified explicitly
        CASwigPython::callFunction(QFileInfo("scripts:" + strCmd.mid(12)).absoluteFilePath(), _strEntryFunc, argsPython, true);
        emit sig_txtAppend(">>> ",txtNormal);		// if not emitted, error from python and this are not in order
        return true;
    }
    
    else if (strCmd == "/entryfunc"){
        txtAppend("Default entry function: " + _strEntryFunc + "\n" ,txtStdout);
        txtAppend(">>> ",txtNormal);
    }
    
    else if (strCmd.startsWith("/entryfunc ")) {
        _strEntryFunc = strCmd.mid(11);
        txtAppend(">>> ",txtNormal);
    }

	else {
		txtAppend("Error: no such command\n", txtStderr);
		txtAppend(">>> ", txtNormal);
	}    

    return true;
}
#else

// If there is no python, generate dummy functions
void CAPyConsole::txtAppend(QString const&, CAPyConsole::TxtType) {}
void CAPyConsole::on_txtChanged() {}
void CAPyConsole::on_posChanged() {}
void CAPyConsole::on_selChanged() {}
void CAPyConsole::on_fmtChanged() {}
void CAPyConsole::syncPluginInit() {}
void CAPyConsole::keyPressEvent (QKeyEvent * e) {}
#endif //USE_PYTHON

