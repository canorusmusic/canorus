/** @file staff.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef STAFF_H_
#define STAFF_H_

#include <QList>
#include <QString>

class QPainter;

#include "core/context.h"
#include "core/clef.h"

class CASheet;
class CAContext;
class CAVoice;
class CANote;

class CAStaff : public CAContext {
	public:
		CAStaff(CASheet *s, const QString name);
		
		inline int numberOfLines() { return _numberOfLines; }
		inline void setNumberOfLines(int val) { _numberOfLines = val; }
		void clear();
		
		/**
		 * Return the end of the last music element in the staff.
		 * 
		 * @return End of the last music element in the staff.
		 */
		int lastTimeEnd();
		
		/**
		 * Return the number of voices.
		 * 
		 * @return Number of voices.
		 */
		int voiceCount() { return _voiceList.size(); }
		
		/**
		 * Return voice with the specified index.
		 * 
		 * @return Pointer to the voice with the given index.
		 */
		CAVoice *voiceAt(int i) { return _voiceList[i]; }
		
		/**
		 * Look up for the voice with the given name and return it.
		 * 
		 * @param name The name of the voice being looked for.
		 * @return Pointer to the voice with the given name. If such a voice doesn't exist, return 0.
		 */
		CAVoice *voice(const QString name);
		
		/**
		 * Add a voice to the voice list.
		 * 
		 * @param voice Voice to be added.
		 */
		void addVoice(CAVoice *voice) { _voiceList << voice; }
		
		/**
		 * Insert a sign to the staff at certain time.
		 * This method inserts the sign to all voices!
		 * This is the difference between inserting a sign or a playable note - note is present in a single voice, sign in both.
		 * 
		 * @param sign Pointer to the already created CAMusElement object about to be added.
		 */
		void insertSign(CAMusElement *sign);
		
		/**
		 * Insert any sign (clef, siganutres, barline etc.) before the given music element.
		 * This method inserts the sign to all voices!
		 * This is the difference between inserting a sign or a playable note - note is present in a single voice, sign in both.
		 * 
		 * @param sign Pointer to the already created CAMusElement object about to be added.
		 * @param eltAfter The music element before which the sign should be inserted. The element should be present in all voices! If eltAfter is 0, append the sign to the voices.
		 * @return True, if a sign was inserted/appended, false if the eltAfter wasn't found.
		 */
		bool insertSignBefore(CAMusElement *sign, CAMusElement *eltAfter);
		
		/**
		 * Insert any sign (clef, siganutres, barline etc.) after the given music element.
		 * This method inserts the sign to all voices!
		 * This is the difference between inserting a sign or a playable note - note is present in a single voice, sign in all the voices.
		 * 
		 * @param sign Pointer to the already created CAMusElement object about to be added.
		 * @param eltBefore The music element after which the sign should be inserted. The element should be present in all voices! If eltBefore is 0, append the sign to the voices.
		 * @return True, if a sign was inserted/appended, false if the eltAfter wasn't found.
		 */
		bool insertSignAfter(CAMusElement *sign, CAMusElement *eltBefore);

		/**
		 * Remove the given music element from all the voices.
		 * 
		 * @param elt Pointer to the element to be removed.
		 * @param cleanup Delete the object and free memory as well. This is the default behaviour. If false, only remove the reference but not the object itself.
		 * @return True, if element was found and removed, false otherwise.
		 */
		bool removeMusElement(CAMusElement* elt, bool cleanup = true);
		
		CAMusElement *findNextMusElement(CAMusElement *elt);
		CAMusElement *findPrevMusElement(CAMusElement *elt);
		
		void setName(QString name) { _name = name; }
		QString name() { return _name; }
		
		void insertNote(CANote *note);
		
		/**
		 * Insert the given note to the already existing note and create a single-voice chord.
		 * 
		 * @param note Pointer to the note to be added.
		 * @param referenceNote Pointer to the note which the given note should be added to.
		 * @return True, if the reference note was found and a note was added, false otherwise.
		 */
		bool insertNoteToChord(CANote *note, CANote *referenceNote);
		
		/**
		 * Insert the note before the given music element.
		 * 
		 * @param note Pointer to the note being added.
		 * @param eltAfter Pointer to the next music element.
		 * @return True, if the next element was found and note was added before it, false otherwise.
		 */
		bool insertNoteBefore(CANote *note, CAMusElement *eltAfter);
		
	private:
		QList<CAVoice *> _voiceList;
		QString _name;
		
		int _numberOfLines;
};

#endif /*STAFF_H_*/
