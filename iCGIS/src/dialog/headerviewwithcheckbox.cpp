#include "dialog/headerviewwithcheckbox.h"

HeaderViewWithCheckbox::HeaderViewWithCheckbox(int checkColIndex, Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent),
    checkState(Qt::Unchecked),
    mousePoint({ 100, 100 }),
    checkColIndex(checkColIndex)
{
}

// slot
// update checkboxes in header
void HeaderViewWithCheckbox::updateCheckState(Qt::CheckState checkState)
{
    this->checkState = checkState;
    updateSection(checkColIndex);
}

void HeaderViewWithCheckbox::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == 0) {
        rectHeaderCheckBox.setX(rect.x() + 10);
        rectHeaderCheckBox.setY(rect.y() + 5);
        rectHeaderCheckBox.setWidth(14);
        rectHeaderCheckBox.setHeight(14);
        QStyleOptionButton option;
        QPixmap pix;
        if (checkState == Qt::Checked) {
            pix.load("res/icons/check-sel.png");
            option.state = QStyle::State_On;
        }
        else if (checkState == Qt::PartiallyChecked) {
            pix.load("res/icons/check-part.png");
            option.state = QStyle::State_On;
        }
        else if (rectHeaderCheckBox.contains(mousePoint)) {
            pix.load("res/icons/check-hov.png");
            option.state = QStyle::State_HasFocus;
        }
        else if (checkState == Qt::Unchecked) {
            pix.load("res/icons/check-nor.png");
            option.state = QStyle::State_Off;
        }
        style()->drawItemPixmap(painter, rect, Qt::AlignCenter, pix);
    }
}

void HeaderViewWithCheckbox::mousePressEvent(QMouseEvent *event)
{
    if (isEnabled() && logicalIndexAt(event->pos()) == 0 && rectHeaderCheckBox.contains(event->pos())) {
        if (checkState == Qt::Unchecked)
            checkState = Qt::Checked;
        else
            checkState = Qt::Unchecked;
        updateSection(checkColIndex);
        emit headCheckboxToggled(checkState);
    }
    else {
        QHeaderView::mousePressEvent(event);
    }
}

void HeaderViewWithCheckbox::mouseMoveEvent(QMouseEvent *event)
{
    mousePoint = event->pos();
    if (rectHeaderCheckBox.contains(mousePoint)) {
        updateSection(checkColIndex);
    }
    else {
        QHeaderView::mouseMoveEvent(event);
    }
}
