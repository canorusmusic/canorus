/*!
        Copyright (c) 2009, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.

		This code is based on
		smplayer, GUI front-end for mplayer, v0.6.7.
		Copyright (C) 2006-2009 Ricardo Villalba <rvm@escomposlinux.org>
		which again based it on qq14-actioneditor-code.zip from Qt
		and heavily adapted to our needs.
*/

#ifndef _CAACTIONSEDITOR_H_
#define _CAACTIONSEDITOR_H_

#include <QDialog>
#include <QList>
#include <QStringList>
#include "ui/singleaction.h"

class QTableWidget;
class QTableWidgetItem;
class CASingleAction;
class QSettings;
class QLineEdit;
class QPushButton;

class CAActionsEditor : public QWidget
{
    Q_OBJECT

public:

    // Definition of file type
    enum fileType
    {
    //FT_COMPLETE     = 0,       // All together like when stored in settings
      FT_SHORTCUT     = 1,       // Keyboard shortcut
      FT_MIDI         = 2,       // Midi command
      FT_MIDISCUT     = 3        // Requires Midi and Shortcut at one time to be used
    };

	// Constructor
	// Parameters are standard QWidget parameters
	CAActionsEditor( QWidget * parent = nullptr, Qt::WindowFlags f = nullptr );
	// Destructor
	~CAActionsEditor();

	// Clear the actionlist
	void clear();

	// There are no actions yet?
	bool isEmpty();

	// See QWidget documentation
    void addActions(const QList<CASingleAction *> &actionList);

	// Static functions
    static CASingleAction * findAction(QWidget *widget, const QString & name);
    static QStringList actionsNames(QWidget *widget);

    static void saveToConfig(QWidget *widget, QSettings *set);
    static void loadFromConfig(QWidget *widget, QSettings *set);

//#if USE_MULTIPLE_SHORTCUTS
//	static QString shortcutsToString(QList <QKeySequence> shortcuts_list);
//	static QList <QKeySequence> stringToShortcuts(QString shortcuts);
//#endif

public slots:
	// save changes from editing (shortcut or midi command)
	void applyChanges();
    // Get file type from suffix
    enum fileType getFType(const QString &suffix);
    // save action profile (shortcuts or midi commands) separately to disc
	void saveActionsTable();
    bool saveActionsTable(const QString & filename, enum fileType type = FT_SHORTCUT);
	// load action profile (shortcuts or midi commands) separately from disc
	void loadActionsTable();
    bool loadActionsTable(const QString & filename, enum fileType type = FT_SHORTCUT);

	// Add all actions to the table widget
	void updateView();

protected:
	// Translate strings dynamically
	// Adapts save buttons according to column focus
	virtual void retranslateStrings();
	virtual void changeEvent ( QEvent * event ) ;

	// Find in table, not in actionslist (command, shortcut or midi)
	int findActionCommand(const QString & name);
	int findActionAccel(const QString & accel, int ignoreRow = -1);
	int findActionMidi(const QString & midi, int ignoreRow = -1);
	// Check for Conflicts of shortcut or midi command
	bool hasConflicts(bool bMidi = false);

protected slots:
//#if !USE_SHORTCUTGETTER
	// Start recording (save current Text)
	void recordAction(QTableWidgetItem*);
	// Easy check of edited shortcut / midi command
	void validateAction(QTableWidgetItem*);
//#else
    void editShortcut();
//#endif

private:
	QTableWidget *actionsTable;
    QList<CASingleAction *> m_actionsList;
	QPushButton *saveButton;
	QPushButton *loadButton;
	QString latest_dir;

//#if USE_SHORTCUTGETTER
    QPushButton *editButton;
//#else
	QString oldAccelText;
	QString oldMidiText;
	bool dont_validate;   // Lock validate during update
//#endif
};

class ShortcutGetter : public QDialog
{
      Q_OBJECT

public:
      ShortcutGetter(QWidget *parent = nullptr);

      int exec() override { return QDialog::exec(); }
      QString exec(const QString& s);

protected slots:
      void setCaptureKeyboard(bool b);

protected:
      bool captureKeyboard() { return capture; }

      bool event(QEvent *e) override;
      bool eventFilter(QObject *o, QEvent *e) override;
      void setText();

private:
      bool bStop;
      QLineEdit *leKey;
      QStringList lKeys;
      bool capture;
};


#endif
