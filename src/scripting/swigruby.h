/** @file scripting/swigruby.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifdef USE_RUBY
#include <ruby.h>

#include <QString>
#include <QList>

class CASwigRuby {
	public:
		enum CAClassType {
			Document,
			Sheet,
			Context,
			Staff,
			Voice,
			MusElement,
			Note,
			Rest,
			KeySignature,
			TimeSignature,
			Clef,
			Barline
		};
		
		static void init();	///Initializes Ruby and loads base 'CanorusRuby' module. Call this before any other Ruby operations! Call this before calling toRuby() or any other conversation functions as well!

		/**
	 	 * Call an external Ruby function in the given module with the list of arguments and return its Ruby value.
	 	 * 
	 	 * @param fileName Absolute path to the filename of the script
	 	 * @param function Function or method name.
	 	 * @param args List of arguments in Ruby's VALUE format. Use toRuby() to convert C++ classes to Ruby objects.
	 	 * @return Ruby's function return value in Ruby VALUE format.
	 	 */
		static VALUE callFunction(QString fileName, QString function, QList<VALUE> args);
		
};

/**
 * Ruby uses different objects than C++. They are actually wrappers around the original ones, but still share different memory and structure.
 * Use this function to create a Ruby object out of the C++ one.
 * 
 * @param object Pointer to the C++ object which the Ruby object should be derived from.
 * @param objectType See CAClassType. C++ doesn't support figuring out the object type from the raw pointer - you have to pass its class type as well.
 * @return Pointer to the Ruby object in Ruby's VALUE format.
 */
VALUE toRubyObject(void *object, CASwigRuby::CAClassType type);	//defined in canorusruby.i file

#endif
