/*!
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#include "score/muselement.h"
#include "score/notecheckererror.h"
#include "context.h"
#include "sheet.h"

/*!
	\class CANoteCheckerError
	\brief Class representing the error produced by the note checker

	CANoteCheckerError is a class corresponding to the error, warning, hint etc.
	in the score, produced by the CANoteChecker.
	
	\sa CANoteChecker, CADrawableNoteCheckerError
*/

CANoteCheckerError::CANoteCheckerError(CAMusElement *targetElement, QString message)
 : _targetElement(targetElement), _message(message) {
	targetElement->addNoteCheckerError(this);
}

CANoteCheckerError::~CANoteCheckerError() {
	if (_targetElement) {
		_targetElement->removeNoteCheckerError(this);
		if (_targetElement->context() && _targetElement->context()->sheet()) {
			_targetElement->context()->sheet()->noteCheckerErrorList().removeAll(this);
		}
	}
}
