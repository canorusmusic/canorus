/*!
    Copyright (c) 2015, Reinhard Katzmann, Matevž Jekovec, Canorus development team
    All Rights Reserved. See AUTHORS for a complete list of authors.

    Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QDebug>

#include "mainwin.h"
#include "widgets/undotoolbutton.h"
#include "actionstorage.h"
#include "singleaction.h"
#include "core/actiondelegate.h"

CAActionStorage::CAActionStorage() :
    _actionDelegate(0)
{
    _actionWidget.actions().clear();
}

CAActionStorage::~CAActionStorage()
{
    _actionWidget.actions().clear();
    if( 0 != _actionDelegate )
        delete _actionDelegate;
    _actionDelegate = 0;
}

void CAActionStorage::storeActionsFromMainWindow(CAMainWin &mainWin)
{
    storeAction(mainWin.uiQuit);
    storeAction(mainWin.uiNewDocument);
    storeAction(mainWin.uiOpenDocument);
    storeAction(mainWin.uiSaveDocument);
    storeAction(mainWin.uiSaveDocumentAs);
    storeAction(mainWin.uiCloseDocument);
    storeAction(mainWin.uiImportDocument);
    storeAction(mainWin.uiExportDocument);
    storeAction(mainWin.uiPrintPreview);
    storeAction(mainWin.uiPrint);
    storeAction(mainWin.uiCopy);
    storeAction(mainWin.uiCut);
    storeAction(mainWin.uiPaste);
    storeAction(mainWin.uiSelectAll);
    storeAction(mainWin.uiInvertSelection);
    storeAction(mainWin.uiZoomToWidth);
    storeAction(mainWin.uiGotoBar);
    storeAction(mainWin.uiShowStatusBar);
    storeAction(mainWin.uiFullscreen);
    storeAction(mainWin.uiInsertTimeSig);
    storeAction(mainWin.uiInsertKeySig);
    storeAction(mainWin.uiInsertBarline);
    storeAction(mainWin.uiNoteCount);
    storeAction(mainWin.uiSettings);
    storeAction(mainWin.uiUsersGuide);
    storeAction(mainWin.uiWhatsThis);
    storeAction(mainWin.uiTipOfTheDay);
    storeAction(mainWin.uiAboutCanorus);
    storeAction(mainWin.uiAboutQt);
    storeAction(mainWin.uiSplitHorizontally);
    storeAction(mainWin.uiSplitVertically);
    storeAction(mainWin.uiCloseCurrentView);
    storeAction(mainWin.uiUnsplitAll);
    storeAction(mainWin.uiNewView);
    storeAction(mainWin.uiNewSheet);
    storeAction(mainWin.uiNewContext);
    storeAction(mainWin.uiPlayFromSelection);
    storeAction(mainWin.uiAnimatedScroll);
    storeAction(mainWin.uiLockScrollPlayback);
    storeAction(mainWin.uiZoomToHeight);
    storeAction(mainWin.uiZoomToFit);
    storeAction(mainWin.uiZoomToSelection);
    storeAction(mainWin.uiNewWindow);
    storeAction(mainWin.uiCustomZoom);
    storeAction(mainWin.uiScoreView);
    storeAction(mainWin.uiLilyPondSource);
    storeAction(mainWin.uiCanorusMLSource);
    storeAction(mainWin.uiNewVoice);
    storeAction(mainWin.uiDocumentProperties);
    storeAction(mainWin.uiPrintDirectly);
    storeAction(mainWin.uiExportToPdf);
    storeAction(mainWin.uiNewDocumentWizard);
    storeAction(mainWin.uiInsertPlayable);
    storeAction(mainWin.uiInsertFM);
    storeAction(mainWin.uiInsertClef);
    storeAction(mainWin.uiSelectMode);
    storeAction(mainWin.uiRemoveVoice);
    storeAction(mainWin.uiVoiceProperties);
    storeAction(mainWin.uiAccsVisible);
    storeAction(mainWin.uiHiddenRest);
    storeAction(mainWin.uiRemoveContext);
    storeAction(mainWin.uiContextProperties);
    storeAction(mainWin.uiRemoveSheet);
    storeAction(mainWin.uiSheetProperties);
    storeAction(mainWin.uiTranspose);
    storeAction(mainWin.uiFMEllipse);
    storeAction(mainWin.uiInsertSyllable);
    storeAction(mainWin.uiEditMode);
    storeAction(mainWin.uiInsertMark);
    storeAction(mainWin.uiInsertArticulation);
    storeAction(mainWin.actionRecent_documents);
    storeAction(mainWin.uiMidiRecorder);
    storeAction(mainWin.uiResourceView);
    storeAction(mainWin.uiInsertFBM);
    storeAction(mainWin.uiShowRuler);
    storeAction(mainWin.uiUndo->defaultAction());
    storeAction(mainWin.uiRedo->defaultAction());
    _actionDelegate = new CAActionDelegate(&mainWin);
}

void CAActionStorage::storeAction(QAction *action)
{
    _actionWidget.addAction(action);
    qWarning() << "Storage: Added new action, stored " << _actionWidget.actions().size();
}

void CAActionStorage::addWinActions()
{
   _actionDelegate->addWinActions(_actionWidget);
   _actionDelegate->updateMainWinActions();
}
