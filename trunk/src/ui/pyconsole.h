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
#include <QToolBar>		// parent->show???
#include "core/document.h"

class CAPyConsole : public QTextEdit {
	Q_OBJECT
public:
	CAPyConsole ( CADocument *doc, QWidget *parent=0);

	QString buffered_input(QString prompt);
	void buffered_output(QString bufInp, bool bStdErr);
	void plugin_init(void);

protected:
	void keyPressEvent (QKeyEvent * e);

private slots:
	void selChanged();
	void txtChanged();
	void curPosChanged();
	void insertTextAtEnd(const QString & text);
	void safePluginInit();

signals:
	void thr_insertTextAtEnd(const QString & text);
	void thr_pluginInit();

private:
	QWidget* _parent;
	// text in the console
	QTextCursor *consoleCursor;	// to select input
	int _iCurStart, _iCurNowOld, _iCurNow;	// positions
	QString qStrFixed;
	QString qStrInput;

	CADocument *_document;
	bool bNoTxtChange;

	QString bufSend;	// \todo: synch
	bool bCmdProcess;	//python received command, bud yet didn't respond

};

#endif /* PYCONSOLE_H_ */
