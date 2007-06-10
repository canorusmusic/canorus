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
	CAUndoCommand( CASheet *sheet, QString text );
	CAUndoCommand( CADocument *document, QString text );
	virtual ~CAUndoCommand();
	virtual void undo();
	virtual void redo();
	
	static void undoSheet( CASheet *current, CASheet *newSheet );
	static void undoDocument( CADocument *current, CADocument *newDocument );
	
	inline CASheet *getUndoSheet() { return _undoSheet; }
	inline void setUndoSheet( CASheet *sheet ) { _undoSheet = sheet; }
	inline CADocument *getUndoDocument() { return _undoDocument; }
	inline void setUndoDocument( CADocument *doc ) { _undoDocument = doc; }	
	inline CASheet *getRedoSheet() { return _redoSheet; }
	inline void setRedoSheet( CASheet *sheet ) { _redoSheet = sheet; }
	inline CADocument *getRedoDocument() { return _redoDocument; }
	inline void setRedoDocument( CADocument *doc ) { _redoDocument = doc; }	
	
private:
	CASheet *_undoSheet;
	CADocument *_undoDocument;
	CASheet *_redoSheet;
	CADocument *_redoDocument;
};

#endif /* UNDO_H_ */
