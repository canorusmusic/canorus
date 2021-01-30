/*!
	Copyright (c) 2009-2019, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#include <QTimer>

#include "control/mainwinprogressctl.h"
#include "core/file.h"
#include "ui/mainwin.h"
#include "widgets/progressstatusbar.h"

CAMainWinProgressCtl::CAMainWinProgressCtl(CAMainWin* mainWin)
    : _mainWin(mainWin)
    , _bar(nullptr)
    , _updateTimer(nullptr)
    , _file(nullptr)
{
}

CAMainWinProgressCtl::~CAMainWinProgressCtl() = default;

void CAMainWinProgressCtl::on_updateTimer_timeout()
{
    if (_file) {
        _bar->setProgress(_file->readableStatus(), _file->progress());

        if (_file->isFinished()) {
            restoreStatusBar();
            _updateTimer->stop();
        }
    }
}

void CAMainWinProgressCtl::on_cancelButton_clicked(bool)
{
    if (_file) {
        _file->exit();
        restoreStatusBar();
        _updateTimer->stop();

        _file->wait();
    }
}

void CAMainWinProgressCtl::restoreStatusBar()
{
    _mainWin->statusBar()->removeWidget(_bar.get());
    _bar.reset();
}

void CAMainWinProgressCtl::startProgress(CAFile &f)
{
    _file = &f;
    _mainWin->setMode(CAMainWin::ProgressMode);

    if (_updateTimer) {
        _updateTimer.reset();
    }

    _updateTimer = std::make_unique<QTimer>();
    _updateTimer->setInterval(150);
    _updateTimer->setSingleShot(false);

    connect(_updateTimer.get(), SIGNAL(timeout()), this, SLOT(on_updateTimer_timeout()));

    _bar = std::make_unique<CAProgressStatusBar>(_mainWin.get());
    _mainWin->statusBar()->addWidget(_bar.get());
    connect(_bar.get(), SIGNAL(cancelButtonClicked(bool)), this, SLOT(on_cancelButton_clicked(bool)));

    _updateTimer->start();
}
