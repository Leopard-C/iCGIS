#include "widget/statusbar.h"
#include "util/appevent.h"

#include <QString>

StatusBar::StatusBar(QStatusBar *statusBarIn)
    : statusBar(statusBarIn) {
    createWidgets();
    setupLayout();
    setUnit(kDegree);
    onUpdateCoord(0.0, 0.0);

    connect(AppEvent::getInstance(), &AppEvent::sigUpdateCoord,
            this, &StatusBar::onUpdateCoord);
}

StatusBar::~StatusBar() {}

void StatusBar::setUnit(CoordUint unitIn) {
    switch (unitIn) {
    case kDegree:
        labelUnit->setText("Degree");
        break;
    case kMeter:
        break;
    case kKiloMeter:
        break;
    default:
        break;
    }

    unit = unitIn;
}

// show current coordinate of mouse cursor's position
void StatusBar::onUpdateCoord(double x, double y) {
    switch (unit) {
    case kDegree: {
        QString coordText = QString("%1E %2N").arg(x, 8, 'f', 5).arg(y, 8, 'f', 5);
        labelCoord->setText(coordText);
        break;
    }
    case kMeter:
        break;
    case kKiloMeter:
        break;
    default:
        break;
    }
}

// show message for timeMs milliseconds in left area
void StatusBar::showMsg(const QString &msg, int timeMs)
{
    statusBar->showMessage(msg, timeMs);
}

void StatusBar::createWidgets() {
    labelCoord = new QLabel();
    labelUnit = new QLabel();
}

void StatusBar::setupLayout() {
    this->statusBar->addPermanentWidget(labelCoord);
    this->statusBar->addPermanentWidget(labelUnit);
}
