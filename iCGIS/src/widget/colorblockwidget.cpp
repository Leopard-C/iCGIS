#include "widget/colorblockwidget.h"

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
    QColor color = QColorDialog::getColor(Qt::white, nullptr);
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

const QColor& ColorBlockWidget::getColor() const
{
    return this->palette().background().color();
}
