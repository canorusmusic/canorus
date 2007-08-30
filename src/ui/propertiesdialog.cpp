/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QTreeWidgetItem>

#include "ui/propertiesdialog.h"

#include "core/canorus.h"
#include "core/undo.h"

#include "core/document.h"
#include "core/sheet.h"
#include "core/staff.h"
#include "core/voice.h"
#include "core/lyricscontext.h"
#include "core/functionmarkingcontext.h"

/*!
	\class CAPropertiesDialog
	\brief Advanced Document, Sheet, Staff etc. properties
	
	This dialog offers changing all the settings for Canorus objects. It is similar to CASettingsDialog.
	On the left it shows you a tree widget view of the current document and allows you to select
	one of the objects and sub-objects. On the right, the object properties are shown. Changes are
	managed using Ok, Apply and Cancel buttons.
	
	To use this dialog, call one of the static methods documentProperties(), sheetProperties(), contextProperties()
	or voiceProperties() and pass the current document, sheet, context or voice. Methods will generate and show
	the dialog and delete it in the end. 
	
	Actual objects properties widgets (properties widget for document, sheet etc.) are stored inside *.ui files.
*/

CAPropertiesDialog::CAPropertiesDialog( CADocument *doc, QWidget *parent )
 : QDialog(parent) {
	_document = doc;
	
	Ui::uiPropertiesDialog::setupUi( this );
	buildTree();
}

CAPropertiesDialog::~CAPropertiesDialog() {
}

/*!
	Fills the content of the document tree on the left.
	Tree always shows the whole Document structure.
*/
void CAPropertiesDialog::buildTree() {
	// Locate resources (images, icons)
	QString currentPath = QDir::currentPath();
	
	QList<QString> resourcesLocations = CACanorus::locateResourceDir(QString("images"));
	if (!resourcesLocations.size()) // when Canorus not installed, search the source path
		resourcesLocations = CACanorus::locateResourceDir(QString("ui/images"));
		
	QDir::setCurrent( resourcesLocations[0] ); /// \todo Button and menu icons by default look at the current working directory as their resource path only. QResource::addSearchPath() doesn't work for external icons. Any other ideas? -Matevz

	QWidget *w=0;
	QTreeWidgetItem *docItem=0;
	if (_document) {
		w = new CADocumentProperties( this );
		_documentPropertiesWidget = w;
		uiPropertiesWidget->addWidget( w );
		updateDocumentProperties( _document );
		
		docItem = new QTreeWidgetItem( uiDocumentTree );
		docItem->setText( 0, tr("Document") );
		docItem->setIcon( 0, QIcon("images/document.svg") );
		_documentItem = docItem;
		
		for ( int i=0; i<_document->sheetCount(); i++) {
			w = new CASheetProperties( this );
			_sheetPropertiesWidget[ _document->sheetAt(i) ] = w;
			uiPropertiesWidget->addWidget( w );
			updateSheetProperties( _document->sheetAt(i) );
			
			QTreeWidgetItem *sheetItem=0;
			sheetItem = new QTreeWidgetItem( docItem );
			sheetItem->setText( 0, _document->sheetAt(i)->name() );
			sheetItem->setIcon( 0, QIcon("images/sheet.svg") );
			_sheetItem[ sheetItem ] = _document->sheetAt(i);
			
			for ( int j=0; j<_document->sheetAt(i)->contextCount(); j++ ) {
				QTreeWidgetItem *contextItem=0;
				contextItem = new QTreeWidgetItem( sheetItem );
				contextItem->setText( 0, _document->sheetAt(i)->contextAt(j)->name() );
				_contextItem[ contextItem ] = _document->sheetAt(i)->contextAt(j);
				
				if (dynamic_cast<CAStaff*>( _document->sheetAt(i)->contextAt(j) )) {
					contextItem->setIcon( 0, QIcon("images/staff.svg") );
					w = new CAStaffProperties( this );
					uiPropertiesWidget->addWidget( w );
					_contextPropertiesWidget[ _document->sheetAt(i)->contextAt(j) ] = w;
					CAStaff *s = static_cast<CAStaff*>(_document->sheetAt(i)->contextAt(j));
					updateStaffProperties( s );	
					
					for (int k=0; k<s->voiceCount(); k++) {
						QWidget *v = new CAVoiceProperties( this );
						uiPropertiesWidget->addWidget( v );
						_voicePropertiesWidget[ s->voiceAt(k) ] = v;
						updateVoiceProperties( s->voiceAt(k) );
						
						QTreeWidgetItem *voiceItem=0;
						voiceItem = new QTreeWidgetItem( contextItem );
						voiceItem->setText( 0, s->voiceAt(k)->name() );
						voiceItem->setIcon( 0, QIcon("images/voice.svg") );
						_voiceItem[ voiceItem ] = s->voiceAt(k);
					}
				} else
				if (dynamic_cast<CALyricsContext*>( _document->sheetAt(i)->contextAt(j) )) {
					contextItem->setIcon( 0, QIcon("images/lyricscontext.svg") );
					w = new CALyricsContextProperties( this );
					uiPropertiesWidget->addWidget( w );
					_contextPropertiesWidget[ _document->sheetAt(i)->contextAt(j) ] = w;
					updateLyricsContextProperties( static_cast<CALyricsContext*>(_document->sheetAt(i)->contextAt(j)) );					
				} else
				if (dynamic_cast<CAFunctionMarkingContext*>( _document->sheetAt(i)->contextAt(j) )) {
				contextItem->setIcon( 0, QIcon("images/fmcontext.svg") );
				w = new CAFunctionMarkingContextProperties( this );
					uiPropertiesWidget->addWidget( w );
					_contextPropertiesWidget[ _document->sheetAt(i)->contextAt(j) ] = w;
					updateFunctionMarkingContextProperties( static_cast<CAFunctionMarkingContext*>(_document->sheetAt(i)->contextAt(j)) );					
				}
				
			}
		}
		
		uiDocumentTree->addTopLevelItem( docItem );
		uiDocumentTree->expandAll();
	}
	
	QDir::setCurrent( currentPath );
}

void CAPropertiesDialog::documentProperties( CADocument *doc, QWidget *parent ) {
	if (doc) {
		CAPropertiesDialog pd( doc, parent );
		pd.uiDocumentTree->setCurrentItem( pd.documentItem() );
		pd.exec();
		CACanorus::rebuildUI( doc );
	}
}

void CAPropertiesDialog::sheetProperties( CASheet *sheet, QWidget *parent ) {
	if ( sheet && sheet->document() ) {
		CAPropertiesDialog pd( sheet->document(), parent );
		pd.uiDocumentTree->setCurrentItem( pd.sheetItem().key(sheet) );
		pd.exec();
		CACanorus::rebuildUI( sheet->document() );
	}
}

void CAPropertiesDialog::contextProperties( CAContext *context, QWidget *parent ) {
	if ( context && context->sheet() && context->sheet()->document() ) {
		CAPropertiesDialog pd( context->sheet()->document(), parent );
		pd.uiDocumentTree->setCurrentItem( pd.contextItem().key(context) );
		pd.exec();
		CACanorus::rebuildUI( context->sheet()->document() );
	}
}

void CAPropertiesDialog::voiceProperties( CAVoice *voice, QWidget *parent ) {
	if ( voice && voice->staff() && voice->staff()->sheet() &&
	     voice->staff()->sheet()->document() ) {
		CAPropertiesDialog pd( voice->staff()->sheet()->document(), parent );
		pd.uiDocumentTree->setCurrentItem( pd.voiceItem().key(voice) );
		pd.exec();
		CACanorus::rebuildUI( voice->staff()->sheet()->document() );
	}
}

void CAPropertiesDialog::on_uiDocumentTree_currentItemChanged( QTreeWidgetItem *cur, QTreeWidgetItem *prev ) {
	uiElementName->setText( cur->text(0) );
	
	if ( _documentItem == cur ) {
		updateDocumentProperties( _document );
	} else
	if ( _sheetItem.contains(cur) ) {
		updateSheetProperties( _sheetItem[cur] );
	} else
	if ( _contextItem.contains(cur) ) {
		switch ( _contextItem[cur]->contextType() ) {
			case CAContext::Staff:
				updateStaffProperties( static_cast<CAStaff*>( _contextItem[cur] ) );
				break;
			case CAContext::LyricsContext:
				updateLyricsContextProperties( static_cast<CALyricsContext*>( _contextItem[cur] ) );
				break;
			case CAContext::FunctionMarkingContext:
				updateFunctionMarkingContextProperties( static_cast<CAFunctionMarkingContext*>( _contextItem[cur] ) );
				break;
		}
	} else
	if ( _voiceItem.contains(cur) ) {
		updateVoiceProperties( _voiceItem[cur] );
	}
}

void CAPropertiesDialog::on_uiButtonBox_clicked( QAbstractButton* button ) {
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Ok ) {
		applyProperties();
		hide();
	} else
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Cancel ) {
		hide();
	} else
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Apply ) {
		applyProperties();
	}
}

void CAPropertiesDialog::applyProperties() {
	QTreeWidgetItem *item = uiDocumentTree->topLevelItem(0);
	
	CACanorus::undo()->createUndoCommand( _document, tr("apply properties") );
	CACanorus::undo()->pushUndoCommand();
	
	// store Document properties
	CADocumentProperties *dp = static_cast<CADocumentProperties*>(_documentPropertiesWidget);
	_document->setTitle( dp->uiTitle->text() );
	_document->setSubtitle( dp->uiSubtitle->text() );
	_document->setComposer( dp->uiComposer->text() );
	_document->setArranger( dp->uiArranger->text() );
	_document->setPoet( dp->uiPoet->text() );
	_document->setTextTranslator( dp->uiTextTranslator->text() );
	_document->setDedication( dp->uiDedication->text() );
	_document->setCopyright( dp->uiCopyright->text() );
	_document->setComments( dp->uiComments->toPlainText() );
	
	// store Sheet properties
	for (int i=0; i<_sheetPropertiesWidget.keys().size(); i++) {
		CASheet *s = _sheetPropertiesWidget.keys().at(i);
	}
	
	// store Context properties
	for (int i=0; i<_contextPropertiesWidget.keys().size(); i++) {
		CAContext *c = _contextPropertiesWidget.keys().at(i);
		switch (c->contextType()) {
			case CAContext::Staff:
				break;
			case CAContext::LyricsContext:
				break;
			case CAContext::FunctionMarkingContext:
				break;
		}
	}
	
	// store Voice properties
	for (int i=0; i<_voicePropertiesWidget.keys().size(); i++) {
		CAVoice *voice = _voicePropertiesWidget.keys().at(i);
		CAVoiceProperties *vp = static_cast<CAVoiceProperties*>(_voicePropertiesWidget[ voice ]);
		voice->setMidiChannel( vp->uiMidiChannel->value() );
	}
}

void CAPropertiesDialog::updateDocumentProperties( CADocument *doc ) {
	CADocumentProperties *dp = static_cast<CADocumentProperties*>(_documentPropertiesWidget);
	
	dp->uiTitle->setText( doc->title() );
	dp->uiSubtitle->setText( doc->subtitle() );
	dp->uiComposer->setText( doc->composer() );
	dp->uiArranger->setText( doc->arranger() );
	dp->uiPoet->setText( doc->poet() );
	dp->uiTextTranslator->setText( doc->textTranslator() );
	dp->uiDedication->setText( doc->dedication() );
	dp->uiCopyright->setText( doc->copyright() );
	dp->uiComments->setText( doc->comments() );
	
	uiPropertiesWidget->setCurrentWidget( dp );
}

void CAPropertiesDialog::updateSheetProperties( CASheet *sheet ) {
	uiPropertiesWidget->setCurrentWidget( _sheetPropertiesWidget[sheet] );
}

void CAPropertiesDialog::updateStaffProperties( CAStaff *staff ) {
	CAStaffProperties *sp = static_cast<CAStaffProperties*>( _contextPropertiesWidget[staff] );
	
	uiPropertiesWidget->setCurrentWidget( _contextPropertiesWidget[staff] );
}

void CAPropertiesDialog::updateVoiceProperties( CAVoice *voice ) {
	static_cast<CAVoiceProperties*>(_voicePropertiesWidget[voice])->uiMidiChannel->setValue( voice->midiChannel() );
	
	uiPropertiesWidget->setCurrentWidget( _voicePropertiesWidget[voice] );
}

void CAPropertiesDialog::updateLyricsContextProperties( CALyricsContext *lc ) {
	uiPropertiesWidget->setCurrentWidget( _contextPropertiesWidget[lc] );
}

void CAPropertiesDialog::updateFunctionMarkingContextProperties( CAFunctionMarkingContext *fmc ) {
	uiPropertiesWidget->setCurrentWidget( _contextPropertiesWidget[fmc] );
}
