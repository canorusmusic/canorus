/** @file core/voice.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef VOICE_H_
#define VOICE_H_

#include <QList>

#include "core/muselement.h"
#include "core/note.h"

class CAStaff;
class CAClef;

class CAVoice {
	public:
		CAVoice(CAStaff *staff, const QString name);
		inline CAStaff *staff() { return _staff; }
		void clear();
		
		/**
		 * Insert the music element before the first element which startTime is equal or greater to the given element's one.
		 * WARNING: If you're inserting signs like barlines, clefs, time signatures etc. use CAStaff::insertSign(), because the mentioned signs MUST be present in *all* voices. Use manually this method only if you know what you're doing. -Matevz
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 * @param updateTimes Should the following elements' start times be increased by the inserted note length. This is false when adding a note to a chord.
		 */
		void insertMusElement(CAMusElement *elt, bool updateTimes=true);
		
		/**
		 * Insert the CAMusElement right before the given CAMusElement.
		 * WARNING: If you're inserting signs like barlines, clefs, time signatures etc. use CAStaff::insertSign(), because the mentioned signs MUST be present in *all* voices. Use manually this method only if you know what you're doing. -Matevz
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 * @param eltAfter Pointer to CAMusElement the given element should be inserted before. If eltAfter is 0, append the music element to the voice.
		 * @param updateTimes Should the following elements' start times be increased by the inserted note length. This is false when adding a note to a chord. 
		 * @param force If the eltBefore isn't found, find a nearest time equivalent in the voice and place it before it.
		 * @return True, if eltAfter was found and the elt was inserted/appended, otherwise false.
		 */
		bool insertMusElementBefore(CAMusElement *elt, CAMusElement *eltAfter, bool updateTimes = true, bool force=false);
		
		/**
		 * Insert the CAMusElement right after the given CAMusElement.
		 * WARNING: If you're inserting signs like barlines, clefs, time signatures etc. use CAStaff::insertSign(), because the mentioned signs MUST be present in *all* voices. Use manually this method only if you know what you're doing. -Matevz
		 * 
		 * @param elt Pointer to CAMusElement to be inserted.
		 * @param eltBefore Pointer to CAMusElement the given element should be inserted after. If eltBefore is 0, append the music element to the voice.
		 * @param updateTimes Should the following elements' start times be increased by the inserted note length. This is false when adding a note to a chord.
		 * @param force If the eltBefore isn't found, find a nearest time equivalent in the voice and place it before it.
		 * @return True, if eltAfter was found and the elt was inserted/appended, otherwise false.
		 */
		bool insertMusElementAfter(CAMusElement *elt, CAMusElement *eltBefore, bool updateTimes = true, bool force=false);
		
		/**
		 * Add a music element to the end
		 * 
		 * @param elt Pointer to the music element to be added.
		 * @return True, if music element as successfully added, False otherwise.
		 */
		bool appendMusElement(CAMusElement *elt) { return insertMusElementAfter(elt, 0, false, false); }
		
		bool prependMusElement(CAMusElement *elt) { if (_musElementList.size()) return insertMusElementBefore(elt, _musElementList.front(), true, false); else return appendMusElement(elt); }
		
		/**
		 * Add a note to an already existing chord or a single note and create a chord out of it.
		 * Notes in a chord always need to be sorted by the pitch falling.
		 * 
		 * @param note Note to be added.
		 * @param referenceNote Already existing chord or note.
		 * @return True, if a referenceNote was found and a note was added, False otherwise.
		 */
		bool addNoteToChord(CANote *note, CANote *referenceNote);
		
		/**
		 * Remove the given music element from the voice.
		 * This method doesn't delete its contents, but only the pointer.
		 * 
		 * @param elt Pointer to the element to be removed.
		 * @return True, if the element was found and removed, false otherwise.
		 */
		bool removeElement(CAMusElement *elt);
		
		int voiceNumber() { return _voiceNumber; }
		bool isFirstVoice() { return !_voiceNumber; }
		void setVoiceNumber(int idx) { _voiceNumber = idx; }
		
		int musElementCount() { return _musElementList.count(); }
		CAMusElement *musElementAt(int i) { return _musElementList[i]; }
		int indexOf(CAMusElement *elt) { return _musElementList.indexOf(elt); }
		bool contains(CAMusElement *elt) { return _musElementList.contains(elt); }
		
		/**
		 * Return a list of pointers to actual music elements which have the given startTime and are of given type.
		 * 
		 * @param type Search for the music elements of the given type only.
		 * @param startTime Search for the music elements with the given start time only.
		 * @return List of pointers to music elements with the given startTime and type.
		 */
		QList<CAMusElement*> getEltByType(CAMusElement::CAMusElementType type, int startTime);
		
		/**
		 * Return true, if this voice contains a note with the given pitch and the given startTime.
		 * This is useful when inserting a note and there needs to be determined if a user is adding a note to a chord, but the note is already there.
		 * 
		 * @param pitch Note pitch (only the level, accidentals don't matter).
		 * @param startTime Note start time.
		 * @return True, if such a note was found, False otherwise.
		 */
		bool containsPitch(int pitch, int startTime);
		
		QList<CAMusElement*> *musElementList() { return &_musElementList; }	///Return the list of music elements
		int lastTimeEnd() { return (_musElementList.size()?_musElementList.back()->timeEnd():0); }
		int lastTimeStart() { return (_musElementList.size()?_musElementList.back()->timeStart():0); }
		CAMusElement *lastMusElement() { return _musElementList.back(); }
		CAMusElement *eltBefore(CAMusElement *elt);
		CAMusElement *eltAfter(CAMusElement *elt);
		
		/**
		 * Return the pitch of the last note in the voice (default) or of the first note in the last chord.
		 * If there is a clef after the last notes, return the clef's center pitch.
		 * 
		 * @param inChord Return the pitch of the first note in the last chord.
		 * @return Note pitch.
		 */
		int lastNotePitch(bool inChord=false);
		
		CAPlayable* lastPlayableElt();
		
		CAClef *getClef(CAMusElement *elt);
		QList<CAPlayable*> getChord(int time);	/// Return a list of notes or a rest in the given time slice (chord)
		
		const QString name() { return _name; }
		unsigned char midiChannel() { return _midiChannel; }
		unsigned char midiProgram() { return _midiProgram; }
	
		void setName(const QString name) { _name = name; }
		void setMidiChannel(const unsigned char ch) { _midiChannel = ch; }
		void setMidiProgram(const unsigned char program) { _midiProgram = program; }
		
		/**
		 * Update the musElements timeStarts from idx-th and on, adding the length from every one.
		 * Called when an element is deleted or inserted and startTimes after it need to be updated.
		 * If length is not set, take the element at idx as the inserted one and update times after it.
		 * 
		 * @param elt Element where the update should begin, if length is not 0. Or right after it, if length is 0.
		 * @param length Length which should be substracted by the timeStarts.
		 * @return True, if element was found and update was made, False otherwise.
		 */
		bool updateTimes(CAMusElement *elt, int length=0);
		
		/**
		 * Update the musElements timeStarts from idx+1-th and on, adding the length from every one.
		 * Called when an element is deleted or inserted and startTimes after it need to be updated.
		 * If length is not set, take the element at idx as the inserted one and update times after it.
		 * 
		 * @param eltBefore The previous element of where the update should begin, if length is not 0. Or right after it, if length is 0.
		 * @param length Length which should be substracted by the timeStarts.
		 * @return True, if element was found and update was made, False otherwise.
		 */
		bool updateTimesAfter(CAMusElement *eltBefore, int length=0);
		
	private:
		/**
		 * Update the musElements timeStarts from idx-th and on and adding the length from every one.
		 * Called when an element is deleted or inserted and startTimes after it need to be updated.
		 * If length is not set, take the element at idx as the inserted one and update times after it.
		 * 
		 * @param idx Index of the element where the update should begin. Or after it, if length is 0.
		 * @param length Length which should be substracted by the timeStarts.
		 */
		void updateTimes(int idx, int length=0);
		QList<CAMusElement *> _musElementList;
		CAStaff *_staff;	///Staff which this voice belongs to by default.
		int _voiceNumber;	///Voice number starting at 1.
		
		QString _name;
		
		////////////////////////////////////////////////
		//MIDI properties
		////////////////////////////////////////////////
		unsigned char _midiChannel;
		unsigned char _midiProgram;
};

#endif /*VOICE_H_*/
