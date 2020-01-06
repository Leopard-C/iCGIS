/************************************************************************
** class name:  HeaderViewWidthCheckbox
**
** description: 重写QHeaderView，在第一列加入复选框，实现全选/全不选功能
**				应用在：PostgresqlTableSelect
**				参见：dialog/postgresqltableselect.h
**
** last change: 2020-01-02
*************************************************************************/
#pragma once

#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>


class HeaderViewWithCheckbox : public QHeaderView
{
	Q_OBJECT
public:
	explicit HeaderViewWithCheckbox(int checkColIndex, Qt::Orientation orientation, QWidget* parent = 0);

signals:
	void headCheckboxToggled(Qt::CheckState checkState);

public slots:
	void updateCheckState(Qt::CheckState checkState);

protected:
	void paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const override;
	void mousePressEvent(QMouseEvent *event) override;
	void mouseMoveEvent(QMouseEvent *event) override;

private:
	Qt::CheckState checkState;
	QPoint mousePoint;
	int checkColIndex;
	mutable QRect rectHeaderCheckBox;
};
