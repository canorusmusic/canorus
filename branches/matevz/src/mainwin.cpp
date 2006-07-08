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

// Constructor
CAMainWin::CAMainWin(QMainWindow *oParent)
  : QMainWindow( oParent )
{
  oMainWin.setupUi( this );
  
  connect( oMainWin.actionSplit_horizontally, SIGNAL(triggered(bool)), this, SLOT(processSplitHorizontallyEvent(bool)) );
  connect( oMainWin.actionSplit_vertically, SIGNAL(triggered(bool)), this, SLOT(processSplitVerticallyEvent(bool)) );
  connect( oMainWin.actionUnsplit, SIGNAL(triggered(bool)), this, SLOT(processUnsplitEvent(bool)) );
  connect( oMainWin.actionNew_viewport, SIGNAL(triggered(bool)), this, SLOT(processNewViewPortEvent(bool)) );
}

void CAMainWin::processSplitHorizontallyEvent(bool checked) {
	oMainWin.frame->splitHorizontally();
}

void CAMainWin::processSplitVerticallyEvent(bool checked) {
	oMainWin.frame->splitVertically();
}

void CAMainWin::processUnsplitEvent(bool checked) {
	oMainWin.frame->unsplit();
}

void CAMainWin::processNewViewPortEvent(bool checked) {
	oMainWin.frame->newViewPort();
}
