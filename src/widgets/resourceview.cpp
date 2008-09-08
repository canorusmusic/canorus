/*!
	Copyright (c) 2008, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include "widgets/resourceview.h"
#include "core/document.h"
#include "core/resource.h"
#include "canorus.h"

#include <QStringList>

/*!
	\class CAResourceView
	\brief Tree view of all the resources inside the document

	This widget shows the resources stored inside the document in a
	tree-view style. Pass the document to the constructor and call
	rebuildUi() to refresh the GUI.

	This widget also allows user to remove or rename the resources.
 */

/*!
	Default constructor.

	Pass the document \a doc. The view automatically gathers the resources
	stored inside the document and shows them.
 */
CAResourceView::CAResourceView( CADocument *doc, QWidget *parent )
 : QTreeWidget( parent ), _document(doc) {
	setColumnCount(2);
	setHeaderLabels( QStringList() << tr("Name") << tr("Linked") );
	setWindowTitle(tr("Document Resources"));

	rebuildUi();
}

CAResourceView::~CAResourceView() {
}

void CAResourceView::rebuildUi() {
	clear();
	_items.clear();

	if (document()) {
		for (int i=0; i<document()->resourceList().size(); i++) {
			QTreeWidgetItem *item = new QTreeWidgetItem( QStringList() << document()->resourceList()[i]->name() << (document()->resourceList()[i]->isLinked()?tr("yes"):tr("no")) );
			_items[ item ] = document()->resourceList()[i];
			addTopLevelItem( item );
		}
	}
}

void CAResourceView::showEvent( QShowEvent * event ) {
	QList<CAMainWin*> mainWins = CACanorus::findMainWin( document() );

	for (int i=0; i<mainWins.size(); i++) {
		mainWins[i]->resourceViewAction()->setChecked(true);
	}
}

void CAResourceView::closeEvent( QCloseEvent * event ) {
	QList<CAMainWin*> mainWins = CACanorus::findMainWin( document() );

	for (int i=0; i<mainWins.size(); i++) {
		mainWins[i]->resourceViewAction()->setChecked(false);
	}
}
