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
	CAPyConsole ( CADocument *doc, QWidget *parent=0);

	QString buffered_input(QString prompt);
	void buffered_output(QString bufInp, bool bStderr);
	void plugin_init(void);
	void keyboard_interrupt(void);

	enum TxtType {
		txtNormal,
		txtStdout,
		txtStderr
	};

protected:
	void keyPressEvent (QKeyEvent * e);

private slots:
	void insertTextAtEnd(const QString & text, TxtType txtType = txtNormal );
	void txtChanged();
	void posChanged();
	void selChanged();
	void fmtChanged();
	void safePluginInit();

signals:
	void thr_insertTextAtEnd(const QString & text, TxtType stdType);
	void thr_pluginInit();

private:
	void txtRevert();

	CADocument *canorusDoc;		// not used (yet?)
	bool bNoTxtChange;

	struct TxtFragment {
		QString text;
		TxtType type;
	};

	QWidget* _parent;

	// text in the console
	QTextCursor *consoleCursor;
	int _iCurStart, _iCurNowOld, _iCurNow;
	QList<TxtFragment*> txtFixed;
	QString strInput;

	TxtFragment *tf;
	QTextCursor newCur;

	// history
	void histAdd();
	void histGet(bool prev);

	QList<QString> histList;
	int histIndex;
	QString histOldInput;	// old strInput
	
	QTextCharFormat _fmtNormal;
	QTextCharFormat _fmtStdout;
	QTextCharFormat _fmtStderr;

	// thread
	QString bufSend;
	QMutex *thrWaitMut;
	QWaitCondition *thrWait;
};

#endif /* PYCONSOLE_H_ */
