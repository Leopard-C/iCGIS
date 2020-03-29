/*******************************************************
** class name:  StatusBar
**
** last change: 2020-02-06
*******************************************************/
#pragma once

#include <QLabel>
#include <QObject>
#include <QStatusBar>

enum CoordUint {
    kDegree = 0,
    kMeter = 1,
    kKiloMeter = 2
};

class StatusBar : public QObject
{
    Q_OBJECT
public:
    StatusBar(QStatusBar* statusBarIn);
    ~StatusBar();

public slots:
    void onUpdateCoord(double x, double y);

public:
    void setUnit(CoordUint unit);
    void showMsg(const QString& msg, int timeMs = 0);

private:
    void createWidgets();
    void setupLayout();

private:
    QStatusBar* statusBar;
    QLabel* labelCoord;
    QLabel* labelUnit;
    CoordUint unit = kDegree;
};
