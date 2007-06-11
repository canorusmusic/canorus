/*!
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef UNDO_H_
#define UNDO_H_

#include <QUndoCommand>

class CASheet;
class CADocument;

class CAUndoCommand : public QUndoCommand {
public:
	CAUndoCommand( CADocument *document, QString text );
	virtual ~CAUndoCommand();
	virtual void undo();
	virtual void redo();
	
	static void undoDocument( CADocument *current, CADocument *newDocument );
	
	inline CADocument *getUndoDocument() { return _undoDocument; }
	inline void setUndoDocument( CADocument *doc ) { _undoDocument = doc; }	
	inline CADocument *getRedoDocument() { return _redoDocument; }
	inline void setRedoDocument( CADocument *doc ) { _redoDocument = doc; }	
	
private:
	CADocument *_undoDocument;
	CADocument *_redoDocument;
};

#endif /* UNDO_H_ */
