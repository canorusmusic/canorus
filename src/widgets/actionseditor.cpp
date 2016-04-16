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
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QFileInfo>
#include <QRegExp>
#include <QApplication>
#include <QAction>
#include <QLineEdit>
#include <QKeyEvent>
#include <QDebug>

//#include "images.h"
#include "canorus.h"
#include "core/settings.h"

// Number of columns used: Command, Context, Shortcut, Midi Command, Combined
#define COL_NUM 5

// Definition of column positions
enum actionCol
{
  COL_COMMAND      = 0,   // name of the command (not internal!)
  COL_DESCRIPTION  = 1,   // Context of the command like mode
  COL_SHORTCUT     = 2,   // Keyboard shortcut
  COL_MIDI         = 3,   // Midi command
  COL_MIDISCUT     = 4    // Requires Midi and Shortcut at one time to be used
};

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

CAActionsEditor::CAActionsEditor(QWidget * parent, Qt::WindowFlags f)
	: QWidget(parent, f)
{
	// Get directory used last time to adapt default directory to it.
	latest_dir = CACanorus::settings()->latestShortcutsDirectory().dirName();
    actionsTable = new QTableWidget(0, COL_NUM, this);
	actionsTable->setSelectionMode( QAbstractItemView::SingleSelection );
    actionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	actionsTable->verticalHeader()->hide();

#if QT_VERSION >= 0x050000
	actionsTable->horizontalHeader()->setSectionResizeMode(COL_COMMAND, QHeaderView::Stretch);
    actionsTable->horizontalHeader()->setSectionResizeMode(COL_DESCRIPTION, QHeaderView::Stretch);
#else
	actionsTable->horizontalHeader()->setResizeMode(COL_COMMAND, QHeaderView::Stretch);
    actionsTable->horizontalHeader()->setResizeMode(COL_DESCRIPTION, QHeaderView::Stretch);
#endif
	
	actionsTable->setAlternatingRowColors(true);
//#if USE_SHORTCUTGETTER
//  actionsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    actionsTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
//#endif
	//actionsTable->setItemDelegateForColumn( COL_SHORTCUT, new MyDelegate(actionsTable) );

//#if !USE_SHORTCUTGETTER // In place editing of shortcut ??
	connect(actionsTable, SIGNAL(currentItemChanged(QTableWidgetItem *,QTableWidgetItem *)),
            this, SLOT(recordAction(QTableWidgetItem *)) );
	connect(actionsTable, SIGNAL(itemChanged(QTableWidgetItem *)),
            this, SLOT(validateAction(QTableWidgetItem *)) );
//#else
    connect(actionsTable, SIGNAL(itemActivated(QTableWidgetItem *)),
            this, SLOT(editShortcut()) );
//#endif

    saveButton = new QPushButton(this);
    loadButton = new QPushButton(this);

	connect(saveButton, SIGNAL(clicked()), this, SLOT(saveActionsTable()));
	connect(loadButton, SIGNAL(clicked()), this, SLOT(loadActionsTable()));

//#if USE_SHORTCUTGETTER
    editButton = new QPushButton(this);
    connect( editButton, SIGNAL(clicked()), this, SLOT(editShortcut()) );
//#endif

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(8);
//#if USE_SHORTCUTGETTER
    buttonLayout->addWidget(editButton);
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
    actionsTable->setHorizontalHeaderLabels( QStringList() <<
        tr("Name") << tr("Description") << tr("Shortcut") << tr("Midi Command") << tr("Combined") );

    //saveButton->setIcon(Images::icon("save"));
	//loadButton->setIcon(Images::icon("open"));

    saveButton->setText(tr("&Save shortcuts..."));
    loadButton->setText(tr("&Load shortcuts..."));

//#if USE_SHORTCUTGETTER
    editButton->setText(tr("&Change shortcut..."));
//#endif

	//updateView(); // The actions are translated later, so it's useless
}

bool CAActionsEditor::isEmpty() {
    return m_actionsList.isEmpty();
}

void CAActionsEditor::clear() {
    m_actionsList.clear();
}

void CAActionsEditor::addActions(const QList<CASingleAction *> &actionList) {
	CASingleAction *action;

    // Issue: Actions not associated to objects anymore due to
    // Step that converts QAction -> CASingleAction
    //QList<CASingleAction *> actions = widget->findChildren<CASingleAction *>();
    //qWarning() << "CAActionsEditor::addActions - size " << actions.size() << " orig size " << widget->actions().size() << endl;
    //QList<QAction *> actions = widget->actions();
    for (int n=0; n < actionList.size(); n++) {
        action = actionList[n];
        //action->getAction()->setParent(this);
        qWarning() << "CAActionsEditor::addActions - objectName " << action->getAction()->objectName() << " inherits " << action->getAction()->inherits("QWidgetAction") << endl;
        if (!action->getAction()->objectName().isEmpty() && !action->getAction()->inherits("QWidgetAction"))
            m_actionsList.append(action);
    }

	updateView();
}

void CAActionsEditor::updateView() {
    actionsTable->setRowCount( m_actionsList.count() );

    CASingleAction *action;
    QString accelText, midi_com, midi_scut, description;

//#if !USE_SHORTCUTGETTER
//	dont_validate = true;
//#endif
	//actionsTable->setSortingEnabled(false);

// @ToDo: Replace with our own list of Canorus actions
    for (int n=0; n < m_actionsList.count(); n++) {
        action = m_actionsList[n];

//#if USE_MULTIPLE_SHORTCUTS
//		accelText = shortcutsToString( action->shortcuts() );
//#else
		accelText = action->getShortCutAsString();
//#endif
        description  = action->getDescription();
        midi_com = action->getMidiKeySequence();
        midi_scut = action->getMidiShortCutCombined();
		
		QTableWidgetItem * i_conf = new QTableWidgetItem();

		// Command column
        QTableWidgetItem * i_command = new QTableWidgetItem(action->getCommandName(true));

		// Context column
        QTableWidgetItem * i_context = new QTableWidgetItem( description );

		// Shortcut column
		QTableWidgetItem * i_shortcut = new QTableWidgetItem(accelText);

		// Midi command
		QTableWidgetItem * i_midi = new QTableWidgetItem(midi_com);

        // Midi command
        QTableWidgetItem * i_midiscut = new QTableWidgetItem(midi_scut);

		// Set flags
//#if !USE_SHORTCUTGETTER
//		i_conf->setFlags(Qt::ItemIsEnabled);
//		i_command->setFlags(Qt::ItemIsEnabled);
//		i_context->setFlags(Qt::ItemIsEnabled);
//#else
		i_conf->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_command->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
		i_context->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        i_shortcut->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEnabled);
		i_midi->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        i_midiscut->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
//#endif

		// Add items to table
        actionsTable->setItem(n, COL_COMMAND, i_command );
        actionsTable->setItem(n, COL_DESCRIPTION, i_context );
        actionsTable->setItem(n, COL_SHORTCUT, i_shortcut );
        actionsTable->setItem(n, COL_MIDI, i_midi );
        actionsTable->setItem(n, COL_MIDISCUT, i_midiscut );

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

    for (int row = 0; row < (int)m_actionsList.size(); ++row) {
        CASingleAction *action = m_actionsList[row];
		QTableWidgetItem *i = actionsTable->item(row, COL_SHORTCUT);

//#if USE_MULTIPLE_SHORTCUTS
//		action->setShortcuts( stringToShortcuts(i->text()) );
//#else
		// @ToDo Update Midi/Shortcut corresponding but
		// Update our own list of settings
		action->setShortCutAsString( QKeySequence(i->text()).toString() );
//#endif
    }
}

CAActionsEditor::fileType CAActionsEditor::getFType(const QString &suffix)
{
    CAActionsEditor::fileType type;
    if( suffix == "cakey" )
        type = FT_SHORTCUT;
    if( suffix == "cakmid" )
        type = FT_MIDI;
    if( suffix == "cacks" )
        type = FT_MIDISCUT;
    return type;
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
	else if (iCol == COL_MIDI)
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
		if ( (i) && (i->text() == midi) ) {
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
		//i = actionsTable->item( n, COL_CONFLICTS );
        i = actionsTable->item(n, iType );
        if (i) i->setIcon( QPixmap() );

		if (i) {
			accelText = i->text();
			if (!accelText.isEmpty()) {
				found = bMidi ? findActionMidi( accelText, n ) :
				                findActionAccel( accelText, n );
				if ( (found != -1) && (found != n) ) {
					conflict = true;
				}
			}
		}
	}
	//if (conflict) qApp->beep();
	return conflict;
}

void CAActionsEditor::saveActionsTable() {
    QString sk;
    sk = tr("Shortcut files") +" (*.cakey);;" + tr("Midi Key Sequence files") + " (*.cakmid);;" + tr("Combined Key Sequence files") + "(*.cacks)";

	QString s = QFileDialog::getSaveFileName(
                    this, tr("Choose a filename"), 
                    latest_dir, sk );

    QFileInfo shortCutFileInfo(s);

	if (!s.isEmpty()) {
		// If filename has no extension, add it
        /*if (QFileInfo(s).completeSuffix().isEmpty()) {
			s = s + end;
        }*/
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

        enum fileType type = getFType( shortCutFileInfo.completeSuffix() );

        bool r = saveActionsTable(s, type);
		if (!r) {
			QMessageBox::warning(this, tr("Error"), 
               tr("The file couldn't be saved"), 
               QMessageBox::Ok, Qt::NoButton);
		}
	} 
}

bool CAActionsEditor::saveActionsTable(const QString & filename, enum fileType type /* = FT_SHORTCUT */) {
	qDebug("CAActionsEditor::saveActions: '%s'", filename.toUtf8().data());

	QFile f( filename );
	if ( f.open( QIODevice::WriteOnly ) ) {
		QTextStream stream( &f );
		QString accelText;
		stream.setCodec("UTF-8");

		// @ToDo: Pretty Format output by adding \t as necessary
		for (int row=0; row < actionsTable->rowCount(); row++) {
			stream << actionsTable->item(row, COL_COMMAND)->text() << "\t" 
                   << actionsTable->item(row, COL_DESCRIPTION)->text() << "\t";
            if( type == FT_SHORTCUT /*|| type == FT_MIDISCUT*/)
			{
				accelText = actionsTable->item(row, COL_SHORTCUT)->text();
				if( accelText.isEmpty() )
					accelText = "none";
            	stream << accelText << "\t";
			}
			stream << actionsTable->item(row, COL_MIDI)->text() << "\n";
		}
		f.close();
		return true;
	}
	return false;
}

void CAActionsEditor::loadActionsTable() {
	QString sk;
    sk = tr("Shortcut files") +" (*.cakey *.cakmid *.cacks)";
    QString s = QFileDialog::getOpenFileName(
                    this, tr("Choose a file"),
                    latest_dir, sk );

    QFileInfo shortCutFileInfo(s);    
    enum fileType type = getFType( shortCutFileInfo.completeSuffix() );

    if (!s.isEmpty()) {
		latest_dir = QFileInfo(s).absolutePath();
        bool r = loadActionsTable(s, type);
		if (!r) {
			QMessageBox::warning(this, tr("Error"), 
               tr("The file couldn't be loaded"), 
               QMessageBox::Ok, Qt::NoButton);
		}
	}
}

bool CAActionsEditor::loadActionsTable(const QString & filename, enum fileType type /* = FT_SHORTCUT */) {
	qDebug("CAActionsEditor::loadActions: '%s'", filename.toUtf8().data());

	// Lines with '#' (comments) will be ignored
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
		QString command, context, accelText, midiText;
        while ( !stream.atEnd() ) {
            line = stream.readLine();
			qDebug("line: '%s'", line.toUtf8().data());
			if (rx.indexIn(line) > -1) {
				command = rx.cap(1);
				context = rx.cap(2);
                if( type == FT_SHORTCUT /*|| type == FT_MIDISCUT*/ )
				{
					accelText = rx.cap(3);
					if( accelText == "none" )
						accelText.clear();
					midiText = rx.cap(4) + " " + rx.cap(5) + " " + rx.cap(6);
					qDebug(" command: '%s' context: '%s' accel: '%s' midi: '%s'",
						command.toUtf8().data(), context.toUtf8().data(),
						accelText.toUtf8().data(), midiText.toUtf8().data());
					// @ToDo: command name is not identical to command identifier!
					row = findActionCommand(command);
					if (row > -1) {
						qDebug("Action found!");
						actionsTable->item(row, COL_SHORTCUT)->setText(accelText);
					}
				}
                if( type == FT_MIDI /*|| type == FT_MIDISCUT*/ )
				{
					midiText = rx.cap(3) + " " + rx.cap(4) + " " + rx.cap(5);
					qDebug(" command: '%s' context: '%s' midi: '%s'",
				 command.toUtf8().data(), accelText.toUtf8().data(), midiText.toUtf8().data());
				}
				actionsTable->item(row, COL_MIDI)->setText(accelText);
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

void CAActionsEditor::saveToConfig(QWidget *widget, QSettings *set) {
	qDebug("ActionsEditor::saveToConfig");

	set->beginGroup("actions");

	CASingleAction *action;
    // Issue: Actions not associated to objects anymore due to
    // Step that converts QAction -> CASingleAction
    //QList<CASingleAction *> actions = o->findChildren<CASingleAction *>();
	QString accelText;
    QList<QAction *> actions = widget->actions();
	for (int n=0; n < actions.count(); n++) {
        action = reinterpret_cast<CASingleAction*> (actions[n]);
        if (!action->getAction()->text().isEmpty() && !action->getAction()->inherits("QWidgetAction")) {
//#if USE_MULTIPLE_SHORTCUTS
//			accelText = shortcutsToString(action->shortcuts());
//#else
			accelText = action->getShortCutAsString();
//#endif
			if( accelText.isEmpty() )
                set->setValue(action->getAction()->text(), "none");
			else
                set->setValue(action->getAction()->text(), accelText);
		}
    }

	set->endGroup();
}


void CAActionsEditor::loadFromConfig(QWidget *widget, QSettings *set) {
	qDebug("ActionsEditor::loadFromConfig");

	set->beginGroup("actions");

	CASingleAction *action;
	QString accelText;

    // Issue: Actions not associated to objects anymore due to
    // Step that converts QAction -> CASingleAction
    //QList<CASingleAction *> actions = o->findChildren<CASingleAction *>();
//#if USE_MULTIPLE_SHORTCUTS
//	QString current;
//#endif
    QList<QAction *> actions = widget->actions();
    for (int n=0; n < actions.count(); n++) {
        action = reinterpret_cast<CASingleAction*> (actions[n]);
        if (!action->getAction()->objectName().isEmpty() && !action->getAction()->inherits("QWidgetAction")) {
//#if USE_MULTIPLE_SHORTCUTS
//			current = shortcutsToString(action->shortcuts());
//			accelText = set->value(action->objectName(), current).toString();
//			action->setShortcuts( stringToShortcuts( accelText ) );
//#else
            accelText = set->value(action->getAction()->text(), action->getShortCutAsString()).toString();
			if( accelText != "none" )
				action->setShortCutAsString( QKeySequence(accelText).toString() );
//#endif
		}
    }

	set->endGroup();
}

CASingleAction * CAActionsEditor::findAction(QWidget *widget, const QString & name) {
	CASingleAction *action;

    // Issue: Actions not associated to objects anymore due to
    // Step that converts QAction -> CASingleAction
    //QList<CASingleAction *> actions = o->findChildren<CASingleAction *>();
    QList<QAction *> actions = widget->actions();
    for (int n=0; n < actions.count(); n++) {
        action = reinterpret_cast<CASingleAction*> (actions[n]);
        if (name == action->getAction()->objectName()) return action;
    }

	return 0;
}

QStringList CAActionsEditor::actionsNames(QWidget *widget) {
	QStringList l;

	CASingleAction *action;

    // Issue: Actions not associated to objects anymore due to
    // Step that converts QAction -> CASingleAction
    //QList<CASingleAction *> actions = o->findChildren<CASingleAction *>();
    QList<QAction *> actions = widget->actions();
    for (int n=0; n < actions.count(); n++) {
        action = reinterpret_cast<CASingleAction*> (actions[n]);
		//qDebug("action name: '%s'", action->objectName().toUtf8().data());
		//qDebug("action name: '%s'", action->text().toUtf8().data());
        if (!action->getAction()->text().isEmpty())
            l.append( action->getAction()->text() );
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

static QString keyToString(int k)
{
      if (  k == Qt::Key_Shift || k == Qt::Key_Control || k == Qt::Key_Meta ||
                  k == Qt::Key_Alt || k == Qt::Key_AltGr )
            return QString::null;

      return QKeySequence(k).toString();
}

static QStringList modToString(Qt::KeyboardModifiers k)
{
      //qDebug("modToString: k: %x", (int) k);

      QStringList l;

      if ( k & Qt::ShiftModifier )
            l << "Shift";
      if ( k & Qt::ControlModifier )
            l << "Ctrl";
      if ( k & Qt::AltModifier )
            l << "Alt";
      if ( k & Qt::MetaModifier )
            l << "Meta";
      if ( k & Qt::GroupSwitchModifier )
            ;
      if ( k & Qt::KeypadModifier )
            ;

      return l;
}


ShortcutGetter::ShortcutGetter(QWidget *parent) : QDialog(parent)
{
      setWindowTitle(tr("Modify shortcut"));


      QVBoxLayout *vbox = new QVBoxLayout(this);
      vbox->setMargin(2);
      vbox->setSpacing(4);

      QLabel *l = new QLabel(this);
      l->setText(tr("Press the key combination you want to assign"));
      vbox->addWidget(l);

      leKey = new QLineEdit(this);

      leKey->installEventFilter(this);
      vbox->addWidget(leKey);

      // Change by rvm: use a QDialogButtonBox instead of QPushButtons
      // and add a clear button
      setCaptureKeyboard(true);
      QDialogButtonBox * buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                     QDialogButtonBox::Cancel |
                                                     QDialogButtonBox::Reset );
      QPushButton * clearbutton = buttonbox->button(QDialogButtonBox::Reset);
      clearbutton->setText( tr("Clear") );

      QPushButton * captureButton = new QPushButton(tr("Capture"), this);
      captureButton->setToolTip( tr("Capture keystrokes") );
      captureButton->setCheckable( captureKeyboard() );
      captureButton->setChecked( captureKeyboard() );
      connect(captureButton, SIGNAL(toggled(bool)),
            this, SLOT(setCaptureKeyboard(bool)));


      buttonbox->addButton(captureButton, QDialogButtonBox::ActionRole);

      connect( buttonbox, SIGNAL(accepted()), this, SLOT(accept()) );
      connect( buttonbox, SIGNAL(rejected()), this, SLOT(reject()) );
      connect( clearbutton, SIGNAL(clicked()), leKey, SLOT(clear()) );
      vbox->addWidget(buttonbox);
}

void ShortcutGetter::setCaptureKeyboard(bool b) {
      capture = b;
      leKey->setReadOnly(b);
      leKey->setFocus();
}


QString ShortcutGetter::exec(const QString& s)
{
      bStop = false;
      leKey->setText(s);

      if ( QDialog::exec() == QDialog::Accepted )
            return leKey->text();

      return QString();
}

bool ShortcutGetter::event(QEvent *e)
{
      if (!capture) return QDialog::event(e);


      QString key;
      QStringList mods;
      QKeyEvent *k = static_cast<QKeyEvent *>(e);

      switch ( e->type() )
      {
            case QEvent::KeyPress :

            if ( bStop )
            {
                  lKeys.clear();
                  bStop = false;
            }

            key = keyToString(k->key());
            mods = modToString(k->modifiers());

            //qDebug("event: key.count: %d, mods.count: %d", key.count(), mods.count());

            if ( key.count() || mods.count() )
            {

                  if ( key.count() && !lKeys.contains(key) )
                        lKeys << key;

                  foreach ( key, mods )
                        if ( !lKeys.contains(key) )
                              lKeys << key;

                  } else {
                        key = k->text();

                        if ( !lKeys.contains(key) )
                              lKeys << key;
                  }

                  setText();
                  break;

            case QEvent::KeyRelease :

                  bStop = true;
                  break;

                  /*
            case QEvent::ShortcutOverride :
                  leKey->setText("Shortcut override");
                  break;
                  */

            default:
                  return QDialog::event(e);
                  break;
      }

      return true;
}

bool ShortcutGetter::eventFilter(QObject *o, QEvent *e)
{
      if (!capture) return QDialog::eventFilter(o, e);

      if (  e->type() == QEvent::KeyPress ||
                  e->type() ==QEvent::KeyRelease )
            return event(e);
      else
            return QDialog::eventFilter(o, e);
}

void ShortcutGetter::setText()
{
      QStringList seq;

      if ( lKeys.contains("Shift") )
            seq << "Shift";

      if ( lKeys.contains("Ctrl") )
            seq << "Ctrl";

      if ( lKeys.contains("Alt") )
            seq << "Alt";

      if ( lKeys.contains("Meta") )
            seq << "Meta";

      foreach ( QString s, lKeys ) {
            //qDebug("setText: s: '%s'", s.toUtf8().data());
            if ( s != "Shift" && s != "Ctrl"
                  && s != "Alt" && s != "Meta" )
                  seq << s;
      }

      leKey->setText(seq.join("+"));
      //leKey->selectAll();
}
