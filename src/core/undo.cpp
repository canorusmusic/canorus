/*!
	Copyright (c) 2007-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "core/undo.h"
#include "core/undocommand.h"
#include "score/document.h" // needed for setting the modified flag
#include <iostream>

/*!
	\class CAUndo
	\brief Undo/Redo support

	This class implements undo and redo Canorus functionality.

    This singleton object is created upon Canorus startup and is accessed via CACanorus::undo() getter.
    In essence there is one undo stack for each opened file. When changes to the document are made, the
    undo action is created and the current document is cloned. This copy is then used to replace the
    active document in case of undo. A mapping of any document copy out there to the undo stack it
    belongs to is stored in _undoStack.

    Each main window has one undo stack or a shared undo stack, if multiple windows represent the same
    document.

    Undo stack contains one or more undo commands called upon undo/redo events (each undo command then
    makes one change to the document). _undoIndex stores the current index of the undo command for each
    undo stack (i.e. the command which gets called next time the user presses the Undo button).

	Usage of undo/redo:
	1) Create undo stack when creating/opening a new document by calling CAUndo::createUndoStack()
	2) Before each action (insertion, removal, editing of elements), call CAUndo::createUndoCommand() and
	   pass the current document to be saved for that action.
	3) If the action was successful, commit the command by calling CAUndo::pushUndoCommand(). If not, do
	   nothing - non-pushed commands will get deleted when createUndoCommand() will be issued the next time.
	4) For undo/redo, simply call CAUndo::undoStack()->undo().
	5) When destroying the document, also destroy the undo stack (which also destroys all its commands) by
	   calling CAUndo::deleteUndoStack(). This is not done automatically because CADocument is part of the
	   data model and CAUndo part of the controller.

    If the user already created its own instance of the new document without calling CAUndo::createUndoCommand()
    (e.g. when parsing the source-view of the whole document), he should use CAUndo::replaceDocument().

	\sa CAUndoCommand
*/

CAUndo::CAUndo()
{
    _undoCommand = nullptr;
}

CAUndo::~CAUndo()
{
}

/*!
	Creates a new undoStack for the given document.
	This should be called at the beginning when the first main window for the given document is created.
*/
void CAUndo::createUndoStack(CADocument* doc)
{
    _undoStack[doc] = new QList<CAUndoCommand*>;
    undoIndex(doc) = -1;
}

/*!
	Undoes the last change made to document.
	This method already calls CAUndoCommand::undo().
*/
void CAUndo::undo(CADocument* doc)
{
    if (_undoStack[doc] && canUndo(doc)) {
        _undoStack[doc]->at(undoIndex(doc))->undo();
        undoIndex(doc)--;
    }
}

/*!
	Redoes the last change made to document.
	This method already calls CAUndoCommand::redo().
*/
void CAUndo::redo(CADocument* doc)
{
    if (_undoStack[doc] && canRedo(doc)) {
        _undoStack[doc]->at(undoIndex(doc) + 1)->redo();
        undoIndex(doc)++;
    }
}

/*!
	Deletes the undoStack object for the given document.
	This should be called at the end where no main windows are pointing to the given document anymore.
*/
void CAUndo::deleteUndoStack(CADocument* doc)
{
    clearUndoCommand();
    QList<CAUndoCommand*>* stack = undoStack(doc);
    while (!stack->isEmpty()) {
        delete stack->first();
        stack->takeFirst();
    }
    delete stack;

    QList<CADocument*> keys = _undoStack.keys(stack);
    for (int i = 0; i < keys.size(); i++)
        removeUndoStack(keys[i]);
}

/*!
	Call this to add an undo command (created by createUndoCommand()) to the stack.
	Undo commands *after* the currently active command will be deleted.
	undoIndex is updated to the size of the stack - 1.

	\warning This function is not thread-safe. createUndoCommand() and pushUndoCommand() should be called
	from the same thread and main window.
*/
void CAUndo::pushUndoCommand()
{
    if (!_undoCommand || !_undoCommand->getRedoDocument() || !_undoCommand->getUndoDocument())
        return;

    CADocument* d = _undoCommand->getRedoDocument();

    _undoCommand->getUndoDocument()->setModified(true);
    _undoCommand->getRedoDocument()->setModified(true);

    QList<CAUndoCommand*>* s = _undoStack[d];
    CAUndoCommand* prevUndoCommand = (undoIndex(d) < s->size() && undoIndex(d) >= 0 ? s->at(undoIndex(d)) : nullptr);

    // delete undo commands after the new one, if any (eg. 3x changes, 2x undo, 1x change => removes last 2 undos when making a change)
    for (int i = undoIndex(d) + 1; i < s->size();) {
        _undoStack.remove(s->at(i)->getRedoDocument());
        delete s->at(i);
        s->removeAt(i);
    }

    if (prevUndoCommand) {
        if (_undoCommand->getRedoDocument() && prevUndoCommand->getRedoDocument())
            prevUndoCommand->setRedoDocument(_undoCommand->getUndoDocument());
    }

    s->append(_undoCommand); // push the command on stack
    _undoStack[_undoCommand->getUndoDocument()] = s;
    undoIndex(d) = _undoStack[d]->size() - 1;
    _undoCommand = nullptr;
}

/*!
	Returns True, if changes to the current document have been made and undo
	is possible. False otherwise.

	\sa canRedo()
*/
bool CAUndo::canUndo(CADocument* d)
{
    if (_undoStack[d] && _undoStack[d]->size() && undoIndex(d) != -1)
        return true;
    else
        return false;
}

/*!
	Returns True, if changes to the current document have been undone at least
	once and redo is possible. False otherwise.

	\sa canUndo()
*/
bool CAUndo::canRedo(CADocument* d)
{
    if (_undoStack[d] && _undoStack[d]->size() && undoIndex(d) != (_undoStack[d]->size() - 1))
        return true;
    else
        return false;
}

/*!
	Destroys the undo command if decided not to be put on the stack.
	Does nothing if undo command is null.
*/
void CAUndo::clearUndoCommand()
{
    if (_undoCommand) {
        delete _undoCommand;
        _undoCommand = nullptr;
    }
}

/*!
	Creates an undo command which is later put on the stack.
	This function is usually called when making changes to the document in the score -
	all changes ranging from creation/removal of sheets and editing document properties.

	\warning This function is not thread-safe. createUndoCommand() and pushUndoCommand() should be called from the same thread.
*/
void CAUndo::createUndoCommand(CADocument* d, QString text)
{
    clearUndoCommand();
    _undoCommand = new CAUndoCommand(d, text);
}

/*!
    Replace the document pointer to an undo stack.
    This function is called when the document is rebuilt, e.g. when a CanorusML
    view commits changes and the old document should be forgotten.
*/
void CAUndo::replaceDocument(CADocument* oldDoc, CADocument* newDoc)
{
    clearUndoCommand();
    QList<CAUndoCommand*>* stack = _undoStack[oldDoc];

    CAUndoCommand* prevUndoCommand = (undoIndex(oldDoc) < stack->size() && undoIndex(oldDoc) >= 0 ? stack->at(undoIndex(oldDoc)) : nullptr);
    if (prevUndoCommand) {
        if (prevUndoCommand->getRedoDocument() == oldDoc)
            prevUndoCommand->setRedoDocument(newDoc);
    }

    _undoStack.remove(oldDoc);
    _undoStack[newDoc] = stack;
}

/*!
	Gathers a list of all redo and undo instances of the given document \a d.
	This function is usually called when adding/removing a resource to/from a document.
	Because resources are not undoable, they should be the same for all the documents.
 */
QList<CADocument*> CAUndo::getAllDocuments(CADocument* d)
{
    QList<CADocument*> documents;

    QList<CAUndoCommand*>* undoCommands = _undoStack[d];

    if (undoCommands && undoCommands->size()) {
        for (int i = 0; i < undoCommands->size(); i++) {
            documents << undoCommands->at(i)->getUndoDocument();
        }

        if (undoCommands->size() > 0) {
            documents << undoCommands->at(undoCommands->size() - 1)->getRedoDocument();
        }
    } else {
        documents << d;
    }

    return documents;
}
