/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "core/undo.h"
#include "core/undocommand.h"
#include "canorus.h"
#include "score/sheet.h"
#include "score/document.h"
#include "score/lyricscontext.h"
#include "score/voice.h"
#include "score/resource.h"
#include "widgets/scoreview.h"
#include "widgets/sourceview.h"

/*!
	\class CAUndoCommand
	\brief Internal Undo/Redo command

	This class implements undo and redo action.

	It inherits QUndoCommand and is usually stored inside QUndoStack or a list.

	Create this object by passing it a pointer to a document which the state should be saved for
	the future use. When called undo() or redo() (usually called by CAUndo class) all the documents and
	sheets currently opened are updated pointing to the previous (undone) or next (redone) states of the
	structures.

	\warning You should never directly access this class. Use CAUndo instead.

	\sa CAUndo
*/

/*!
	Creates a new undo command.
	Internally, it clones the given document and sets it as an undo document.
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
	if ( getUndoDocument() && (!CACanorus::mainWinCount(getUndoDocument())) )
		delete getUndoDocument();

	// delete also redoDocument, if the last on the stack
	if ( getRedoDocument() && !CACanorus::mainWinCount(getRedoDocument()) &&
	     CACanorus::undo()->undoStack(getRedoDocument()) &&
	     CACanorus::undo()->undoStack(getRedoDocument())->indexOf(this)==CACanorus::undo()->undoStack(getRedoDocument())->count()-1
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
	The updating GUI part is quite complicated as it has to update all views showing
	the right structure and sub-structure (eg. voice with the same index in the new document).
*/
void CAUndoCommand::undoDocument( CADocument *current, CADocument *newDocument ) {
	QHash< CASheet*, CASheet* > sheetMap;       // map old->new sheets
	QHash< CAContext*, CAContext* > contextMap; // map old->new contexts
	QHash< CAVoice*, CAVoice* > voiceMap;       // map old->new voices
	bool rebuildNeeded=false;

	for (int i=0; i<newDocument->sheetList().size() && i<current->sheetList().size(); i++) {
		sheetMap[current->sheetList()[i]] = newDocument->sheetList()[i];
		for (int j=0; j<newDocument->sheetList()[i]->contextList().size() && j<current->sheetList()[i]->contextList().size(); j++) {
			contextMap[current->sheetList()[i]->contextList()[j]] = newDocument->sheetList()[i]->contextList()[j];
		}
		for (int j=0; j<newDocument->sheetList()[i]->voiceList().size() && j<current->sheetList()[i]->voiceList().size(); j++) {
			voiceMap[current->sheetList()[i]->voiceList()[j]] = newDocument->sheetList()[i]->voiceList()[j];
		}
	}

	QList<CAMainWin*> mainWinList = CACanorus::findMainWin( current );
	if (newDocument->sheetList().size() != current->sheetList().size()) {
		for (int i=0; i<mainWinList.size(); i++) {
			mainWinList[i]->setDocument( newDocument );
		}
		rebuildNeeded=true;
	} else {
		// rebuild UI and replace sheets with new sheets
		// TODO: This should be moved to the UI module!
		for (int i=0; i<mainWinList.size(); i++) {
			QList<CAView*> viewList = mainWinList[i]->viewList();
			for (int j=0; j<viewList.size(); j++) {
				switch (viewList[j]->viewType()) {
					case CAView::ScoreView: {
						CAScoreView *sv = static_cast<CAScoreView*>(viewList[j]);
						sv->setSheet( sheetMap[sv->sheet()] );

						break;
					}
					case CAView::SourceView: {
						CASourceView *sv = static_cast<CASourceView*>(viewList[j]);
						if ( sv->voice() ) {
							if (voiceMap.contains(sv->voice()))
								// set the new voice
								sv->setVoice( voiceMap[sv->voice()] );
							else
								// or close the view if not available
								delete sv;
						}
						if ( sv->lyricsContext() ) {
							if ( contextMap.contains(sv->lyricsContext()) &&
							     contextMap[sv->lyricsContext()]->contextType()==CAContext::LyricsContext )
								// set the new lyrics context
								sv->setLyricsContext( static_cast<CALyricsContext*>( contextMap[sv->lyricsContext()] ) );
							else
								// or close the view if not available
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

	// update resources
	for (int i=0; i<current->resourceList().size(); i++) {
		current->resourceList()[i]->setDocument(newDocument);
	}

	if (rebuildNeeded)
		CACanorus::rebuildUI( newDocument );
}
