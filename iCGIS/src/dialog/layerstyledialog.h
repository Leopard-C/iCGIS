/*************************************************************************
** class name:  LayerStyleDialog
**
** description: Set layer's style
**                  Single style
**					Color ramp
**                  Rule based (.sld file)
**
** last change: 2020-01-04
**************************************************************************/
#pragma once

#include <QCollator>
#include <QColorDialog>
#include <QComboBox>
#include <QCheckBox>
#include <QDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QStackedLayout>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

#include <vector>

#include "geo/map/geolayer.h"
#include "widget/colorblockwidget.h"


class LayerStyleDialog : public QDialog
{
    Q_OBJECT
public:
    LayerStyleDialog(GeoFeatureLayer* layerIn, QWidget *parent);
    ~LayerStyleDialog();

signals:
    void sigUpdateOpengl();

public slots:
    void onClassify();
    void onBtnLoadSldClicked();
    void onBtnOkClicked();
    void onBtnApplyClicked();

public:
    void createColorRampItems();
    void processSingleStyle();
    void processCategorizedStyle();
    void processRuleBasedStyle();

private:
    void setupLayout();

    // create different pages for differnt style-type
    void createPageSingleStyle();
    void createPageCategorizedStyle();
    void createPageRuleBasedStyle();

    // color ramp
    void addColorRamp(const QColor& startColor, const QColor& endColor);
    void addRandomColorRamp();

    // classify, and put data in classifyResultWidget
    // 1. random color
    template<typename T>
    void classifyRandomColor(int fieldIndex);
    // 2. color ramp
    template<typename T>
    void classify(int fieldIndex, const QColor& startColor, const QColor& endColor);

    template<typename T>
    void insertClassifyItem(QTableWidget* tableWidget, int row, int FID, const QColor& color, T value);

private:
    GeoFeatureLayer* layer = nullptr;

    std::vector<std::pair<QColor, QColor>> colorPairs;

    QComboBox* modeComboBox = nullptr;
    QVBoxLayout* mainLayout = nullptr;
    QStackedLayout* stackedLayout = nullptr;

    QPushButton* btnLoadSLD;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QPushButton* btnApply;

    // Single style
    QWidget* singleStyleWidget;
    QColorDialog* colorSelectWidget;

    // Classify style
    QWidget* categorizedStyleWidget;
    QComboBox* classifyFieldComboBox;
    QComboBox* colorRampComboBox;
    QPushButton* btnClassify;
    QTableWidget* classifyResultWidget;

    // Rule based (read .sld file)
    QWidget* ruleBaseStyleWidget;
    QLabel*  filedNameLabel;
    QTableWidget* loadSldResultWidget;
};

// Random color
template<typename T>
void LayerStyleDialog::classifyRandomColor(int fieldIndex) {
    int featuresCount = layer->getFeatureCount();

    // put date to classifyResultWidget
    for (int i = 0; i < featuresCount; ++i) {
        GeoFeature* feature = layer->getFeature(i);
        T value;
        feature->getField(fieldIndex, &value);
        QColor color(rand() % 256, rand() % 256, rand() % 256);
        insertClassifyItem(classifyResultWidget, i, feature->getFID(), color, value);
    }
}


// Color ramp
template<typename T>
void LayerStyleDialog::classify(int fieldIndex, const QColor& startColor, const QColor& endColor)
{
    int featuresCount = layer->getFeatureCount();
    std::vector<std::pair<GeoFeature*, T>> pairs;
    pairs.reserve(featuresCount);

    // Get all values of the specified field
    for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
        GeoFeature* feature = layer->getFeature(iFeature);
        T fieldValue;
        feature->getField(fieldIndex, &fieldValue);
        pairs.emplace_back(feature, fieldValue);
    }

    // evaluate the value of the function or variable at compile time
    //  Used in an object declaration or non-static member function (until C++14) implies const.
    //  Used in a function or static member variable (since C++17) declaration implies inline.

    // assending sortk
    if constexpr (std::is_same<T, QString>::value) {
        // order strings
        // Support for chinese string. But it is not perfect.
        QLocale loc(QLocale::Chinese, QLocale::China);
        QCollator qcol(loc);
        std::sort(pairs.begin(), pairs.end(),
                  [&qcol](const std::pair<GeoFeature*, T>& in1, const std::pair<GeoFeature*, T>& in2) {
                      return qcol.compare(in1.second, in2.second) < 0;
                  });
    }
    else {
        std::sort(pairs.begin(), pairs.end(),
                  [](const std::pair<GeoFeature*, T>& in1, const std::pair<GeoFeature*, T>& in2) {
                      return in1.second < in2.second;
                  });
    }

    // R, G, B increment
    double deltaR = (endColor.red() - startColor.red()) / featuresCount;
    double deltaG = (endColor.green() - startColor.green()) / featuresCount;
    double deltaB = (endColor.blue() - startColor.blue()) / featuresCount;

    for (int i = 0; i < featuresCount; ++i) {
        GeoFeature* feature = pairs[i].first;
        T value = pairs[i].second;
        QColor color(
            startColor.red() + int(deltaR * i),
            startColor.green() + int(deltaG * i),
            startColor.blue() + int(deltaB * i)
            );
        insertClassifyItem(classifyResultWidget, i, feature->getFID(), color, value);
    }
}


template<typename T>
void LayerStyleDialog::insertClassifyItem(QTableWidget* tableWidget, int row, int nFID, const QColor& color, T value)
{
    // column 0
    QWidget* widget = new QWidget(categorizedStyleWidget);
    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setSpacing(0);
    layout->setMargin(5);
    QCheckBox* checkBox = new QCheckBox(widget);
    checkBox->setCheckState(Qt::Checked);
    layout->addWidget(checkBox);
    ColorBlockWidget* colorWidget = new ColorBlockWidget(widget);
    colorWidget->setFID(nFID);
    colorWidget->setColor(color);
    layout->addWidget(colorWidget);
    tableWidget->setCellWidget(row, 0, widget);

    // column 1
    // column 2
    QTableWidgetItem* item1 = new QTableWidgetItem();
    QTableWidgetItem* item2 = new QTableWidgetItem();
    if constexpr (std::is_same<T, QString>::value) {
        item1->setText(value);
        item2->setText(value);
    }
    else {
        item1->setText(QString::number(value));
        item2->setText(QString::number(value));
    }
    tableWidget->setItem(row, 1, item1);
    tableWidget->setItem(row, 2, item2);
}
