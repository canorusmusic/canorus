/** @file muselementfactory.h
 *
 * This program is free software; you can redistribute it and/or modify it   
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.                       
 *                                                                           
 * This program is distributed in the hope that it will be useful, but       
 * WITHOUT ANY WARRANTY; without even the implied warranty of               
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  
 * Public License for more details.                                          
 *                                                                           
 * You should have received a copy of the GNU General Public License along   
 * with this program; (See "LICENSE.GPL"). If not, write to the Free         
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        
 * 02111-1307, USA.                                                          
 *                                                                           
 *---------------------------------------------------------------------------
 *                                                                           
 * Copyright (c) 2006, Reinhard Katzmann, Canorus development team           
 * All Rights Reserved. See AUTHORS for a complete list of authors.          
 *                                                                           
 */

// Music element factory (creation, removal, configuration)

#ifndef MUSELEMENTFACTORY_H_
#define MUSELEMENTFACTORY_H_

class CAMusElement;

class CAMusElementFactory
{
public:
	/**
	* Con/Destructor for music element factory
	*
	*/
	CAMusElementFactory();
	~CAMusElementFactory();

	/**
	* Create placebo music element (type: undefined)
	*
	*/
	CAMusElement *createMusElem();

	/**
	* Remove current music element
	*
	* @param bReallyRemove Delete from memory too independent of type
	*
	*/
	void removeMusElem( bool bReallyRemove = false );

	/**
	* Configure music element via clone
	*
	* @param roMusElement Instance of a music element to be cloned
	*
	*/
	void configureMusElem( CAMusElement &roMusElement );

	/**
	* Read current music element
	*
	* @return Instance of the music element
	*
	*/
	inline CAMusElement *getMusElement() { return mpoMusElement; };

	/**
	* Configure new clef music element
	*
	* @param context     context within the new clef is inserted
	* @param left        music element left of new clef
	*
	*/
	bool configureClef( CADrawableContext *context, 
	                    CADrawableMusElement *left );

	/**
	* Configure new key signature music element
	*
	* @param iKeySignature  key signature as integer value
	* @param context        context within the new key signature is inserted
	* @param left           music element left of new key signature
	*
	*/
	bool configureKeySignature( int iKeySignature,
	                            CADrawableContext *context, 
	                            CADrawableMusElement *left );

	/**
	* Configure new time signature music element
	*
	* @param context        context within the new time signature is inserted
	* @param left           music element left of new time signature
	*
	*/
	bool configureTimeSignature( CADrawableContext *context, 
                                     CADrawableMusElement *left );

	/**
	* Configure new rest music element
	*
	* @param voice            voice where new rest is inserted
	* @param coords           mouse position where the new rest is inserted
	* @param context          context within the new rest is inserted
	* @param left             music element left of new rest
	*
	*/
	bool configureRest( CAVoice *voice,
	                    const QPoint coords,
	                    CADrawableContext *context, 
	                    CADrawableMusElement *left );

	/**
	* Configure new note music element
	*
	* @param iVoiceNum        voice number where new note is inserted
	* @param voice            voice where new note is inserted
	* @param coords           mouse position where the new note is inserted
	* @param context          context within the new note is inserted
	* @param left             music element left of new note
	*
	*/
	bool configureNote( CAVoice *voice,
	                    const QPoint coords,
	                    CADrawableContext *context, 
	                    CADrawableMusElement *left );

	/**
	* Read current music element type
	*
	* @return music element type
	*
	*/
	inline CAMusElement::CAMusElementType musElementType()
	{ return mpoMusElement->musElementType(); };

	/**
	* Set new current music element type, does not create a new element of this type!
	*
	* @param eMEType music element type
	*
	*/
	void setMusElementType( CAMusElement::CAMusElementType eMEType );

	inline void setPlayableLength( CAPlayable::CAPlayableLength ePlayableLength )
	{ _ePlayableLength = ePlayableLength; };

        inline int  playableDotted() { return _iPlayableDotted; };

	inline void setPlayableDotted( int iPlaybleDotted )
	{ _iPlayableDotted = iPlaybleDotted; };

	inline void addPlayableDotted( int iAdd )
	{ _iPlayableDotted = (_iPlayableDotted+iAdd)%4; };

        inline int  noteAccs() { return _iNoteAccs; };

	inline void setNoteAccs( int iNoteAccs )
	{ _iNoteAccs = iNoteAccs; };

	inline void addNoteAccs( int iAdd )
	{ _iNoteAccs+= iAdd; };

	inline void subNoteAccs( int iSub )
	{ _iNoteAccs-= iSub; };

        inline int  noteExtraAccs() { return _iNoteExtraAccs; };

	inline void setNoteExtraAccs( int iNoteExtraAccs )
	{ _iNoteExtraAccs = iNoteExtraAccs; };

	inline void addNoteExtraAccs( int iAdd )
	{ _iNoteExtraAccs += iAdd; };

	inline void subNoteExtraAccs( int iSub )
	{ _iNoteExtraAccs -= iSub; };

	inline void setTimeSigBeats( int iTimeSigBeats )
	{ _iTimeSigBeats = iTimeSigBeats; };

	inline void setTimeSigBeat( int iTimeSigBeat )
	{ _iTimeSigBeat = iTimeSigBeat; };

	inline void setClef( CAClef::CAClefType eClefType )
	{ _eClef = eClefType; };

private:
	CAMusElement *mpoMusElement;    /// newly created music element itself
	////////////////////////////////////////////////////
	// Element creation parameters
	////////////////////////////////////////////////////
	CAPlayable::CAPlayableLength _ePlayableLength;	/// Length of note/rest to be added
	int _iPlayableDotted;	   /// Number of dots to be inserted for the note/rest
	int _iNoteExtraAccs;	   /// Extra note accidentals for new notes which user adds/removes with +/- keys
	int _iNoteAccs;	           /// Note accidentals at specific coordinates updated regularily when in insert mode
	int _iTimeSigBeats;        /// Time signature number of beats to be inserted
	int _iTimeSigBeat;         /// Time signature beat to be inserted
	CAClef::CAClefType _eClef; /// Type of the clef to be inserted
};

#endif // MUSELEMENTFACTORY_H_
