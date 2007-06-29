/*!
	Copyright (c) 2006-2007, Reinhard Katzmann, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.
	
	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef MENUTOOLBUTTON_H_
#define MENUTOOLBUTTON_H_

#include <QToolButton>
#include <QButtonGroup>
#include <QGroupBox>
#include <QGridLayout>
#include <QHash>

class CAMainWin;

class CAMenuToolButton : public QToolButton {
	Q_OBJECT
public:
	CAMenuToolButton( QString title, int numIconsRow = 4, QWidget * parent = 0 );
	~CAMenuToolButton();

	void addButton( const QIcon icon, int buttonId, const QString toolTip="" );
	inline QAbstractButton *getButton( int buttonId ) { return _buttonGroup->button( buttonId ); }

	inline QList<QToolButton*> buttonList() { return _buttonList; }
	
	inline int spacing() { return _spacing; }
	inline int layoutMargin() { return _layoutMargin; }
	inline int margin() { return _margin;}
	inline int numIconsPerRow() { return _numIconsRow; }
	inline int currentId() { return _currentId; }
	
	inline void setSpacing(int spacing) { _spacing = spacing; }
	inline void setLayoutMargin(int margin) { _layoutMargin = margin; }
	inline void setMargin(int margin) { _margin = margin; }
	inline void setNumIconsPerRow( int numIconsRow )  { _numIconsRow = numIconsRow; }
	void setCurrentId(int id);
	
public slots:
	void showButtons();
	void hideButtons( int buttonId );
	void hideButtons();
	
signals:
	void toggled( bool checked, int id );

private slots:
	void handleToggled(bool checked);
	void handleTriggered();
	
protected:
	void wheelEvent(QWheelEvent*);
	inline CAMainWin *mainWin() { return _mainWin; }
	inline void setMainWin( CAMainWin *m ) { _mainWin = m; }
	void mousePressEvent( QMouseEvent* );
	
	CAMainWin          *_mainWin;         // Pointer to the main window for toolbar location polling etc.
	QButtonGroup       *_buttonGroup;     // Abstract group for the button actions
	QGroupBox          *_groupBox;        // Group box containing title and buttons
	QGridLayout        *_boxLayout;       // Layout for the group box
	QGridLayout        *_menuLayout;      // Layout for the button menu
	QList<QToolButton*> _buttonList;      // List of created buttons in button box
	QHash<QString, int> _buttonIds;       // hash of IDs of buttons
	int                 _currentId;       // current ID of the button
	int                 _buttonXPos;      // X position of next button
	int                 _buttonYPos;      // Y position of next button
	int                 _numIconsRow;     // Number of icons per row
	int                 _spacing;         // Space between buttons
	int                 _margin;          // Margin around the buttons
	int                 _layoutMargin;    // Margin of layout
};
#endif /* MENUTOOLBUTTON_H_ */
