/*! 
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef NOTECHECKER_H_
#define NOTECHECKER_H_

class CASheet;

class CANoteChecker {
public:
	CANoteChecker();
	virtual ~CANoteChecker();
	
	void checkSheet(CASheet*);
};

#endif /* NOTECHECKER_H_ */
