/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QSettings>
#include <QDir>

// Python.h needs to be loaded first!
#include "canorus.h"
#include "ui/settingsdialog.h"
#include "interface/mididevice.h"
#include "core/settings.h"
#include "core/sheet.h"         // needed for preview sheet
#include "core/staff.h"         // needed for preview sheet
#include "core/voice.h"         // needed for preview sheet
#include "core/clef.h"          // needed for preview sheet
#include "core/timesignature.h" // needed for preview sheet

/*!
	\class CASettingsDialog
	Settings dialog for various options like editor behaviour, loading/saving settings,
	colors, playback options etc.
*/

CASettingsDialog::CASettingsDialog( CASettingsPage currentPage, QWidget *parent )
 : QDialog( parent ) {
	CACanorus::setImagesPath();

	setupUi( this );

	CACanorus::restorePath();

	buildPreviewSheet();
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

	// Appearance Page
	uiForegroundColor->setPalette( QPalette( CACanorus::settings()->foregroundColor() ) );
	uiBackgroundColor->setPalette( QPalette( CACanorus::settings()->backgroundColor() ) );
	uiSelectionColor->setPalette( QPalette( CACanorus::settings()->selectionColor() ) );
	uiSelectionAreaColor->setPalette( QPalette( CACanorus::settings()->selectionAreaColor() ) );
	uiSelectedContextColor->setPalette( QPalette( CACanorus::settings()->selectedContextColor() ) );
	uiHiddenElementsColor->setPalette( QPalette( CACanorus::settings()->hiddenElementsColor() ) );
	uiDisabledElementsColor->setPalette( QPalette( CACanorus::settings()->disabledElementsColor() ) );
	uiPreviewScoreViewPort->setSheet( _previewSheet );
	uiPreviewScoreViewPort->setScrollBarVisible( CAScoreViewPort::ScrollBarAlwaysHidden );
	uiPreviewScoreViewPort->rebuild();
	uiPreviewScoreViewPort->setZoom(0.6, 0, 0, false, false);
	uiPreviewScoreViewPort->setCurrentContext( uiPreviewScoreViewPort->findCElement( _previewSheet->staffAt(0) ) );
	uiPreviewScoreViewPort->addSelectionRegion( QRect(50, 40, 70, 90) );
	uiPreviewScoreViewPort->addToSelection( _previewSheet->staffAt(0)->voiceAt(0)->musElementAt(1) );
	uiPreviewScoreViewPort->repaint();

	// Loading Saving Page
	uiDocumentsDirectory->setText( CACanorus::settings()->documentsDirectory().absolutePath() );

	uiDefaultSaveComboBox->addItems( CAMainWin::uiSaveDialog->filters() );
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

	// Saving/Loading Page
	CACanorus::settings()->setDocumentsDirectory( uiDocumentsDirectory->text() );
	CAMainWin::uiOpenDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiSaveDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiImportDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CAMainWin::uiExportDialog->setDirectory( CACanorus::settings()->documentsDirectory() );
	CACanorus::settings()->setDefaultSaveFormat( CAFileFormats::getType( uiDefaultSaveComboBox->currentText() ) );
	CAMainWin::uiSaveDialog->selectFilter( uiDefaultSaveComboBox->currentText() );
	CACanorus::settings()->setAutoRecoveryInterval( uiAutoRecoverySpinBox->value() );
	CACanorus::autoRecovery()->updateTimer();

	// Appearance Page
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

	CACanorus::settings()->writeSettings();
}

void CASettingsDialog::buildPreviewSheet() {
	_previewSheet = new CASheet( "", 0 );
	_previewSheet->addStaff();
	_previewSheet->staffAt(0)->addVoice( new CAVoice( "", _previewSheet->staffAt(0) ) );
	_previewSheet->staffAt(0)->voiceAt(0)->append( new CAClef( CAClef::Treble, _previewSheet->staffAt(0), 0 ) );
	_previewSheet->staffAt(0)->voiceAt(0)->append( new CATimeSignature( 2, 2, _previewSheet->staffAt(0), 0 ) );
	_previewSheet->addStaff();
	_previewSheet->staffAt(1)->addVoice( new CAVoice( "", _previewSheet->staffAt(0) ) );
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
		uiPreviewScoreViewPort->setBackgroundColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiBackgroundRevert_clicked(bool) {
	uiBackgroundColor->setPalette( QPalette(CASettings::DEFAULT_BACKGROUND_COLOR) );
	uiPreviewScoreViewPort->setBackgroundColor(CASettings::DEFAULT_BACKGROUND_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiForegroundColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiForegroundColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiForegroundColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setForegroundColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiForegroundRevert_clicked(bool) {
	uiForegroundColor->setPalette( QPalette(CASettings::DEFAULT_FOREGROUND_COLOR) );
	uiPreviewScoreViewPort->setForegroundColor(CASettings::DEFAULT_FOREGROUND_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiSelectionColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectionColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectionColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setSelectionColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiSelectionRevert_clicked(bool) {
	uiSelectionColor->setPalette( QPalette(CASettings::DEFAULT_SELECTION_COLOR) );
	uiPreviewScoreViewPort->setSelectionColor(CASettings::DEFAULT_SELECTION_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiSelectionAreaColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectionAreaColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectionAreaColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setSelectionAreaColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiSelectionAreaRevert_clicked(bool) {
	uiSelectionAreaColor->setPalette( QPalette(CASettings::DEFAULT_SELECTION_AREA_COLOR) );
	uiPreviewScoreViewPort->setSelectionAreaColor(CASettings::DEFAULT_SELECTION_AREA_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiSelectedContextColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiSelectedContextColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiSelectedContextColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setSelectedContextColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiSelectedContextRevert_clicked(bool) {
	uiSelectedContextColor->setPalette( QPalette(CASettings::DEFAULT_SELECTED_CONTEXT_COLOR) );
	uiPreviewScoreViewPort->setSelectedContextColor(CASettings::DEFAULT_SELECTED_CONTEXT_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiHiddenElementsColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiHiddenElementsColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiHiddenElementsColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setHiddenElementsColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiHiddenElementsRevert_clicked(bool) {
	uiHiddenElementsColor->setPalette( QPalette(CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR) );
	uiPreviewScoreViewPort->setHiddenElementsColor(CASettings::DEFAULT_HIDDEN_ELEMENTS_COLOR);
	uiPreviewScoreViewPort->repaint();
}

void CASettingsDialog::on_uiDisabledElementsColor_clicked(bool) {
	QColor c = QColor::fromRgba(QColorDialog::getRgba( uiDisabledElementsColor->palette().color(QPalette::Window).rgba(), 0, this ));
	if (c.isValid()) {
		uiDisabledElementsColor->setPalette( QPalette(c) );
		uiPreviewScoreViewPort->setDisabledElementsColor(c);
		uiPreviewScoreViewPort->repaint();
	}
}

void CASettingsDialog::on_uiDisabledElementsRevert_clicked(bool) {
	uiDisabledElementsColor->setPalette( QPalette(CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR) );
	uiPreviewScoreViewPort->setDisabledElementsColor(CASettings::DEFAULT_DISABLED_ELEMENTS_COLOR);
	uiPreviewScoreViewPort->repaint();
}
