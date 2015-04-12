/*!
	Copyright (c) 2015, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
*/

#ifndef NOTECHECKERERROR_H_
#define NOTECHECKERERROR_H_

class CAMusElement;

class CANoteCheckerError {
public:
	CANoteCheckerError(CAMusElement *targetElement, QString message);
	~CANoteCheckerError();

private:
	CAMusElement *_targetElement;
	QString       _message;
};
#endif /* NOTECHECKERERROR_H_ */
