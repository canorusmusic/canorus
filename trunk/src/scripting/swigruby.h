/*!
	Copyright (c) 2006-2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifdef USE_RUBY
#ifndef SWIGRUBY_H_
#define SWIGRUBY_H_

#include <ruby.h>

#include <QString>
#include <QList>

class CASwigRuby {
	public:
		enum CAClassType {
			//Qt objects
			String,
			
			//Canorus objects
			Document,
			Sheet,
			Context,
			Staff,
			Voice,
			FunctionMarkContext,
			MusElement,
			Playable,
			Note,
			Rest,
			KeySignature,
			TimeSignature,
			Clef,
			Barline,
			FunctionMark,
			LyricsContext,
			Syllable,
			Mark
		};
		
		static void init();	///Initializes Ruby and loads base 'CanorusRuby' module. Call this before any other Ruby operations! Call this before calling toRubyObject() or any other conversation functions as well!

		/**
	 	 * Call an external Ruby function in the given module with the list of arguments and return its Ruby value.
	 	 * 
	 	 * @param fileName Absolute path to the filename of the script
	 	 * @param function Function or method name.
	 	 * @param args List of arguments in Ruby's VALUE format. Use toRubyObject() to convert C++ classes to Ruby objects.
	 	 * @return Ruby's function return value in Ruby VALUE format.
	 	 */
		static VALUE callFunction(QString fileName, QString function, QList<VALUE> args);
	
		/**
 		 * Ruby uses different objects than C++. They are actually wrappers around the original ones, but still share different memory and structure.
 		 * Use this function to create a Ruby object out of the C++ one.
 		 * 
 		 * @param object Pointer to the C++ object which the Ruby object should be derived from.
 		 * @param objectType See CAClassType. C++ doesn't support figuring out the object type from the raw pointer - you have to pass its class type as well.
 		 * @return Pointer to the Ruby object in Ruby's VALUE format.
 		 */
		static VALUE toRubyObject(void *object, CAClassType type);	//defined in scripting/canorusruby.i file
};

#endif /*SWIGRUBY_H_*/
#endif
