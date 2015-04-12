/*!
	Copyright (c) 2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef _CASINGLEACTION_H_
#define _CASINGLEACTION_H_

#include <QAction>
#include <QShortcut>

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
    inline QString getCommandName()     { return _oCommandName; }
	inline QString getContext()     { return _oContext; }
	inline QString getDescription() { return _oDescription; }
    inline QString getShortCutAsString() { return _oShortCut; }
    inline QString getMidiKeySequence() { return _oMidiKeySequence; }
    inline bool    getMidiShortCutCombined() { return _bMidiShortCutCombined; }
    // Application-specific Getter (Refs, so no Setter required)
    inline QAction      &getAction() { return (QAction &)*this; }
    inline QKeySequence &getSysShortCut() { return _oSysShortCut; }
    inline QList<int>   &getMidiKeyParameters() { return _oMidiKeyParameters; }
	
	// Setter methods
    void setCommandName( QString oCommandName );
	void setContext( QString oContext );
	void setDescription( QString oDescription );
    void setShortCutAsString( QString oShortCut );
    void setMidiKeySequence( QString oMidiKeySequence, bool combined = false );

protected:
    // Action parameters to be stored / loaded via Settings Dialog
    QString _oCommandName;
	QString _oContext;
	QString _oDescription;
	QString _oShortCut;
    QString _oMidiKeySequence;
    bool    _bMidiShortCutCombined;
    // ShortCut, Midi Key Sequence for the application
    QKeySequence _oSysShortCut;
    QList<int>   _oMidiKeyParameters;
};

#endif // _CASINGLEACTION_H_
