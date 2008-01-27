/*! 
	Copyright (c) 2007, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef AUTOSAVE_H_
#define AUTOSAVE_H_

#include <QObject>

class QTimer;

class CAAutoRecovery : public QObject {
	Q_OBJECT
	
public:
	CAAutoRecovery();
	~CAAutoRecovery();
	void updateTimer();
	void openRecovery();
	
public slots:
	void cleanupRecovery();
	void saveRecovery();
	
private:
	QTimer *_autoRecoveryTimer;
};

#endif /* AUTOSAVE_H_ */
