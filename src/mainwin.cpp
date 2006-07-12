/*****************************************************************************/
/*									     */
/* This program is free software; you can redistribute it and/or modify it   */
/* under the terms of the GNU General Public License as published by the     */ 
/* Free Software Foundation; version 2 of the License.	                     */
/*									     */
/* This program is distributed in the hope that it will be useful, but       */
/* WITHOUT ANY WARRANTY; without even the implied warranty of                */ 
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General  */
/* Public License for more details.                                          */
/*									     */
/* You should have received a copy of the GNU General Public License along   */
/* with this program; (See "LICENSE.GPL"). If not, write to the Free         */
/* Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA        */
/* 02111-1307, USA.				                             */
/*									     */
/*---------------------------------------------------------------------------*/
/*									     */
/*		Reinhard Katzmann, GERMANY			             */
/*		reinhard@suamor.de					     */
/*									     */
/*									     */
/*****************************************************************************/

#include <QtGui/QtGui>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPoint>

#include <iostream>

using namespace std;

#include "mainwin.h"
#include "sheet.h"
#include "scrollwidget.h"
#include "engraver.h"
#include "scoreviewport.h"

// Constructor
CAMainWin::CAMainWin(QMainWindow *oParent)
  : QMainWindow( oParent )
{
	oMainWin.setupUi( this );
	connectActions();
	
	newDocument();
}

void CAMainWin::connectActions() {
}

void CAMainWin::newDocument() {
	_document.clear();	//clear the data
	clearUI();			//clear the UI
	
	addSheet();			//add a new empty sheet
}

void CAMainWin::addSheet() {
	CASheet *s = _document.addSheet(QString("Sheet ") + QString::number(_document.sheetCount()+1));
	CAScoreViewPort *v = new CAScoreViewPort(s, 0);

	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
	
	_viewPortList.append(v);
	
	oMainWin.tabWidget->addTab(new CAScrollWidget(v, 0), s->name());
	oMainWin.tabWidget->setCurrentIndex(oMainWin.tabWidget->count()-1);
	
	_activeViewPort = v;
}

void CAMainWin::clearUI() {
	for (int i=0; i<_viewPortList.size(); i++)
		delete _viewPortList[i];

	while (oMainWin.tabWidget->count()) {
		delete _currentScrollWidget;
		oMainWin.tabWidget->removeTab(oMainWin.tabWidget->currentIndex());
	}
}

void CAMainWin::on_action_Fullscreen_toggled(bool checked) {
	if (checked)
		this->showFullScreen();
	else
		this->showNormal();
}

void CAMainWin::on_actionSplit_horizontally_activated() {
	CAScoreViewPort *v = (CAScoreViewPort *)_currentScrollWidget->splitHorizontally();
	
	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
}

void CAMainWin::on_actionSplit_vertically_activated() {
	CAScoreViewPort *v = (CAScoreViewPort *)_currentScrollWidget->splitVertically();
	
	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
}

void CAMainWin::on_actionUnsplit_activated() {
	_currentScrollWidget->unsplit();
}

void CAMainWin::on_actionNew_viewport_activated() {
	CAScoreViewPort *v = (CAScoreViewPort *)_currentScrollWidget->newViewPort();

	connect(v, SIGNAL(CAMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortMousePressEvent(QMouseEvent *, QPoint, CAViewPort *)));
	connect(v, SIGNAL(CAWheelEvent(QWheelEvent *, QPoint, CAViewPort *)), this, SLOT(viewPortWheelEvent(QWheelEvent *, QPoint, CAViewPort *)));
}

void CAMainWin::on_actionNew_activated() {
	newDocument();
}

void CAMainWin::on_actionNew_sheet_activated() {
	addSheet();
}

void CAMainWin::on_actionNew_staff_activated() {
	CASheet *sheet = ((CAScoreViewPort*)_activeViewPort)->sheet();
	CAStaff *staff = sheet->addStaff();
	
	for (int i=0; i<_viewPortList.size(); i++) {
		if ( (((CAScoreViewPort*)(_viewPortList[i]))->sheet() == sheet) )
			((CAScoreViewPort*)(_viewPortList[i]))->update();
	}

	_activeViewPort->repaint();
}

void CAMainWin::viewPortMousePressEvent(QMouseEvent *e, QPoint coords, CAViewPort *v) {
	_activeViewPort = v;

	if (e->modifiers()==Qt::ControlModifier) {
		//_musElements.removeElement(coords.x(), coords.y());
	} else {
		//_musElements.addElement(new CANote(4, coords.x(), coords.y()));
		//_musElements.addElement(new CAStaff(_sheet, 0, coords.y()));
	}
	
	for (int i=0; i<_viewPortList.size(); i++) {
		((CAScoreViewPort*)(_viewPortList[i]))->checkScrollBars();
		_viewPortList[i]->repaint();
	}
}

void CAMainWin::viewPortWheelEvent(QWheelEvent *e, QPoint coords, CAViewPort *c) {
	_activeViewPort = c;
	CAScoreViewPort *v = (CAScoreViewPort*) c;

	int val;
	switch (e->modifiers()) {
		case Qt::NoModifier:			//scroll horizontally
			v->setWorldX( v->worldX() - (int)((0.5*e->delta()) / v->zoom()) );
			break;
		case Qt::AltModifier:			//scroll horizontally, fast
			v->setWorldX( v->worldX() - (int)(e->delta() / v->zoom()) );
			break;
		case Qt::ShiftModifier:			//scroll vertically
			v->setWorldY( v->worldY() - (int)((0.5*e->delta()) / v->zoom()) );
			break;
		case 0x0A000000://SHIFT+ALT		//scroll vertically, fast
			v->setWorldY( v->worldY() - (int)(e->delta() / v->zoom()) );
			break;
		case Qt::ControlModifier:		//zoom
			if (e->delta() > 0)
				v->setZoom( v->zoom()*1.1, coords.x(), coords.y() );
			else
				v->setZoom( v->zoom()/1.1, coords.x(), coords.y() );
			
			break;
	}

	v->repaint();
}
