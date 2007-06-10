/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/undocommand.h"
#include "core/canorus.h"
#include "core/sheet.h"
#include "core/document.h"
#include "core/lyricscontext.h"
#include "core/voice.h"
#include "widgets/scoreviewport.h"
#include "widgets/sourceviewport.h"

/*!
	\class CAUndoCommand
	\brief Undo/Redo functionality
	
	This class introduces undo and redo functionality for Canorus.
	
	It inherits QUndoCommand and is usually stored inside QUndoStack.
	
	Create this class by passing it a pointer to a sheet or document which the state should be saved for
	the future use. When called undo() or redo() (usually called by QUndoStack) all the documents and
	sheets currently opened are updated pointing to the previous (undone) or next (redone) states of the
	structures.
	
	Usage of undo/redo:
	1) Create undo stack when creating/opening a new document by calling CACanorus::createUndoStack()
	2) Before each action (insertion, removal, editing of elements), call CACanorus::createUndoCommand() and
	   pass the appropriate structure needed to be saved for that action.
	3) If the action was successful, commit the command by calling CACanorus::pushUndoCommand().
	4) For undo/redo, simply call CACanorus::undoStack()->undo().
	5) When closing the document, also destroy the undo stack (which also destroys all its commands) by
	   calling CACanorus::deleteUndoStack().
*/

CAUndoCommand::CAUndoCommand( CASheet *sheet, QString text )
 : QUndoCommand( text ) {
	setUndoSheet( sheet->clone() );
	setRedoSheet( sheet );
	
	setUndoDocument( 0 );
	setRedoDocument( 0 );
}

CAUndoCommand::CAUndoCommand( CADocument *document, QString text )
 : QUndoCommand( text ) {
	setUndoSheet( 0 );
	setRedoSheet( 0 );
	
	setUndoDocument( document->clone() );
	setRedoDocument( document );
}

CAUndoCommand::~CAUndoCommand() {
	CACanorus::updateLastUndoCommand( this );
	
	if ( getUndoSheet() && (getUndoSheet()->document()->sheetList().indexOf(getUndoSheet())==-1) )
		delete getUndoSheet();
	
	// delete also undoSheet, if the last on the stack
	if ( getRedoSheet() && !(CACanorus::undoStack(getRedoSheet()->document())->count()) &&
	     (getRedoSheet()->document()->sheetList().indexOf(getRedoSheet())==-1) )
		delete getRedoSheet();
	
	if ( getUndoDocument() && (!CACanorus::mainWinCount(getUndoDocument())) )
		delete getUndoDocument();

	// delete also undoDocument, if the last on the stack
	if ( getRedoDocument() && (!CACanorus::undoStack(getRedoDocument())->count()) &&
	    (!CACanorus::mainWinCount(getRedoDocument())) )
		delete getRedoDocument();
}

void CAUndoCommand::undo() {
	if ( getRedoSheet() ) {
		CAUndoCommand::undoSheet( getRedoSheet(), getUndoSheet() );
	} else if ( getRedoDocument() ) {
		CAUndoCommand::undoDocument( getRedoDocument(), getUndoDocument() );
	}
}

void CAUndoCommand::redo() {
	if ( getRedoSheet() ) {
		CAUndoCommand::undoSheet( getUndoSheet(), getRedoSheet() );
	} else if ( getRedoDocument() ) {
		CAUndoCommand::undoDocument( getUndoDocument(), getRedoDocument() );
	}
}

void CAUndoCommand::undoSheet( CASheet *current, CASheet *newSheet ) {
	CADocument *doc = current->document();
	// set document's sheet to point to cloned sheet
	for (int i=0; i<doc->sheetCount(); i++) {
		if (doc->sheetAt(i)==current) {
			doc->setSheetAt(i, newSheet);
			break;
		}
	}
	
	// update viewports to point to the new sheet and its sub-elements (contexts, voices etc.)
	QList<CAMainWin*> mainWinList = CACanorus::findMainWin( doc );
	for (int i=0; i<mainWinList.size(); i++) {
		QList<CAViewPort*> viewPortList = mainWinList[i]->viewPortList();
		for (int j=0; j<viewPortList.size(); j++) {
			switch (viewPortList[j]->viewPortType()) {
				case CAViewPort::ScoreViewPort:
					if (static_cast<CAScoreViewPort*>(viewPortList[j])->sheet()==current)
						static_cast<CAScoreViewPort*>(viewPortList[j])->setSheet( newSheet );
					break;
				case CAViewPort::SourceViewPort: {
					CASourceViewPort *sv = static_cast<CASourceViewPort*>(viewPortList[j]);
					if ( sv->voice() &&
					     sv->voice()->staff()->sheet()==current ) {
						if (current->voiceList().indexOf(sv->voice()) < newSheet->voiceList().size())
							// set the new voice
							sv->setVoice( newSheet->voiceList().at(current->voiceList().indexOf(sv->voice())) );
						else
							// or close the viewport if not available
							delete sv;
					}
					if ( sv->lyricsContext() &&
					     sv->lyricsContext()->sheet()==current ) {
						if (current->contextList().indexOf(sv->lyricsContext()) < newSheet->contextList().size() &&
						    newSheet->contextList().at(current->contextList().indexOf(sv->lyricsContext()))->contextType()==CAContext::LyricsContext )
							// set the new lyrics context
							sv->setLyricsContext( static_cast<CALyricsContext*>(newSheet->contextList().at(current->contextList().indexOf(sv->lyricsContext()))) );
						else
							// or close the viewport if not available
							delete sv;
					}
					
					break;
				}
			}
		}
	}
	
}

void CAUndoCommand::undoDocument( CADocument *current, CADocument *newDocument ) {
	QList<CAMainWin*> mainWinList = CACanorus::findMainWin( current );
	for (int i=0; i<mainWinList.size(); i++) {
		QList<CAViewPort*> viewPortList = mainWinList[i]->viewPortList();
		for (int j=0; j<viewPortList.size(); j++) {
			switch (viewPortList[j]->viewPortType()) {
				case CAViewPort::ScoreViewPort:
					break;
				case CAViewPort::SourceViewPort: {
					CASourceViewPort *sv = static_cast<CASourceViewPort*>(viewPortList[j]);
					if ( sv->document() &&
					     sv->document()==current ) {
						sv->setDocument( newDocument );
					}
					break;
				}
			}
		}
		
		mainWinList[i]->setDocument( newDocument );
	}
	
	if (mainWinList.size()) {
		CACanorus::setUndoStack( newDocument, CACanorus::undoStack(current) );
		CACanorus::removeUndoStack( current );
	}	
}
