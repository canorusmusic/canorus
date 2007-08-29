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
public:
	CADocumentProperties( QWidget *parent=0 ) : QWidget( parent ) { setupUi(this); }
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
	
	inline QTreeWidgetItem *documentItem() { return _documentItem; }
	inline QHash< QTreeWidgetItem*, CASheet* >& sheetItem() { return _sheetItem; }
	inline QHash< QTreeWidgetItem*, CAContext* >& contextItem() { return _contextItem; }
	inline QHash< QTreeWidgetItem*, CAVoice* >& voiceItem() { return _voiceItem; }
	
public slots:
	void on_uiDocumentTree_currentItemChanged( QTreeWidgetItem *cur, QTreeWidgetItem *prev );
	void on_uiButtonBox_clicked( QAbstractButton* );
	
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
	QTreeWidgetItem                      *_documentItem;
	QWidget                              *_documentPropertiesWidget;
	QHash< CASheet*, QWidget* >           _sheetPropertiesWidget;
	QHash< QTreeWidgetItem*, CASheet* >   _sheetItem;
	QHash< CAContext*, QWidget* >         _contextPropertiesWidget;
	QHash< QTreeWidgetItem*, CAContext* > _contextItem;
	QHash< CAVoice*, QWidget* >           _voicePropertiesWidget;
	QHash< QTreeWidgetItem*, CAVoice* >   _voiceItem;
};

#endif /* PROPERTIESDIALOG_H_ */
