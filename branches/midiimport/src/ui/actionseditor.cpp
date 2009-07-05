/*!
        Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
        All Rights Reserved. See AUTHORS for a complete list of authors.

        Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.

		This code is based on
		smplayer, GUI front-end for mplayer, v0.6.7.
		Copyright (C) 2006-2009 Ricardo Villalba <rvm@escomposlinux.org>
		which again based it on qq14-actioneditor-code.zip from Qt
		and heavily adapted to our needs.
*/


#include "actionseditor.h"

#include <QTableWidget>
#include <QHeaderView>

#include <QLayout>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QSettings>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QRegExp>
#include <QApplication>
#include <QAction>

#include "images.h"
#include "filedialog.h"
#include "paths.h"

// Number of columns used: Conflicts, Command, Context, Shortcut, Midi Command
#define COL_NUM 5

/*
#if USE_MULTIPLE_SHORTCUTS
QString ActionsEditor::shortcutsToString(QList <QKeySequence> shortcuts_list) {
	QString accelText = "";

	for (int n=0; n < shortcuts_list.count(); n++) {
		accelText += shortcuts_list[n].toString(QKeySequence::PortableText);
		if (n < (shortcuts_list.count()-1)) accelText += ", ";
	}

	return accelText;
}

QList <QKeySequence> ActionsEditor::stringToShortcuts(QString shortcuts) {
	QList <QKeySequence> shortcuts_list;

	QStringList l = shortcuts.split(',');

	for (int n=0; n < l.count(); n++) {
		//qDebug("%s", l[n].toUtf8().data());
#if QT_VERSION >= 0x040300
		// Qt 4.3 and 4.4 (at least on linux) seems to have a problem when using Traditional Chinese
		// QKeysequence deletes the arrow key names from the shortcut
		// so this is a work-around.
		QString s = l[n].simplified();
#else
		QString s = QKeySequence( l[n].simplified() );
#endif
		
		//Work-around for Simplified-Chinese
		s.replace( QString::fromUtf8("左"), "Left");
		s.replace( QString::fromUtf8("下"), "Down");
		s.replace( QString::fromUtf8("右"), "Right");
		s.replace( QString::fromUtf8("上"), "Up");

		shortcuts_list.append( s );
		//qDebug("ActionsEditor::stringToShortcuts: shortcut %d: '%s'", n, s.toUtf8().data());
	}

	return shortcuts_list;
}
#endif
*/

// Definition of column positionst
#define COL_CONFLICTS 0 // indicates a conflict with other shortcut
#define COL_COMMAND 1   // name of the command (not internal!)
#define COL_CONTEXT 2   // Context of the command like mode
#define COL_SHORTCUT 3  // Keyboard shortcut
#define COL_MIDI 4      // Midi command

CAActionsEditor::CAActionsEditor(QWidget * parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	// Get directory used last time to adapt default directory to it.
	latest_dir = Paths::shortcutsPath();
    actionsTable = new QTableWidget(0, COL_NUM, this);
	actionsTable->setSelectionMode( QAbstractItemView::SingleSelection );
	actionsTable->verticalHeader()->hide();

	actionsTable->horizontalHeader()->setResizeMode(COL_COMMAND, QHeaderView::Stretch);
	actionsTable->horizontalHeader()->setResizeMode(COL_CONTEXT, QHeaderView::Stretch);

	actionsTable->setAlternatingRowColors(true);
//#if USE_SHORTCUTGETTER
//	actionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
//	actionsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
//#endif
	//actionsTable->setItemDelegateForColumn( COL_SHORTCUT, new MyDelegate(actionsTable) );

//#if !USE_SHORTCUTGETTER // In place editing of shortcut ??
	connect(actionsTable, SIGNAL(currentItemChanged(QTableWidgetItem *,QTableWidgetItem *)),
            this, SLOT(recordAction(QTableWidgetItem *)) );
	connect(actionsTable, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(validateAction(QTableWidgetItem *)) );
//#else
//	connect(actionsTable, SIGNAL(itemActivated(QTableWidgetItem *)),
//            this, SLOT(editShortcut()) );
//#endif

	saveButton = new QPushButton(this);
	loadButton = new QPushButton(this);

	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveActionsTable()));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadActionsTable()));

//#if USE_SHORTCUTGETTER
//	editButton = new QPushButton(this);
//	connect( editButton, SIGNAL(clicked()), this, SLOT(editShortcut()) );
//#endif

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
//#if USE_SHORTCUTGETTER
//	buttonLayout->addWidget(editButton);
//#endif
    buttonLayout->addStretch(1);
	buttonLayout->addWidget(loadButton);
	buttonLayout->addWidget(saveButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setMargin(8);
    mainLayout->setSpacing(8);
    mainLayout->addWidget(actionsTable);
    mainLayout->addLayout(buttonLayout);

	retranslateStrings();
}

CAActionsEditor::~CAActionsEditor() {
}

void CAActionsEditor::retranslateStrings() {
	actionsTable->setHorizontalHeaderLabels( QStringList() << "" <<
		tr("Shortcut") << tr("Description") << tr("Name") );

	saveButton->setIcon(Images::icon("save"));
	loadButton->setIcon(Images::icon("open"));

	if( actionsTable->column( (QTableWidgetItem *)focusWidget() ) == COL_SHORTCUT )
	{
		saveButton->setText(tr("&Save shortcuts"));
		loadButton->setText(tr("&Load shortcuts"));
	}
	else
	{
		saveButton->setText(tr("&Save midi commands"));
		loadButton->setText(tr("&Load midi commands"));
	}

//#if USE_SHORTCUTGETTER
//	editButton->setText(tr("&Change shortcut..."));
//#endif

	//updateView(); // The actions are translated later, so it's useless
}

bool CAActionsEditor::isEmpty() {
	return actionsList.isEmpty();
}

void CAActionsEditor::clear() {
	actionsList.clear();
}

void CAActionsEditor::addActions(QWidget *widget) {
	QAction *action;

	QList<QAction *> actions = widget->findChildren<QAction *>();
	for (int n=0; n < actions.count(); n++) {
		action = static_cast<QAction*> (actions[n]);
		if (!action->objectName().isEmpty() && !action->inherits("QWidgetAction"))
	        actionsList.append(action);
    }

	updateView();
}

void CAActionsEditor::updateView() {
	actionsTable->setRowCount( actionsList.count() );

    QAction *action;
	QString accelText, midi_com, context;

//#if !USE_SHORTCUTGETTER
	dont_validate = true;
//#endif
	//actionsTable->setSortingEnabled(false);

// @ToDo: Replace with our own list of Canorus actions
	for (int n=0; n < actionsList.count(); n++) {
		action = static_cast<QAction*> (actionsList[n]);

//#if USE_MULTIPLE_SHORTCUTS
//		accelText = shortcutsToString( action->shortcuts() );
//#else
		accelText = action->property( CACanorus::propShortCut() ).toString();
//#endif
		context  = action->property( CACanorus::propContext() ).toString();
		midi_com = action->property( CACanorus::propMidiCommand() ).toString();
		
		QTableWidgetItem * i_conf = new QTableWidgetItem();

		// Command column
		QTableWidgetItem * i_command = new QTableWidgetItem(action->text());

		// Context column
		QTableWidgetItem * i_context = new QTableWidgetItem( context );

		// Shortcut column
		QTableWidgetItem * i_shortcut = new QTableWidgetItem(accelText);

		// Midi command
		QTableWidgetItem * i_midi = new QTableWidgetItem(midi_com);
		
		// Set flags
//#if !USE_SHORTCUTGETTER
//		i_conf->setFlags(Qt::ItemIsEnabled);
//		i_name->setFlags(Qt::ItemIsEnabled);
//		i_desc->setFlags(Qt::ItemIsEnabled);
//#else
		// Show specific command description
		i_conf->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_conf->setToolTip( action->property( CACanorus::propDescription ).toString() );
		i_name->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_name->setToolTip( action->property( CACanorus::propDescription ).toString() );
		i_desc->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_desc->setToolTip( action->property( CACanorus::propDescription ).toString() );
		i_shortcut->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_shortcut->setToolTip( action->property( CACanorus::propDescription ).toString() );
		i_midi>setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_midi->setToolTip( action->property( CACanorus::propDescription ).toString() );
//#endif

		// Add items to table
		actionsTable->setItem(n, COL_CONFLICTS, i_conf );
		actionsTable->setItem(n, COL_COMMAND, i_name );
		actionsTable->setItem(n, COL_CONTEXT, i_context );
		actionsTable->setItem(n, COL_SHORTCUT, i_shortcut );
		actionsTable->setItem(n, COL_MIDI, i_midi );

	}
	hasConflicts(); // Check for conflicts

	actionsTable->resizeColumnsToContents();
	// @ToDo: Set to last edited cell type (Midi or Shortcut ?)
	actionsTable->setCurrentCell(0, COL_SHORTCUT);

//#if !USE_SHORTCUTGETTER
	dont_validate = false;
//#endif
	//actionsTable->setSortingEnabled(true);
}


void CAActionsEditor::applyChanges() {
	qDebug("CAActionsEditor::applyChanges");

	for (int row = 0; row < (int)actionsList.size(); ++row) {
		QAction *action = actionsList[row];
		QTableWidgetItem *i = actionsTable->item(row, COL_SHORTCUT);

//#if USE_MULTIPLE_SHORTCUTS
//		action->setShortcuts( stringToShortcuts(i->text()) );
//#else
		// @ToDo Update Midi/Shortcut corresponding but
		// Update our own list of settings
		action->setProperty( CACanorus::propShortCut(), QKeySequence(i->text()) );
		i = actionsTable->item(row, COL_MIDI);
		action->setProperty( CACanorus::propMidiCommand(), i->text() );
//#endif

	}
}

//#if !USE_SHORTCUTGETTER
void CAActionsEditor::recordAction(QTableWidgetItem * i) {
	//qDebug("CAActionsEditor::recordAction");

	int iCol = i->column();
	//QTableWidgetItem * i = actionsTable->currentItem();
	if ( iCol == COL_SHORTCUT ) {
		//qDebug("CAActionsEditor::recordAction: %d %d %s", i->row(), i->column(), i->text().toUtf8().data());
		oldAccelText = i->text();
	}
	else if ( iCol == COL_MIDI )
		oldMidiText = i->text();
}

void CAActionsEditor::validateAction(QTableWidgetItem * i) {
	//qDebug("CAActionsEditor::validateAction");
	if (dont_validate) return;

	int iCol = i->column();
	if (iCol == COL_SHORTCUT) {
	    QString accelText = QKeySequence(i->text()).toString();

	    if (accelText.isEmpty() && !i->text().isEmpty()) {
			/*
			QAction * action = static_cast<QAction*> (actionsList[i->row()]);
			QString oldAccelText= action->accel().toString();
			*/
	        i->setText(oldAccelText);
		}
	    else {
	        i->setText(accelText);
		}
		// @ToDo: Only optional beep ?
		if (hasConflicts()) qApp->beep();
	}
	else if (iCol = COL_MIDI)
	{
	    QString midiText = i->text();

	    if (midiText.isEmpty() && !i->text().isEmpty()) {
			/*
			QAction * action = static_cast<QAction*> (actionsList[i->row()]);
			QString oldAccelText= action->accel().toString();
			*/
	        i->setText(oldMidiText);
		}
	    else {
	        i->setText(midiText);
		}
		// @ToDo: Only optional beep ?
		if (hasConflicts()) qApp->beep();
	}
}

/* #else

void CAActionsEditor::editShortcut() {
	QTableWidgetItem * i = actionsTable->item( actionsTable->currentRow(), COL_SHORTCUT );
	if (i) {
		ShortcutGetter d(this);
		QString result = d.exec( i->text() );

		if (!result.isNull()) {
		    QString accelText = QKeySequence(result).toString(QKeySequence::PortableText);
			i->setText(accelText);
			if (hasConflicts()) qApp->beep();
		}
	}
}
#endif */

int CAActionsEditor::findActionCommand(const QString & name) {
	for (int row=0; row < actionsTable->rowCount(); row++) {
		if (actionsTable->item(row, COL_COMMAND)->text() == name) return row;
	}
	return -1;
}

int CAActionsEditor::findActionAccel(const QString & accel, int ignoreRow) {
	for (int row=0; row < actionsTable->rowCount(); row++) {
		QTableWidgetItem * i = actionsTable->item(row, COL_SHORTCUT);
		if ( (i) && (i->text() == accel) ) {
			if (ignoreRow == -1) return row;
			else
			if (ignoreRow != row) return row;
		}
	}
	return -1;
}

int CAActionsEditor::findActionMidi(const QString & midi, int ignoreRow) {
	for (int row=0; row < actionsTable->rowCount(); row++) {
		QTableWidgetItem * i = actionsTable->item(row, COL_MIDI);
		if ( (i) && (i->text() == accel) ) {
			if (ignoreRow == -1) return row;
			else
			if (ignoreRow != row) return row;
		}
	}
	return -1;
}

bool CAActionsEditor::hasConflicts(bool bMidi) {
	// @ToDo: Consider if we should have two different conflict columns
	int found, iType = bMidi ? COL_MIDI : COL_SHORTCUT;
	bool conflict = false;

	QString accelText;
	QTableWidgetItem *i;

	for (int n=0; n < actionsTable->rowCount(); n++) {
		//actionsTable->setText( n, COL_CONFLICTS, " ");
		i = actionsTable->item( n, COL_CONFLICTS );
		// Think positive, no conflict when we start to search
		if (i) i->setIcon( QPnixmap() );
		// @ToDo: remove hardcoded names for icon and conflict types
		actionsList[row]->setProperty( CACanorus::propConflicts(), "none" );

		i = actionsTable->item(n, iType );
		if (i) {
			accelText = i->text();
			if (!accelText.isEmpty()) {
				found = bMidi ? findActionMidi( accelText, n ) :
				                findActionAccel( accelText, n );
				if ( (found != -1) && (found != n) ) {
					conflict = true;
					//actionsTable->setText( n, COL_CONFLICTS, "!");
					actionsTable->item( n, COL_CONFLICTS )->setIcon( Images::icon("conflict") );
					if( bMidi ) // type of conflict
						actionsList[row]->setProperty( CACanorus::propConflicts(), "midi" );
					else
						actionsList[row]->setProperty( CACanorus::propConflicts(), "shortcut" );
				}
			}
		}
	}
	//if (conflict) qApp->beep();
	return conflict;
}

void CAActionsEditor::saveActionsTable() {
	QString sk, end;
	if( actionsTable->column( (QTableWidgetItem *)focusWidget() ) == COL_SHORTCUT )
	{
		sk = tr("Shortcut files") +" (*.cakey)" );
		end = ".cakey"
	}
	else
	{
		sk = tr("Midi command files") +" (*.camid)" );
		end = ".camid"
	}
	QString s = MyFileDialog::getSaveFileName(
                    this, tr("Choose a filename"), 
                    latest_dir, sk;

	if (!s.isEmpty()) {
		// If filename has no extension, add it
		if (QFileInfo(s).suffix().isEmpty()) {
			s = s + end;
		}
		if (QFileInfo(s).exists()) {
			int res = QMessageBox::question( this,
					tr("Confirm overwrite?"),
                    tr("The file %1 already exists.\n"
                       "Do you want to overwrite?").arg(s),
                    QMessageBox::Yes,
                    QMessageBox::No,
                    Qt::NoButton);
			if (res == QMessageBox::No ) {
            	return;
			}
		}
		latest_dir = QFileInfo(s).absolutePath();
		bool r = saveActionsTable(s);
		if (!r) {
			QMessageBox::warning(this, tr("Error"), 
               tr("The file couldn't be saved"), 
               QMessageBox::Ok, Qt::NoButton);
		}
	} 
}

bool CAActionsEditor::saveActionsTable(const QString & filename) {
	qDebug("CAActionsEditor::saveActions: '%s'", filename.toUtf8().data());

	QFile f( filename );
	if ( f.open( QIODevice::WriteOnly ) ) {
		QTextStream stream( &f );
		stream.setCodec("UTF-8");

		for (int row=0; row < actionsTable->rowCount(); row++) {
			stream << actionsTable->item(row, COL_COMMAND)->text() << "\t" 
                   << actionsTable->item(row, COL_SHORTCUT)->text() << "\n";
				   << actionsTable->item(row, COL_MIDI)->text() << "\n";
		}
		f.close();
		return true;
	}
	return false;
}

void CAActionsEditor::loadActionsTable() {
	QString sk;
	if( actionsTable->column( (QTableWidgetItem *)focusWidget() ) == COL_SHORTCUT )
		sk = tr("Shortcut files") +" (*.cakey)" );
	else
		sk = tr("Midi command files") +" (*.camid)" );
	QString s = MyFileDialog::getOpenFileName(
                    this, tr("Choose a file"),
                    latest_dir, sk );

	if (!s.isEmpty()) {
		latest_dir = QFileInfo(s).absolutePath();
		bool r = loadActionsTable(s);
		if (!r) {
			QMessageBox::warning(this, tr("Error"), 
               tr("The file couldn't be loaded"), 
               QMessageBox::Ok, Qt::NoButton);
		}
	}
}

bool CAActionsEditor::loadActionsTable(const QString & filename) {
	qDebug("CAActionsEditor::loadActions: '%s'", filename.toUtf8().data());

	QRegExp rx("^(.*)\\t|\\s*(.*)\\t|\\s*(.*)\\t|\\s*(.*)\\t|\\s*(.*)\\t|\\s*(.*)");
	int row;

    QFile f( filename );
    if ( f.open( QIODevice::ReadOnly ) ) {

//#if !USE_SHORTCUTGETTER
		dont_validate = true;
//#endif

        QTextStream stream( &f );
		stream.setCodec("UTF-8");

        QString line;
        while ( !stream.atEnd() ) {
            line = stream.readLine();
			qDebug("line: '%s'", line.toUtf8().data());
			if (rx.indexIn(line) > -1) {
				QString command = rx.cap(1);
				QString accelText = rx.cap(2);
				QString midiText = rx.cap(3) + " " + rx.cap(4) + " " + rx.cap(5)
				qDebug(" command: '%s' context: '%s' accel: '%s' midi: '%s'",
				 command.toUtf8().data(), accelText.toUtf8().data(), midiText.toUtf8().data());
				// @ToDo: command name is not identical to command identifier!
				row = findActionName(command);
				if (row > -1) {
					qDebug("Action found!");
					actionsTable->item(row, COL_SHORTCUT)->setText(accelText);
				}				
			} else {
				qDebug(" wrong line");
			}
		}
		f.close();
		hasConflicts(); // Check for conflicts

//#if !USE_SHORTCUTGETTER
		dont_validate = false;
//#endif

		return true;
	} else {
		return false;
	}
}


// Static functions

void CAActionsEditor::saveToConfig(QObject *o, QSettings *set) {
	qDebug("ActionsEditor::saveToConfig");

	set->beginGroup("actions");

	QAction *action;
	QList<QAction *> actions = o->findChildren<QAction *>();
	for (int n=0; n < actions.count(); n++) {
		action = static_cast<QAction*> (actions[/n]);
		if (!action->text().isEmpty() && !action->inherits("QWidgetAction")) {
//#if USE_MULTIPLE_SHORTCUTS
//			QString accelText = shortcutsToString(action->shortcuts());
//#else
			QString accelText = action->property( CACanorus::propShortCut() ).toString();
//#endif
			set->setValue(action->text(), accelText);
		}
    }

	set->endGroup();
}


void CAActionsEditor::loadFromConfig(QObject *o, QSettings *set) {
	qDebug("ActionsEditor::loadFromConfig");

	set->beginGroup("actions");

	QAction *action;
	QString accelText;

	QList<QAction *> actions = o->findChildren<QAction *>();
	for (int n=0; n < actions.count(); n++) {
		action = static_cast<QAction*> (actions[n]);
		if (!action->objectName().isEmpty() && !action->inherits("QWidgetAction")) {
//#if USE_MULTIPLE_SHORTCUTS
//			QString current = shortcutsToString(action->shortcuts());
//			accelText = set->value(action->objectName(), current).toString();
//			action->setShortcuts( stringToShortcuts( accelText ) );
//#else
			accelText = set->value(action->text(), action->getShortCut().toString()).toString();
			action->setProperty( CACanorus::propShortCut(), QKeySequence(accelText));
//#endif
		}
    }

	set->endGroup();
}

QAction * CAActionsEditor::findAction(QObject *o, const QString & name) {
	QAction *action;

	QList<QAction *> actions = o->findChildren<QAction *>();
	for (int n=0; n < actions.count(); n++) {
		action = static_cast<QAction*> (actions[n]);
		if (name == action->objectName()) return action;
    }

	return 0;
}

QStringList CAActionsEditor::actionsNames(QObject *o) {
	QStringList l;

	QAction *action;

	QList<QAction *> actions = o->findChildren<QAction *>();
	for (int n=0; n < actions.count(); n++) {
		action = static_cast<QAction*> (actions[n]);
		//qDebug("action name: '%s'", action->objectName().toUtf8().data());
		//qDebug("action name: '%s'", action->text().toUtf8().data());
		if (!action->text().isEmpty())
			l.append( action->text() );
    }

	return l;
}


// Language change stuff
void CAActionsEditor::changeEvent(QEvent *e) {
	if (e->type() == QEvent::LanguageChange) {
		retranslateStrings();
	} else {
		QWidget::changeEvent(e);
	}
}

#include "moc_actionseditor.cpp"
