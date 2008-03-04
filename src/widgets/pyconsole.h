/*!
	Copyright (c) 2006-2008, Štefan Sakalík, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PYCONSOLE_H_
#define PYCONSOLE_H_

#include <QObject>
#include <QTextEdit>
#include <QMutex>
#include <QWaitCondition>
#include <QToolBar>
#include "core/document.h"

class CAPyConsole : public QTextEdit {
	Q_OBJECT
public:
    enum TxtType {
		txtNormal,
		txtStdout,
		txtStderr
	};

	CAPyConsole ( CADocument *doc, QWidget *parent=0);

	QString asyncBufferedInput(QString prompt);
	void asyncBufferedOutput(QString bufInp, bool bStdErr);
	void asyncPluginInit();
	void asyncKeyboardInterrupt();

protected:
	void keyPressEvent (QKeyEvent * e);

private slots:
	void txtAppend(const QString & text, TxtType txtType = txtNormal );
	void on_txtChanged();
	void on_posChanged();
	void on_selChanged();
	void on_fmtChanged();
	void syncPluginInit();

signals:
	void sig_txtAppend(const QString & text, TxtType stdType);
	void sig_syncPluginInit();

private:
    enum HistLay {
        histPrev,
        histNext
    };

    struct TxtFragment {
		QString text;
		TxtType type;
	};

	// void txtAppend(...) -> is in signals
	void txtRevert();
	QString txtGetInput(bool bReadText = false);
    void txtSetInput(QString input, bool bUpdateText = true);

	void histAdd();
    void histGet(HistLay histLay);

	// text in the console
	QTextCursor _curInput, _curNew;
	int _iCurStart, _iCurNowOld, _iCurNow;
    bool _bIgnTxtChange;

	TxtFragment *_tf;
	QList<TxtFragment*> _txtFixed;
	QString _strInput;

	// history
    int _histIndex;
	QList<QString> _histList;
	QString _histOldInput;	// old _strInput

	QTextCharFormat _fmtNormal;
	QTextCharFormat _fmtStdout;
	QTextCharFormat _fmtStderr;

	// thread
	QString _bufSend;
	QMutex *_thrWaitMut;
	QWaitCondition *_thrWait;
	QMutex *_thrIntrWaitMut;
	QWaitCondition *_thrIntrWait;

	// pyconsole '/' commands
	bool cmdIntern(QString strCmd);
	QString _strEntryFunc;

    // rarely used
	CADocument *_canorusDoc;
	QWidget* _parent;
};

#endif /* PYCONSOLE_H_ */
