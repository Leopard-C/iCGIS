#include "widget/toolboxtreewidget.h"
#include "util/env.h"
#include "util/appevent.h"

#include <QIcon>
#include <QDebug>


ToolBoxTreeWidget::ToolBoxTreeWidget(QWidget* parent /*= nullptr*/)
    : QTreeWidget(parent), map(Env::map)
{
	this->setHeaderHidden(true);
	this->setStyleSheet("QTreeWidget::item{height:25px}");

	toolboxRootItem = new QTreeWidgetItem(this);
	toolboxRootItem->setIcon(0, QIcon("res/icons/toolbox.ico"));
	toolboxRootItem->setText(0, tr("ToolBox"));

	createToolItems();
    this->expandAll();

    connect(this, &QTreeWidget::itemDoubleClicked,
        this, &ToolBoxTreeWidget::onDoubleClicked);
}

ToolBoxTreeWidget::~ToolBoxTreeWidget()
{
}


void ToolBoxTreeWidget::createToolItems()
{
	QTreeWidgetItem* kernelDensityItem = new QTreeWidgetItem(toolboxRootItem);
	kernelDensityItem->setIcon(0, QIcon("res/icons/tool.ico"));
	kernelDensityItem->setText(0, tr("Kernel Density"));
}

void ToolBoxTreeWidget::onDoubleClicked(QTreeWidgetItem* item, int col)
{
	qDebug() << item->text(col);
	QString toolName = item->text(0);
	if (toolName == "Kernel Density") {
        KernelDensityTool* kernelDensityTool = new KernelDensityTool(this);
        kernelDensityTool->show();
	}
}
