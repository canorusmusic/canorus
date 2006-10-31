/** @file staff.cpp
 * 
 * Copyright (c) 2006, Matevž Jekovec, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
 */

#include <QPainter>
#include <iostream>

#include "core/voice.h"
#include "core/staff.h"
#include "core/note.h"

CAStaff::CAStaff(CASheet *s, QString name) : CAContext(s, name) {
	_contextType = CAContext::Staff;
	_numberOfLines = 5;
	_name = name;
}

int CAStaff::lastTimeEnd(CAVoice *voice) {
	if (voice)
		return voice->lastTimeEnd();
	
	int maxTime = 0;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->lastTimeEnd() > maxTime)
			maxTime = _voiceList[i]->lastTimeEnd();
	
	return maxTime;
}

void CAStaff::clear() {
	while (_voiceList.size()) {
		_voiceList.front()->clear();
		delete _voiceList.front();
		_voiceList.removeFirst();
	}
}

void CAStaff::insertSign(CAMusElement *sign) {
	for (int i=0; i<_voiceList.size(); i++)
		_voiceList[i]->insertMusElement(sign);
}

bool CAStaff::insertSignBefore(CAMusElement *sign, CAMusElement *eltAfter, bool force) {
	bool error = false;
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertMusElementBefore(sign, eltAfter, true, force))
			error = true;
	}
	
	return (!error);
}

bool CAStaff::insertSignAfter(CAMusElement *sign, CAMusElement *eltBefore, bool force) {
	bool error = false;
	for (int i=0; i<_voiceList.size(); i++) {
		if (!_voiceList[i]->insertMusElementAfter(sign, eltBefore, true, force))
			error = true;
	}
	
	return (!error);
}

bool CAStaff::removeMusElement(CAMusElement *elt, bool cleanup) {
	bool success = false;
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->removeElement(elt))
			success = true;
	
	if (cleanup)
		delete elt;
		
	return success;
}

CAMusElement *CAStaff::findNextMusElement(CAMusElement *elt) {
	int idx;
	
	for (int i=0; i<_voiceList.size(); i++) {	//go through all the voices and check, if anyone of them includes the given element
		if ((idx = _voiceList[i]->indexOf(elt)) != -1) {
			if (++idx < _voiceList[i]->musElementCount())
				return _voiceList[i]->musElementAt(idx); //return the pointer of the next element
			else
				return 0;	//or 0, if the element is the last one and doesn't have its right neighbour
		}
	}
	
	return 0;	//the element doesn't exist in any of the voices, return 0
}

CAMusElement *CAStaff::findPrevMusElement(CAMusElement *elt) {
	int idx;
	
	for (int i=0; i<_voiceList.size(); i++) {	//go through all the voices and check, if anyone of them includes the given element
		if ((idx = _voiceList[i]->indexOf(elt)) != -1) {
			if (--idx > -1)
				return _voiceList[i]->musElementAt(idx); //return the pointer of the previous element
			else
				return 0;	//or 0, if the element is the first one and doesn't have its left neighbour
		}
	}
	
	return 0;	//the element doesn't exist in any of the voices, return 0
}

CAVoice *CAStaff::voice(const QString name) {
	for (int i=0; i<_voiceList.size(); i++)
		if (_voiceList[i]->name() == name)
			return _voiceList[i];
	
	return 0;
}

QList<CAMusElement*> CAStaff::getEltByType(CAMusElement::CAMusElementType type, int startTime) {
	QList<CAMusElement*> eltList;
	
	for (int i=0; i<_voiceList.size(); i++) {
		QList<CAMusElement*> curList;
		curList = _voiceList[i]->getEltByType(type, startTime);
		eltList += curList;
	}
	
	return eltList;	
}

bool CAStaff::fixVoiceErrors() {
	QList<CAMusElement *> signsNeeded;
	QList<CAMusElement *> prevSignsNeeded;	//list of music elements before that sign in that voice
	QList<CAMusElement *> signsIncluded[voiceCount()];
	
	for (int i=0; i<voiceCount(); i++) {
		QList<CAMusElement*> *list = _voiceList[i]->musElementList();
		for (int j=0; j<list->size(); j++) {
			if (!list->at(j)->isPlayable()) {
				signsIncluded[i] << list->at(j);	//add the current sign to voice's included list
				if (!signsNeeded.contains(list->at(j))) {
					signsNeeded << list->at(j);	//add the current sign to others voices needed list
					prevSignsNeeded << _voiceList[i]->eltBefore(list->at(j));
				}
			}
		}
	}
	
	bool everythingIncluded = true;
	for (int i=0; i<signsNeeded.size(); i++) {
		for (int j=0; j<voiceCount(); j++) {
			if (!signsIncluded[j].contains(signsNeeded[i])) {
				everythingIncluded = false;
				if (prevSignsNeeded[i] && (!prevSignsNeeded[i]->isPlayable()))
					_voiceList[j]->insertMusElementAfter(signsNeeded[i], prevSignsNeeded[i]);
				else
					_voiceList[j]->insertMusElement(signsNeeded[i]);
			}
		}
	}
	
	return everythingIncluded;
}
