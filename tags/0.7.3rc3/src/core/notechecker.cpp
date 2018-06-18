/*! 
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include <QObject>

#include "core/notechecker.h"
#include "score/sheet.h"
#include "score/staff.h"
#include "score/playablelength.h"
#include "score/timesignature.h"
#include "score/barline.h"
#include "score/notecheckererror.h"

/*!
	\class CANoteChecker
	\brief Class checking the user errors in the score (e.g. too long bars etc.)
	
	This class is spell checker that provides tools for checking potential
	"typing" errors made by the user such as too little notes not filling the bar
	and similar.
*/

CANoteChecker::CANoteChecker() {
}

CANoteChecker::~CANoteChecker() {
}

/*!
	Parse/decompress an existing archive
*/
void CANoteChecker::checkSheet(CASheet *sheet) {
	sheet->clearNoteCheckerErrors();
	
	// check for incomplete bars
	QList<CAStaff*> staffs = sheet->staffList();
	for (int i=0; i<staffs.size(); i++) {
		QList<CAMusElement*> timeSigs = staffs[i]->timeSignatureRefs();
		QList<CAMusElement*> barlines = staffs[i]->barlineRefs();
		
		if (!timeSigs.size()) {
			continue;
		}
		
		int lastTimeSigIdx = 0;
		int lastTimeSigRequiredDuration = static_cast<CATimeSignature*>(timeSigs[lastTimeSigIdx])->barDuration();
		int lastBarlineTime = -1;
		for (int j=0; j<barlines.size(); j++) {
			if (static_cast<CABarline*>(barlines[j])->barlineType()==CABarline::Dotted) {
				continue;
			}

			if (((lastTimeSigIdx+1)<timeSigs.size()) && barlines[j]->timeStart()>timeSigs[lastTimeSigIdx]->timeStart() ) {
				// go to next time sig
				lastTimeSigIdx++;
				lastTimeSigRequiredDuration = static_cast<CATimeSignature*>(timeSigs[lastTimeSigIdx])->barDuration();
			}
			
			// check the bar duration.
			// If first bar is partial, the length should be shorter or equal to time sig.
			if ((lastBarlineTime == -1 && barlines[j]->timeStart()>lastTimeSigRequiredDuration) ||
				(lastBarlineTime != -1 && barlines[j]->timeStart()!=lastBarlineTime+lastTimeSigRequiredDuration)) {
				CANoteCheckerError *nce = new CANoteCheckerError(barlines[j], QObject::tr("Bar duration incorrect."));
				sheet->addNoteCheckerError(nce);
			}
			
			lastBarlineTime = barlines[j]->timeStart();
		}
	}
}
