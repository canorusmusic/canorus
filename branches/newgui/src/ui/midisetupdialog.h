/** @file ui/midisetupdialog.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef MIDISETUPDIALOG_H_
#define MIDISETUPDIALOG_H_

#include <QWidget>
#include <QPoint>

#include "ui_midisetupdialog.h"

/**
 * A dialog that shows available MIDI IN & OUT devices and ports and
 * allows user to select among them.
 * 
 * This should probably move to General Canorus Preferences some day,
 * but it's not yet implemented in time of writing. -Matevz
 */
class CAMidiSetupDialog : public QDialog {
	Q_OBJECT
	
public:
	CAMidiSetupDialog(QWidget *parent=0);
	~CAMidiSetupDialog();

public slots:
	void accept();
	void reject();
	
private:
	Ui::MidiSetupDialog _dialog;
	QMap<int, QString> _inPorts;
	QMap<int, QString> _outPorts;
};
#endif /*MIDISETUPDIALOG_H_*/
