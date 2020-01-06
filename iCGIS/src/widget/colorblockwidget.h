/*******************************************************
** class name:  ColorBlockWidget
**
** description: 颜色方块儿，用于LayerStyleDialog中
**              整个widget只有一个背景色
**
** last change: 2020-01-02
*******************************************************/

#pragma once

#include <QWidget>
#include <QColor>

#include "utility.h"


class ColorBlockWidget : public QWidget
{
	Q_OBJECT

public:
	ColorBlockWidget(QWidget *parent);
	~ColorBlockWidget();

protected:
	void mouseDoubleClickEvent(QMouseEvent *event) override;

public:
	void setColor(int r, int g, int b);
	void setColor(const utils::Color& colorIn);
	void setColor(const QColor& colorIn);
	const QColor& getColor() const;

	int getFID() const { return FID; }
	void setFID(int nFID) { FID = nFID; }

private:
	int FID;
};
