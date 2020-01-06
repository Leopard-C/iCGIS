#include "widget/toolboxtreewidget.h"
#include "widget/layerstreewidget.h"
#include "widget/openglwidget.h"

#include <QIcon>
#include <QDebug>


ToolBoxTreeWidget::ToolBoxTreeWidget(GeoMap* mapIn, QWidget* parent /*= nullptr*/)
	: QTreeWidget(parent), map(mapIn)
{
	this->setHeaderHidden(true);
	this->setStyleSheet("QTreeWidget::item{height:25px}");

	toolboxRootItem = new QTreeWidgetItem(this);
	toolboxRootItem->setIcon(0, QIcon("res/icons/toolbox.ico"));
	toolboxRootItem->setText(0, tr("ToolBox"));

	createToolItems();

	this->expandAll();
}

ToolBoxTreeWidget::~ToolBoxTreeWidget()
{
}


void ToolBoxTreeWidget::createToolItems()
{
	QTreeWidgetItem* kernelDensityItem = new QTreeWidgetItem(toolboxRootItem);
	kernelDensityItem->setIcon(0, QIcon("res/icons/tool.ico"));
	kernelDensityItem->setText(0, tr("Kernel Density"));
	connect(this, &QTreeWidget::itemDoubleClicked, 
		this, &ToolBoxTreeWidget::onDoubleClicked);
}

void ToolBoxTreeWidget::onDoubleClicked(QTreeWidgetItem* item, int col)
{
	qDebug() << item->text(col);
	QString toolName = item->text(0);
	if (toolName == "Kernel Density") {
		KernelDensityTool* kernelDensityTool = new KernelDensityTool(map, this);
		connect(kernelDensityTool, &KernelDensityTool::addedTiffToMap,
			[this](GeoRasterLayer* rasterLayer) {
				layersTreeWidget->insertNewItem(rasterLayer);
				openglWidget->sendDataToGPU(rasterLayer);
			}
		);
	}
}

