/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef PROPERTIESDIALOG_H_
#define PROPERTIESDIALOG_H_

#include "ui_propertiesdialog.h"
#include "ui_documentproperties.h"
#include "ui_sheetproperties.h"
#include "ui_staffproperties.h"
#include "ui_voiceproperties.h"
#include "ui_lyricscontextproperties.h"
#include "ui_functionmarkingproperties.h"

class QTreeWidgetItem;

class CADocument;
class CASheet;
class CAStaff;
class CAContext;
class CAVoice;
class CALyricsContext;
class CAFunctionMarkingContext;

class CADocumentProperties : public QWidget, public Ui::uiDocumentProperties {
	Q_OBJECT
public:
	CADocumentProperties( CADocument *doc, QWidget *parent=0 ) : QWidget( parent ) { _document = doc; setupUi(this); }
public slots:
	void on_uiComposer_editingFinished();
private:
	CADocument *_document;
};

class CASheetProperties : public QWidget, public Ui::uiSheetProperties {
public:
	CASheetProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
};

class CAStaffProperties : public QWidget, public Ui::uiStaffProperties {
public:
	CAStaffProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
};

class CAVoiceProperties : public QWidget, public Ui::uiVoiceProperties {
public:
	CAVoiceProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
};

class CALyricsContextProperties : public QWidget, public Ui::uiLyricsContextProperties {
public:
	CALyricsContextProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
};

class CAFunctionMarkingContextProperties : public QWidget, public Ui::uiFunctionMarkingContextProperties {
public:
	CAFunctionMarkingContextProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
};

class CAPropertiesDialog : public QDialog, private Ui::uiPropertiesDialog
{
	Q_OBJECT
public:
	CAPropertiesDialog( CADocument *doc, QWidget *parent=0 );
	virtual ~CAPropertiesDialog();
	static void documentProperties( CADocument *doc, QWidget *parent );
	static void sheetProperties( CASheet *sheet, QWidget *parent );
	static void contextProperties( CAContext *context, QWidget *parent );
	static void voiceProperties( CAVoice *voice, QWidget *parent );
	
	inline CADocument *document() { return _document; }
	inline QTreeWidgetItem *documentItem() { return _documentItem; }
	inline QHash< QTreeWidgetItem*, CASheet* >& sheetItem() { return _sheetItem; }
	inline QHash< QTreeWidgetItem*, CAContext* >& contextItem() { return _contextItem; }
	inline QHash< QTreeWidgetItem*, CAVoice* >& voiceItem() { return _voiceItem; }
	
public slots:
	void on_uiDocumentTree_currentItemChanged( QTreeWidgetItem *cur, QTreeWidgetItem *prev );
	void on_uiButtonBox_clicked( QAbstractButton* );
	void on_uiUp_clicked( bool );
	void on_uiDown_clicked( bool );
	
private:
	void buildTree();
	
	void applyProperties();
	void updateDocumentProperties( CADocument* );
	void updateSheetProperties( CASheet* );
	void updateStaffProperties( CAStaff* );
	void updateVoiceProperties( CAVoice* );
	void updateLyricsContextProperties( CALyricsContext* );
	void updateFunctionMarkingContextProperties( CAFunctionMarkingContext* );
	
	CADocument                           *_document;
	QTreeWidgetItem                      *_documentItem;             // Document => Document properties widget
	QWidget                              *_documentPropertiesWidget; // Document item in tree widget => Document
	QHash< CASheet*, QWidget* >           _sheetPropertiesWidget;    // Sheet => Sheet properties widget
	QHash< QTreeWidgetItem*, CASheet* >   _sheetItem;                // Sheet item in tree widget => Sheet
	QHash< CAContext*, QWidget* >         _contextPropertiesWidget;  // Context => Context properties widget
	QHash< QTreeWidgetItem*, CAContext* > _contextItem;              // Context item in tree widget => Context
	QHash< CAVoice*, QWidget* >           _voicePropertiesWidget;    // Voice => Voice properties widget
	QHash< QTreeWidgetItem*, CAVoice* >   _voiceItem;                // Voice item in tree widget => Voice
};

#endif /* PROPERTIESDIALOG_H_ */
