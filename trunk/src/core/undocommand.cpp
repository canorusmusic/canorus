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
	   pass the current document to be saved for that action.
	3) If the action was successful, commit the command by calling CACanorus::pushUndoCommand().
	4) For undo/redo, simply call CACanorus::undoStack()->undo().
	5) When closing the document, also destroy the undo stack (which also destroys all its commands) by
	   calling CACanorus::deleteUndoStack().
*/

/*!
	Creates a new undo command.
	Internally, it clones the given document and set it as an undo document.
	The redo document is directly the passed document.
	When having multiple undo commands, you should take care of relinking the previous undo commmand's redo
	document to next command's undo document. This is usually done when pushing the command onto the stack.
*/
CAUndoCommand::CAUndoCommand( CADocument *document, QString text )
 : QUndoCommand( text ) {
	setUndoDocument( document->clone() );
	setRedoDocument( document );
}

CAUndoCommand::~CAUndoCommand() {
	CACanorus::updateLastUndoCommand( this );
	
	if ( getUndoDocument() && (!CACanorus::mainWinCount(getUndoDocument())) )
		delete getUndoDocument();
	
	// delete also redoDocument, if the last on the stack
	if ( getRedoDocument() && (!CACanorus::mainWinCount(getRedoDocument())) &&
	     (
	       CACanorus::undoStack(getUndoDocument()) && (!CACanorus::undoStack(getUndoDocument())->count()) ||
	       CACanorus::undoStack(getRedoDocument()) && (!CACanorus::undoStack(getRedoDocument())->count())
	     )
	   )
		delete getRedoDocument();
}

void CAUndoCommand::undo() {
	getUndoDocument()->setTimeEdited( getRedoDocument()->timeEdited() ); // time edited might get lost when saving the document and undoing right after
	getUndoDocument()->setFileName( getRedoDocument()->fileName() );
	CAUndoCommand::undoDocument( getRedoDocument(), getUndoDocument() );
}

void CAUndoCommand::redo() {
	getRedoDocument()->setTimeEdited( getUndoDocument()->timeEdited() ); // time edited might get lost when saving the document and redoing right after
	getRedoDocument()->setFileName( getUndoDocument()->fileName() );
	CAUndoCommand::undoDocument( getUndoDocument(), getRedoDocument() );
}

/*!
	Creates the actual undo (switches the pointers of the document) and updates the GUI.
	The updating GUI part is quite complicated as it has to update all viewports showing
	the right structure and sub-structure (eg. voice with the same index in the new document).
*/
void CAUndoCommand::undoDocument( CADocument *current, CADocument *newDocument ) {
	QHash< CASheet*, CASheet* > sheetMap;       // map old->new sheets
	QHash< CAContext*, CAContext* > contextMap; // map old->new contexts
	QHash< CAVoice*, CAVoice* > voiceMap;       // map old->new voices
	bool rebuildNeeded=false;
	
	for (int i=0; i<newDocument->sheetCount() && i<current->sheetCount(); i++) {
		sheetMap[current->sheetAt(i)] = newDocument->sheetAt(i);
		for (int j=0; j<newDocument->sheetAt(i)->contextCount() && j<current->sheetAt(i)->contextCount(); j++) {
			contextMap[current->sheetAt(i)->contextAt(j)] = newDocument->sheetAt(i)->contextAt(j);
		}
		for (int j=0; j<newDocument->sheetAt(i)->voiceCount() && j<current->sheetAt(i)->voiceCount(); j++) {
			voiceMap[current->sheetAt(i)->voiceAt(j)] = newDocument->sheetAt(i)->voiceAt(j);
		}
	}
	
	QList<CAMainWin*> mainWinList = CACanorus::findMainWin( current );
	if (newDocument->sheetCount() != current->sheetCount()) {
		for (int i=0; i<mainWinList.size(); i++) {
			mainWinList[i]->setDocument( newDocument );
		}
		rebuildNeeded=true;
	} else {
		for (int i=0; i<mainWinList.size(); i++) {
			QList<CAViewPort*> viewPortList = mainWinList[i]->viewPortList();
			for (int j=0; j<viewPortList.size(); j++) {
				switch (viewPortList[j]->viewPortType()) {
					case CAViewPort::ScoreViewPort: {
						CAScoreViewPort *sv = static_cast<CAScoreViewPort*>(viewPortList[j]);
						sv->setSheet( sheetMap[sv->sheet()] );
						
						break;
					}
					case CAViewPort::SourceViewPort: {
						CASourceViewPort *sv = static_cast<CASourceViewPort*>(viewPortList[j]);
						if ( sv->voice() ) {
							if (voiceMap.contains(sv->voice()))
								// set the new voice
								sv->setVoice( voiceMap[sv->voice()] );
							else
								// or close the viewport if not available
								delete sv;
						}
						if ( sv->lyricsContext() ) {
							if ( contextMap.contains(sv->lyricsContext()) &&
							     contextMap[sv->lyricsContext()]->contextType()==CAContext::LyricsContext )
								// set the new lyrics context
								sv->setLyricsContext( static_cast<CALyricsContext*>( contextMap[sv->lyricsContext()] ) );
							else
								// or close the viewport if not available
								delete sv;
						}
						if ( sv->document() ) {
							// set the new document
							sv->setDocument( newDocument );
						}
						break;
					}
				}
			}
			
			mainWinList[i]->setDocument( newDocument );
		}
	}
	
	if (mainWinList.size()) {
		CACanorus::setUndoStack( newDocument, CACanorus::undoStack(current) );
		CACanorus::removeUndoStack( current );
	}
	
	if (rebuildNeeded)
		CACanorus::rebuildUI( newDocument );		
}
