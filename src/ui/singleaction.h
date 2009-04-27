/*!
	Copyright (c) 2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef _CASINGLEACTION_H_
#define _CASINGLEACTION_H_

#include <QAction>

class QString;

// One action based on QAction, Midi and QString. Contains all
// information (including description) for one control command
class CASingleAction : public QAction
{
    Q_OBJECT

public:
	// Constructor / Desctructor
	CASingleAction( QObject * );
	~CASingleAction();

	// Getter methods for all single action parameters
	inline QString getCommand()     { return _oCommand; }
	inline QString getContext()     { return _oContext; }
	inline QString getDescription() { return _oDescription; }
	inline QString getShortCut()    { return _oShortCut; }
	inline QString getMidiCommand() { return _oMidiCommand; }
	
	// Setter methods
	void setCommand( QString oCommand );
	void setContext( QString oContext );
	void setDescription( QString oDescription );
	void setShortCut( QString oShortCut );
	void setMidiCommand( QString oMidiCommand );

protected:
	QString _oCommand;
	QString _oContext;
	QString _oDescription;
	QString _oShortCut;
	QString _oMidiCommand;
};

#endif // _CASINGLEACTION_H_
