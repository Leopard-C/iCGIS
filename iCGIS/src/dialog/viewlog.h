/*******************************************************
** class name:  ViewLog
**
** description: 日志窗口
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

struct LogLevelColor {
	void set(const char* levelStr, const QColor& colorIn) {
		sprintf_s(level, 10, levelStr, strlen(levelStr));
		color = colorIn;
	}
	char level[10] = { 0 };
	QColor color;
};


class ViewLog : public QDialog
{
	Q_OBJECT
public:
	ViewLog(QWidget *parent);
	~ViewLog();

	// 布局
	void setupLayout();

	// 高亮 Error、warning、trace、info、debug等日志级别
	void highlightText();

	// 读取日志文件
	void readLogFile();

public slots:
	// 每500ms检查一次是否有日志写入
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
