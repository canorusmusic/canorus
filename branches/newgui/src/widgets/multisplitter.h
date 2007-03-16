/** @file widgets/multisplitter.h
 * 
 * Copyright (c) 2007, Itay Perl, Canorus development team
 * All Rights Reserved. See AUTHORS for a complete list of authors.
 * 
 * Licensed under the GNU GENERAL PUBLIC LICENSE. See LICENSE.GPL for details.
 */

#ifndef MULTISPLITTER_H
#define MULTISPLITTER_H

#include <QSplitter>

class CAMultiSplitter : public QWidget
{
	Q_OBJECT
	
public:
	CAMultiSplitter(QWidget *first, QWidget *parent = 0);
	~CAMultiSplitter();
	void addWidget(QWidget *w, Qt::Orientation dir);
	void removeWidget(QWidget *w, bool autoDelete = false);
	inline QSplitter *main() { return  _splitterList.first(); }
	inline int widgetCount() { return _wCount; }
	inline QWidget* lastUsedWidget() { return _lastUsedWidget; }
	inline void setLastUsedWidget(QWidget *v) { _lastUsedWidget = v; }
	
private:
	CAMultiSplitter(); // Default constructor should not be used.
	QList<QSplitter*> _splitterList;
	QWidget* _lastUsedWidget;
	int _wCount;
};
#endif /* MULTISPLITTER_H */
