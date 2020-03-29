/*******************************************************
** class name:  ViewLog
**
** description: Show logs
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QDialog>
#include <QWidget>
#include <QTextEdit>
#include <QTimer>
#include <QDateTime>
#include <QTextCursor>
#include <QFileInfo>
#include <QTextDocument>
#include <QColor>

#include <cstring>

struct LogLevelColor {
    void set(const char* levelStr, const QColor& colorIn) {
        snprintf(level, 12, levelStr, strlen(levelStr));
        color = colorIn;
    }
    char level[12] = { 0 };
    QColor color;
};


class ViewLog : public QDialog
{
    Q_OBJECT
public:
    ViewLog(QWidget *parent);
    ~ViewLog();

    // layout
    void setupLayout();

    // Highlight log leve: Error, warning, trace, info, debug
    void highlightText();

    void readLogFile();

public slots:
    // check log file every 500ms
    void handleTimeout();

public:
    QTextEdit* logText;

private:
    QTimer* timer;
    QFileInfo logFileInfo;
    LogLevelColor levelColor[6];
    QDateTime lastModifiedTime;
    const char* logFilePath = "logs/basic-logger.txt";
};
