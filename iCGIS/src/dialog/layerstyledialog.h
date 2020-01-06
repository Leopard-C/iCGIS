/*************************************************************************
** class name:  LayerStyleDialog
**
** description: 图层样式设置 对话框。支持：
**					单一样式：整个图层一种样式
**					分类样式：指定字段，指定颜色渐变，不同字段值不同样式
**
** last change: 2020-01-04
**************************************************************************/
#pragma once

#include <QCollator>
#include <QColorDialog>
#include <QComboBox>
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

#include "geo/map/geolayer.h"

class OpenGLWidget;


class LayerStyleDialog : public QDialog
{
	Q_OBJECT

public:
	LayerStyleDialog(GeoFeatureLayer* layerIn, OpenGLWidget* openglWidget, QWidget *parent);
	~LayerStyleDialog();

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
	// 总体布局
	void setupLayout();

	// 创建不同页面
	void createPageSingleStyle();
	void createPageCategorizedStyle();
	void createPageRuleBasedStyle();

	// 下拉列表添加颜色渐变条
	void addColorRamp(const QColor& startColor, const QColor& endColor);
	void addRandomColorRamp();

	// 分类，并将结果写入classifyResultWidget
	// 1. 随机颜色
	template<typename T>
	void classifyRandomColor(int fieldIndex);
	// 2. 颜色渐变
	template<typename T>
	void classify(int fieldIndex, const QColor& startColor, const QColor& endColor);

	// 插入一条分类结果
	template<typename T>
	void insertClassifyItem(QTableWidget* tableWidget, int row, int FID, const QColor& color, T value);

private:
	GeoFeatureLayer* layer = nullptr;
	OpenGLWidget* openglWidget = nullptr;

	std::vector<std::pair<QColor, QColor>> colorPairs;

	QComboBox* modeComboBox = nullptr;
	QVBoxLayout* mainLayout = nullptr;
	QStackedLayout* stackedLayout = nullptr;

	QPushButton* btnLoadSLD;
	QPushButton* btnOk;
	QPushButton* btnCancel;
	QPushButton* btnApply;

	// 单一样式
	QWidget* singleStyleWidget;
	QColorDialog* colorSelectWidget;

	// 分类样式
	QWidget* categorizedStyleWidget;
	QComboBox* classifyFieldComboBox;
	QComboBox* colorRampComboBox;
	QPushButton* btnClassify;
	QTableWidget* classifyResultWidget;

	// 基于规则的样式(从SLD文件读取）
	QWidget* ruleBaseStyleWidget;
	QLabel*  filedNameLabel;
	QTableWidget* loadSldResultWidget;
};


// 随机颜色分类
template<typename T>
void LayerStyleDialog::classifyRandomColor(int fieldIndex)
{
	int featuresCount = layer->getFeatureCount();

	// 写入分类结果到classifyResultWidget
	for (int i = 0; i < featuresCount; ++i) {
		GeoFeature* feature = layer->getFeature(i);
		T value;
		feature->getField(fieldIndex, &value);
		QColor color(rand() % 256, rand() % 256, rand() % 256);
		insertClassifyItem(classifyResultWidget, i, feature->getFID(), color, value);
	}
}


// 指定字段和颜色渐变样式 进行分类
template<typename T>
void LayerStyleDialog::classify(int fieldIndex, const QColor& startColor, const QColor& endColor)
{
	int featuresCount = layer->getFeatureCount();
	std::vector<std::pair<GeoFeature*, T>> pairs;
	pairs.reserve(featuresCount);

	// 获取所有字段值
	for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
		GeoFeature* feature = layer->getFeature(iFeature);
		T fieldValue;
		feature->getField(fieldIndex, &fieldValue);
		pairs.emplace_back(feature, fieldValue);
	}

	// constexpr
	// 编译时类型判断（C++17标准）
	// 只编译符合条件的代码

	// 从小到大排序
	if constexpr (std::is_same<T, QString>::value) {
		// 字符串排序
		// 支持中文排序
		// "重庆"，总是排在最后，“zhong庆” ？
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

	// R, G, B增量
	double deltaR = (endColor.red() - startColor.red()) / featuresCount;
	double deltaG = (endColor.green() - startColor.green()) / featuresCount;
	double deltaB = (endColor.blue() - startColor.blue()) / featuresCount;

	// 写入分类结果到classifyResultWidget
	for (int i = 0; i < featuresCount; ++i) {
		GeoFeature* feature = pairs[i].first;
		T value = pairs[i].second;
		QColor color(
			startColor.red() + deltaR * i,
			startColor.green() + deltaG * i,
			startColor.blue() + deltaB * i
		);
		insertClassifyItem(classifyResultWidget, i, feature->getFID(), color, value);
	}
}


// 插入一条分类结果
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
	// 编译时类型判断
	// 只编译符合添加的代码
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
