#include "dialog/layerattributetabledialog.h"
#include "widget/openglwidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QHeaderView>

//#include <thread>

#include <set>


LayerAttributeTableDialog::LayerAttributeTableDialog(GeoFeatureLayer* layerIn, OpenGLWidget* openglWidget, QWidget *parent)
	: QDialog(parent), layer(layerIn), openglWidget(openglWidget)
{
	// 最大化和最小化按钮
    Qt::WindowFlags windowFlag  = Qt::Dialog;
    windowFlag |= Qt::WindowMinimizeButtonHint;
    windowFlag |= Qt::WindowMaximizeButtonHint;
    windowFlag |= Qt::WindowCloseButtonHint;
    this->setWindowFlags(windowFlag);
	this->setWindowTitle(tr("Attribute Table"));
	this->setAttribute(Qt::WA_DeleteOnClose, true);		// 关闭即销毁

	createToolBar();
	createWidgets();
	createActions();
	setupLayout();

	readAttributeTable(layer, tableWidget);

	// 开启新线程读取属性表
	// TODO...
	//std::thread t(readAttributeTable, layer, tableWidget);
	//t.detach();
}


LayerAttributeTableDialog::~LayerAttributeTableDialog()
{
}


/*********************************/
/*                               */
/*          Initialize           */
/*                               */
/*********************************/

void LayerAttributeTableDialog::createWidgets()
{
	tableWidget = new QTableWidget(this);
	connect(tableWidget->verticalHeader(), &QHeaderView::sectionClicked,
		this, &LayerAttributeTableDialog::onSelectRows);
}

void LayerAttributeTableDialog::createActions()
{
	removeRecorsdAction = new QAction(tr("Remove selected"), this);
	removeRecorsdAction->setIcon(QIcon("res/icons/remove.ico"));
	toolBar->addAction(removeRecorsdAction);
}

// 工具栏
void LayerAttributeTableDialog::createToolBar()
{
	toolBar = new QToolBar(this);
	toolBar->setFixedHeight(20);
}

void LayerAttributeTableDialog::setupLayout()
{
	QVBoxLayout* mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(5);
	mainLayout->addWidget(toolBar);
	mainLayout->addWidget(tableWidget);
}

// 读取属性表
void LayerAttributeTableDialog::readAttributeTable(GeoFeatureLayer* layer, QTableWidget* tableWidget)
{
	if (!layer || layer->isEmpty())
		return;

	int featuresCount = layer->getFeatureCount();
	int fieldsCount = layer->getNumFields();

	tableWidget->clear();
	tableWidget->setAlternatingRowColors(true);
	tableWidget->setColumnCount(fieldsCount + 1);
	tableWidget->setRowCount(featuresCount);
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableWidget->horizontalHeader()->setFixedHeight(20);
	tableWidget->horizontalHeader()->setStyleSheet(
		"QHeaderView::section{background:rgb(200,200,200);}"
	);

	QStringList header;
	header << "FID";
	for (int i = 0; i < fieldsCount; ++i) {
		header << layer->getFieldDefn(i)->getName();
	}
	tableWidget->setHorizontalHeaderLabels(header);

	// 遍历图层中每一个feature
	for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
		GeoFeature* feature = layer->getFeature(iFeature);
		if (!feature) 
			continue;

		// 第一列（FID）
		QTableWidgetItem* item = new QTableWidgetItem();
		item->setTextAlignment(Qt::AlignRight);
		item->setText(QString::number(feature->getFID()));
		tableWidget->setItem(iFeature, 0, item);

		// 遍历字段的每一列
		for (int iCol = 0; iCol < fieldsCount; ++iCol) {
			QTableWidgetItem* item = new QTableWidgetItem();
			GeoFieldDefn* fieldDefn = layer->getFieldDefn(iCol);

			switch (fieldDefn->getType()) {
			default:
				break;
			case kFieldInt: {
				item->setTextAlignment(Qt::AlignRight);
				int value;
				feature->getField(iCol, &value);
				item->setText(QString::number(value));
				break;
			}
			case kFieldDouble: {
				item->setTextAlignment(Qt::AlignRight);
				double value;
				feature->getField(iCol, &value);
				item->setText(QString::number(value));
				break;
			}
			case kFieldText: {
				item->setTextAlignment(Qt::AlignLeft);
				QString value;
				feature->getField(iCol, &value);
				item->setText(value);
				break;
			}
			} // end switch

			tableWidget->setItem(iFeature, iCol + 1, item);
		} // end for iCol
	}
}

// 选中一行（多行）
// 点击左侧竖直表头
void LayerAttributeTableDialog::onSelectRows()
{
	auto selectedItems = tableWidget->selectedItems();

	// std::set 集合
	// 元素是唯一的
	std::set<int> rowsIndexes;

	// 所有选中的行
	for (const auto& item : selectedItems) {
		rowsIndexes.insert(item->row());
	}

	int rowsCount = rowsIndexes.size();

	std::vector<int> nFIDs;
	nFIDs.reserve(rowsCount);

	for (auto idx : rowsIndexes) {
		nFIDs.push_back(tableWidget->item(idx, 0)->text().toInt());
	}

	openglWidget->onSelectFeatures(layer->getLID(), nFIDs);
}

