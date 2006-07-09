/** @file sheet.h
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#ifndef SHEET_H_
#define SHEET_H_

#include "staff.h"
#include "context.h"

class CASheet {
	public:
		void clear();
	
	private:
		QList<CAContext *> _contextList;	///List of all the contexts in the sheet (lyrics, staffs, tablatures, general-bas markings etc.)
		QList<CAStaff *> _staffList;	//List of all the staffs only in the sheet
		
};

#endif /*SHEET_H_*/
