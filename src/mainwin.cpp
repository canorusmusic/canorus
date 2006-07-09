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

#include "mainwin.h"
#include "sheet.h"
#include "scrollwidget.h"

// Constructor
CAMainWin::CAMainWin(QMainWindow *oParent)
  : QMainWindow( oParent )
{
	oMainWin.setupUi( this );
	connectActions();
	
	newDocument();
}

void CAMainWin::connectActions() {
	//////////////////////////////////////////////////////
	//Menu bar actions
	//////////////////////////////////////////////////////
	//File menu
	connect( oMainWin.actionNew, SIGNAL(triggered(bool)), this, SLOT(processNewEvent(bool)) );
	connect( oMainWin.actionNew_sheet, SIGNAL(triggered(bool)), this, SLOT(processNewSheetEvent(bool)) );
	
	//View menu
	connect( oMainWin.action_Fullscreen, SIGNAL(triggered(bool)), this, SLOT(processFullScreenEvent(bool)) );
	
	//Window menu
	connect( oMainWin.actionSplit_horizontally, SIGNAL(triggered(bool)), this, SLOT(processSplitHorizontallyEvent(bool)) );
	connect( oMainWin.actionSplit_vertically, SIGNAL(triggered(bool)), this, SLOT(processSplitVerticallyEvent(bool)) );
	connect( oMainWin.actionUnsplit, SIGNAL(triggered(bool)), this, SLOT(processUnsplitEvent(bool)) );
	connect( oMainWin.actionNew_viewport, SIGNAL(triggered(bool)), this, SLOT(processNewViewPortEvent(bool)) );
}

void CAMainWin::newDocument() {
	_document.clear();
	clearSheets();
	
	addSheet();
}

void CAMainWin::addSheet() {
	CASheet *s = _document.addSheet();
	oMainWin.tabWidget->addTab(new CAScrollWidget(s, 0), QString("Sheet ") + QString(_document.sheetCount()+48) );
}

void CAMainWin::clearSheets() {
	for (int i=0; i<oMainWin.tabWidget->count(); i++) {
		delete ((CAScrollWidget *)(oMainWin.tabWidget->currentWidget()));
		oMainWin.tabWidget->removeTab(oMainWin.tabWidget->currentIndex());
	}
}

void CAMainWin::processFullScreenEvent(bool checked) {
	if (checked)
		this->showFullScreen();
	else
		this->showNormal();
}

void CAMainWin::processSplitHorizontallyEvent(bool checked) {
	( (CAScrollWidget*)(oMainWin.tabWidget->currentWidget()) )->splitHorizontally();
}

void CAMainWin::processSplitVerticallyEvent(bool checked) {
	( (CAScrollWidget*)(oMainWin.tabWidget->currentWidget()) )->splitVertically();
}

void CAMainWin::processUnsplitEvent(bool checked) {
	( (CAScrollWidget*)(oMainWin.tabWidget->currentWidget()) )->unsplit();
}

void CAMainWin::processNewViewPortEvent(bool checked) {
	( (CAScrollWidget*)(oMainWin.tabWidget->currentWidget()) )->newViewPort();
}

void CAMainWin::processNewEvent(bool checked) {
	newDocument();
}

void CAMainWin::processNewSheetEvent(bool checked) {
	addSheet();
}
