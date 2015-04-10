/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed punder the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QSettings>
#include <QDir>
#include <QFileDialog>

// Python.h needs to be loaded first!
#include "canorus.h"
#include "ui/settingsdialog.h"
#include "interface/mididevice.h"
#include "widgets/actionseditor.h"
#include "core/settings.h"
#include "score/sheet.h"         // needed for preview sheet
#include "score/staff.h"         // needed for preview sheet
#include "score/voice.h"         // needed for preview sheet
#include "score/clef.h"          // needed for preview sheet
#include "score/timesignature.h" // needed for preview sheet

/*!
	\class CASettingsDialog
	Settings dialog for various options like editor behaviour, loading/saving settings,
	colors, playback options etc.

	\sa CASettings
*/

CASettingsDialog::CASettingsDialog( CASettingsPage currentPage, QWidget *parent )
 : QDialog( parent ) {
	setupUi( this );

	buildPreviewSheet();
	buildActionsEditorPage();
	setupPages( currentPage );


	exec();
}

CASettingsDialog::~CASettingsDialog() {
	delete _previewSheet;
}

void CASettingsDialog::setupPages( CASettingsPage currentPage ) {
	// Editor Page
	uiFinaleLyricsCheckBox->setChecked( CACanorus::settings()->finaleLyricsBehaviour() );
	uiShadowNotesInOtherStaffs->setChecked( CACanorus::settings()->shadowNotesInOtherStaffs() );
	uiPlayInsertedNotes->setChecked( CACanorus::settings()->playInsertedNotes() );
	uiAutoBar->setChecked( CACanorus::settings()->autoBar() );
	uiSplitAtQuarterBoundaries->setChecked( CACanorus::settings()->splitAtQuarterBoundaries() );

	// Appearance Page
	uiAntiAliasing->setChecked( CACanorus::settings()->antiAliasing() );
	uiAnimatedScroll->setChecked( CACanorus::settings()->animatedScroll() );
	uiForegroundColor->setPalette( QPalette( CACanorus::settings()->foregroundColor() ) );
	uiBackgroundColor->setPalette( QPalette( CACanorus::settings()->backgroundColor() ) );
	uiSelectionColor->setPalette( QPalette( CACanorus::settings()->selectionColor() ) );
	uiSelectionAreaColor->setPalette( QPalette( CACanorus::settings()->selectionAreaColor() ) );
	uiSelectedContextColor->setPalette( QPalette( CACanorus::settings()->selectedContextColor() ) );
	uiHiddenElementsColor->setPalette( QPalette( CACanorus::settings()->hiddenElementsColor() ) );
	uiDisabledElementsColor->setPalette( QPalette( CACanorus::settings()->disabledElementsColor() ) );
	uiPreviewScoreView->setSheet( _previewSheet );
	uiPreviewScoreView->setGrabTabKey( false );
	uiPreviewScoreView->setScrollBarVisible( CAScoreView::ScrollBarAlwaysHidden );
	uiPreviewScoreView->rebuild();
	uiPreviewScoreView->setZoom(0.6, 0, 0, false, false);
	uiPreviewScoreView->setCurrentContext( uiPreviewScoreView->findCElement( _previewSheet->staffList()[0] ) );
	uiPreviewScoreView->addSelectionRegion( QRect(50, 40, 70, 90) );
	uiPreviewScoreView->addToSelection( _previewSheet->staffList()[0]->voiceList()[0]->musElementList()[1] );
	uiPreviewScoreView->repaint();

	// Commands Settings Page

	// Loading Saving Page
	uiDocumentsDirectory->setText( CACanorus::settings()->documentsDirectory().absolutePath() );

	uiDefaultSaveComboBox->addItems( CAMainWin::uiSaveDialog->nameFilters() );
	uiDefaultSaveComboBox->setCurrentIndex(
		uiDefaultSaveComboBox->findText(
			CAFileFormats::getFilter(CACanorus::settings()->defaultSaveFormat())
		)
	);

	uiAutoRecoverySpinBox->setValue( CACanorus::settings()->autoRecoveryInterval() );

	// Playback Page
	_midiInPorts = CACanorus::midiDevice()->getInputPorts();
	_midiOutPorts = CACanorus::midiDevice()->getOutputPorts();

	uiMidiInList->addItem( tr("None") );
	for ( int i=0; i<_midiInPorts.values().size(); i++ ) {
		uiMidiInList->addItem( _midiInPorts.value(i) );
		if ( CACanorus::settings()->midiInPort()==_midiInPorts.keys().at(i) )
			uiMidiInList->setCurrentItem( uiMidiInList->item(i+1) );               // select the previous device
	}
	if ( CACanorus::settings()->midiInPort()==-1 )
		uiMidiInList->setCurrentItem( uiMidiInList->item(0) );                     // select the previous device

	uiMidiOutList->addItem( tr("None") );
	for ( int i=0; i<_midiOutPorts.values().size(); i++ ) {
		uiMidiOutList->addItem( _midiOutPorts.value(i) );
		if ( CACanorus::settings()->midiOutPort()==_midiOutPorts.keys().at(i) )
			uiMidiOutList->setCurrentItem( uiMidiOutList->item(i+1) );             // select the previous device
	}
	if ( CACanorus::settings()->midiOutPort()==-1 )
		uiMidiOutList->setCurrentItem( uiMidiOutList->item(0) );                   // select the previous device

	uiSettingsList->setCurrentRow( (currentPage!=UndefinedSettings)?currentPage:0 );

	// Printing Page
	uiTypesetter->setCurrentIndex( CACanorus::settings()->typesetter()-1 );
	uiTypesetterLocation->setText( CACanorus::settings()->typesetterLocation() );
	uiTypesetterDefault->setChecked( CACanorus::settings()->useSystemDefaultTypesetter() );
	uiPdfViewerLocation->setText( CACanorus::settings()->pdfViewerLocation() );
	uiPdfViewerDefault->setChecked( CACanorus::settings()->useSystemDefaultPdfViewer() );
}

void CASettingsDialog::on_uiButtonBox_clicked( QAbstractButton *button ) {
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Ok ) {
		applySettings();
		hide();
	} else
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Cancel ) {
		hide();
	} else
	if ( uiButtonBox->standardButton(button) == QDialogButtonBox::Apply ) {
		applySettings();
	}
}

void CASettingsDialog::on_uiSettingsList_currentItemChanged( QListWidgetItem * current, QListWidgetItem * previous ) {
	uiPageNameLabel->setText( current->text() );
	uiStackedWidget->setCurrentIndex( uiSettingsList->row(current) );
}

void CASettingsDialog::applySettings() {
	// Editor Page
	CACanorus::settings()->setFinaleLyricsBehaviour( uiFinaleLyricsCheckBox->isChecked() );
	CACanorus::settings()->setShadowNotesInOtherStaffs( uiShadowNotesInOtherStaffs->isChecked() );
	CACanorus::settings()->setPlayInsertedNotes( uiPlayInsertedNotes->isChecked() );
	CACanorus::settings()->setAutoBar( uiAutoBar->isChecked() );
	CACanorus::settings()->setSplitAtQuarterBoundaries( uiSplitAtQuarterBoundaries->isChecked() );

	// Saving/Loading Page
	CACanorus::settings()->setDocumentsDirectory( uiDocumentsDirectory->text() );
	CAMainWin::uiOpenDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiSaveDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiImportDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiExportDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CACanorus::settings()->setDefaultSaveFormat( CAFileFormats::getType( uiDefaultSaveComboBox->currentText() ) );
	CAMainWin::uiSaveDialog->selectNameFilter( uiDefaultSaveComboBox->currentText() );
	CACanorus::settings()->setAutoRecoveryInterval( uiAutoRecoverySpinBox->value() );
	CACanorus::autoRecovery()->updateTimer();

	// Appearance Page
	CACanorus::settings()->setAntiAliasing( uiAntiAliasing->isChecked() );
	CACanorus::settings()->setAnimatedScroll( uiAnimatedScroll->isChecked() );
	CACanorus::settings()->setBackgroundColor( uiBackgroundColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setForegroundColor( uiForegroundColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setSelectionColor( uiSelectionColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setSelectionAreaColor( uiSelectionAreaColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setSelectedContextColor( uiSelectedContextColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setHiddenElementsColor( uiHiddenElementsColor->palette().color(QPalette::Window) );
	CACanorus::settings()->setDisabledElementsColor( uiDisabledElementsColor->palette().color(QPalette::Window) );

	// Playback Page
	if ( uiMidiInList->currentIndex().row()==0 )
		CACanorus::settings()->setMidiInPort(-1);
	else
		CACanorus::settings()->setMidiInPort(_midiInPorts.keys().at( uiMidiInList->currentIndex().row()-1 ));

	if ( uiMidiOutList->currentIndex().row()==0 )
		CACanorus::settings()->setMidiOutPort(-1);
	else
		CACanorus::settings()->setMidiOutPort(_midiOutPorts.keys().at( uiMidiOutList->currentIndex().row()-1 ));

	// Printing Page
	CACanorus::settings()->setTypesetter( static_cast<CATypesetter::CATypesetterType>(uiTypesetter->currentIndex()+1) );
	CACanorus::settings()->setTypesetterLocation( uiTypesetterLocation->text() );
	CACanorus::settings()->setUseSystemDefaultTypesetter( uiTypesetterDefault->isChecked() );
	CACanorus::settings()->setPdfViewerLocation( uiPdfViewerLocation->text() );
	CACanorus::settings()->setUseSystemDefaultPdfViewer( uiPdfViewerDefault->isChecked() );

	CACanorus::settings()->writeSettings();
}

void CASettingsDialog::buildActionsEditorPage()
{
	int i;
	QWidget oSingleActions; // all actions added here
	const QList<QAction *> &roSAList = CACanorus::settings()->getActionList();
	_commandsEditor = new CAActionsEditor( 0 );
	// Read all elements from single action list (API requirement)
	for(i=0; i< roSAList.size(); ++i)
		oSingleActions.addAction( roSAList[i] );
	// Add all command actions (loading happens earlier in Canorus)
	_commandsEditor->addActions( &oSingleActions );
}

void CASettingsDialog::buildPreviewSheet() {
	_previewSheet = new CASheet( "", 0 );
	_previewSheet->addStaff();
	_previewSheet->staffList()[0]->voiceList()[0]->append( new CAClef( CAClef::Treble, _previewSheet->staffList()[0], 0 ) );
	_previewSheet->staffList()[0]->voiceList()[0]->append( new CATimeSignature( 2, 2, _previewSheet->staffList()[0], 0 ) );
	_previewSheet->addStaff();
}

void CASettingsDialog::on_uiDocumentsDirectoryBrowse_clicked(bool) {
	QString dir = QFileDialog::getExistingDirectory(this, tr("Choose default documents directory"));
	if (dir.size())
		uiDocumentsDirectory->setText( dir );
}

void CASettingsDialog::on_uiDocumentsDirectoryRevert_clicked(bool) {
	uiDocumentsDirectory->setText( CASettings::DEFAULT_DOCUMENTS_DIRECTORY.absolutePath() );
}

void CASettingsDialog::on_uiBackgroundColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiBackgroundColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiBackgroundColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setBackgroundColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiBackgroundRevert_clicked(bool) {
	uiBackgroundColor->setPalette( QPalette(CASettings::DEFAULT_BACKGROUND_COLOR) );
	uiPreviewScoreView->setBackgroundColor(CASettings::DEFAULT_BACKGROUND_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiForegroundColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiForegroundColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiForegroundColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setForegroundColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiForegroundRevert_clicked(bool) {
	uiForegroundColor->setPalette( QPalette(CASettings::DEFAULT_FOREGROUND_COLOR) );
	uiPreviewScoreView->setForegroundColor(CASettings::DEFAULT_FOREGROUND_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiSelectionColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectionColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectionColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setSelectionColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiSelectionRevert_clicked(bool) {
	uiSelectionColor->setPalette( QPalette(CASettings::DEFAULT_SELECTION_COLOR) );
	uiPreviewScoreView->setSelectionColor(CASettings::DEFAULT_SELECTION_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiSelectionAreaColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectionAreaColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectionAreaColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setSelectionAreaColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiSelectionAreaRevert_clicked(bool) {
	uiSelectionAreaColor->setPalette( QPalette(CASettings::DEFAULT_SELECTION_AREA_COLOR) );
	uiPreviewScoreView->setSelectionAreaColor(CASettings::DEFAULT_SELECTION_AREA_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiSelectedContextColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectedContextColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectedContextColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setSelectedContextColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiSelectedContextRevert_clicked(bool) {
	uiSelectedContextColor->setPalette( QPalette(CASettings::DEFAULT_SELECTED_CONTEXT_COLOR) );
	uiPreviewScoreView->setSelectedContextColor(CASettings::DEFAULT_SELECTED_CONTEXT_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiHiddenElementsColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiHiddenElementsColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiHiddenElementsColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setHiddenElementsColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiHiddenElementsRevert_clicked(bool) {
	uiHiddenElementsColor->setPalette( QPalette(CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR) );
	uiPreviewScoreView->setHiddenElementsColor(CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiDisabledElementsColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiDisabledElementsColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiDisabledElementsColor->setPalette( QPalette(c) );
		uiPreviewScoreView->setDisabledElementsColor(c);
		uiPreviewScoreView->repaint();
	}
}

void CASettingsDialog::on_uiDisabledElementsRevert_clicked(bool) {
	uiDisabledElementsColor->setPalette( QPalette(CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR) );
	uiPreviewScoreView->setDisabledElementsColor(CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR);
	uiPreviewScoreView->repaint();
}

void CASettingsDialog::on_uiTypesetterBrowse_clicked(bool) {
	QString path = QFileDialog::getOpenFileName( static_cast<QWidget*>(parent()), tr("Select typesetter executable") );
	if ( !path.isNull() ) {
		uiTypesetterLocation->setText( path );
	}
}

void CASettingsDialog::on_uiPdfViewerBrowse_clicked(bool) {
	QString path = QFileDialog::getOpenFileName( static_cast<QWidget*>(parent()), tr("Select PDF viewer executable") );
	if ( !path.isNull() ) {
		uiPdfViewerLocation->setText( path );
	}
}

void CASettingsDialog::on_uiTypesetterDefault_toggled(bool checked) {
	if (checked) {
		uiTypesetterLocation->setEnabled( false );
		uiTypesetterBrowse->setEnabled( false );
	} else {
		uiTypesetterLocation->setEnabled( true );
		uiTypesetterBrowse->setEnabled( true );
	}
}

void CASettingsDialog::on_uiPdfViewerDefault_toggled(bool checked) {
	if (checked) {
		uiPdfViewerLocation->setEnabled( false );
		uiPdfViewerBrowse->setEnabled( false );
	} else {
		uiPdfViewerLocation->setEnabled( true );
		uiPdfViewerBrowse->setEnabled( true );
	}
}
