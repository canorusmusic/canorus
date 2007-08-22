/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef UNDO_H_
#define UNDO_H_

class CAUndoCommand;
class CADocument;

#include <QHash>
#include <QList>

class CAUndo {
public:
	CAUndo();
	virtual ~CAUndo();
	
	bool canUndo( CADocument* );
	bool canRedo( CADocument* );
	void undo( CADocument* );
	void redo( CADocument* );
	inline bool containsUndoStack( CADocument *d ) { return _undoStack.contains(d); }
	void createUndoStack( CADocument *d );
	inline QList<CAUndoCommand*> *undoStack( CADocument* d ) { return _undoStack[d]; }
	inline int& undoIndex( CADocument* d ) { return _undoIndex[undoStack(d)]; }
	inline void removeUndoStack( CADocument *d ) { _undoStack.remove(d); }
	void deleteUndoStack( CADocument *doc );
	void createUndoCommand( CADocument *d, QString text );
	void pushUndoCommand();
	CAUndoCommand *undoCommand( CADocument *d );
	CAUndoCommand *redoCommand( CADocument *d );
	void updateLastUndoCommand( CAUndoCommand *c );

private:
	void clearUndoCommand();
	CAUndoCommand *_undoCommand; // current undo command created to be put on the undo stack
	
	QHash< CADocument*, QList<CAUndoCommand*>* > _undoStack;
	QHash< QList<CAUndoCommand*>*, int >         _undoIndex;
	
};

#endif /* UNDO_H_ */
