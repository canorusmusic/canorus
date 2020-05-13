/*!
	Copyright (c) 2009, Matevž Jekovec, Canorus development team
	All Rights Reserved. See AUTHORS for a complete list of authors.

	Licensed under the GNU GENERAL PUBLIC LICENSE. See COPYING for details.
*/

#ifndef MAINWINPROGRESSCTL_H_
#define MAINWINPROGRESSCTL_H_

#include <QObject>

#include <memory>

class CAMainWin;
class CAProgressStatusBar;
class CAFile;
class QTimer;

class CAMainWinProgressCtl : public QObject {
    Q_OBJECT
public:
    CAMainWinProgressCtl(CAMainWin* mainWin);
    ~CAMainWinProgressCtl();

    void startProgress(CAFile &f);

private slots:
    void on_updateTimer_timeout();
    void on_cancelButton_clicked(bool);

private:
    void restoreStatusBar();

    std::unique_ptr<CAMainWin> _mainWin;
    std::unique_ptr<CAProgressStatusBar> _bar;
    std::unique_ptr<QTimer> _updateTimer;
    CAFile *_file;
};

#endif /* MAINWINPROGRESSCTL_H_ */
