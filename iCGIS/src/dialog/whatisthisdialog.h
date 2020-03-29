/*******************************************************
** class name:  WhatIsThisDialog
**
** description: show current selected feature's info
**
** last change: 2020-03-26
*******************************************************/
#ifndef WHATISTHISDIALOG_H
#define WHATISTHISDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QHBoxLayout>

class GeoFeatureLayer;
class GeoFeature;


class WhatIsThisDialog : public QDialog
{
    Q_OBJECT
public:
    WhatIsThisDialog(QWidget* parent = nullptr);

public:
    void setupLayout();
    void setFeature(GeoFeatureLayer* layer, GeoFeature* feature);

signals:
    void closed();

protected:
    virtual void closeEvent(QCloseEvent *) override;

private:
    // feature
    QLabel* labelFID;
    QLabel* labelExtentMinX;
    QLabel* labelExtentMaxX;
    QLabel* labelExtentMinY;
    QLabel* labelExtentMaxY;
    QLabel* labelGeomType;

    // parent
    // belong to layer
    QLabel* labelLayerName;

    // Button
    // Show more info
    QPushButton* btnShowMore;

    // attribute
    QTableWidget* attributeTable;
	QHBoxLayout* attributeTableLayout;
};

#endif // WHATISTHISDIALOG_H
