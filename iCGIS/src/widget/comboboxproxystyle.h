
#pragma once

#include <QProxyStyle>
#include <QStyleOptionComboBox>

#include <QStyledItemDelegate>
#include <QApplication>

//class ComboBoxProxyStyle : public QProxyStyle
//{
//public:
//    using QProxyStyle::QProxyStyle;
//    void drawControl(QStyle::ControlElement element, const QStyleOption *opt, QPainter *p, const QWidget *w) const
//    {
//        if(element == QStyle::CE_ComboBoxLabel){
//            if (const QStyleOptionComboBox *cb = qstyleoption_cast<const QStyleOptionComboBox *>(opt)) {
//                QStyleOptionComboBox cb_tmp(*cb);
//                cb_tmp.currentIcon = QIcon();
//                cb_tmp.iconSize = QSize();
//                QProxyStyle::drawControl(element, &cb_tmp, p, w);
//                return;
//            }
//        }
//        QProxyStyle::drawControl(element, opt, p, w);
//    }
//};

struct Delegate: public QStyledItemDelegate {
    Delegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}
    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const {
        auto o = option;
        initStyleOption(&o, index);
        o.decorationSize.setWidth(o.rect.width());
        auto style =  o.widget ? o.widget->style() : QApplication::style();
        style->drawControl(QStyle::CE_ItemViewItem, &o, painter, o.widget);
    }
};
