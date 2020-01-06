#include "colorblockwidget.h"
#include "logger.h"

#include <QString>
#include <QColorDialog>

ColorBlockWidget::ColorBlockWidget(QWidget *parent)
	: QWidget(parent)
{
	this->setAttribute(Qt::WA_StyledBackground, true);
}

ColorBlockWidget::~ColorBlockWidget()
{
}

void ColorBlockWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	Log("Double clicked");
	QColor color = QColorDialog::getColor(Qt::white, 0);
	// 用户是否点击了取消（或者直接关闭了对话框）
	// 颜色值邮箱，就修改当前色块儿的颜色
	if (color.isValid())
		setColor(color);
}


void ColorBlockWidget::setColor(int r, int g, int b)
{
	QString bgColor = QString("background-color:rgb(%1,%2,%3);").
		arg(r).arg(g).arg(b);
	this->setStyleSheet(bgColor);
}

void ColorBlockWidget::setColor(const QColor& colorIn)
{
	setColor(colorIn.red(), colorIn.green(), colorIn.blue());
}

void ColorBlockWidget::setColor(const utils::Color& colorIn)
{
	setColor(colorIn.r, colorIn.g, colorIn.b);
}

const QColor& ColorBlockWidget::getColor() const
{
	return this->palette().background().color();
}
