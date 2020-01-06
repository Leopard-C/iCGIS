#include "globalsearchwidget.h"

#include "logger.h"
#include "dialog/globalsearchresult.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QStringListModel>

GlobalSearchWidget::GlobalSearchWidget(GeoMap* mapIn, QWidget *parent)
	: map(mapIn), QLineEdit(parent)
{
	setupLayout();

	this->setFont(QFont("Microsoft YaHei", 10, QFont::Normal));

	connect(this, &QLineEdit::textChanged, this, 
		&GlobalSearchWidget::onTextChanged);
}

GlobalSearchWidget::~GlobalSearchWidget()
{
}

// 总体布局
void GlobalSearchWidget::setupLayout()
{
	stringList << "icrystal" << "error" << "icrystal0825";
	completer = new QCompleter(stringList, this);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	this->setCompleter(completer);

	// 按钮 清除文字
	btnClearText = new QPushButton(this);
	btnClearText->setFixedSize(15, 15);
	btnClearText->setStyleSheet("border:0px");
	btnClearText->setFocusPolicy(Qt::NoFocus);
	btnClearText->setCursor(QCursor(Qt::ArrowCursor));
	btnClearText->setStyleSheet("QPushButton{border-image: url(res/icons/clear-text.png);}"
		"QPushButton:hover{border-image: url(res/icons/clear-text-hover.png);}"
	);
	btnClearText->hide();
	connect(btnClearText, &QPushButton::clicked,
		this, [this](void) { this->clear(); });

	// 按钮 搜索
	btnSearch = new QPushButton(this);
	btnSearch->setFixedSize(18, 18);
	btnSearch->setStyleSheet("border:0px");
	btnSearch->setFocusPolicy(Qt::NoFocus);
	btnSearch->setCursor(QCursor(Qt::PointingHandCursor));
	btnSearch->hide();
	btnSearch->setStyleSheet("QPushButton{border-image: url(res/icons/search.png);}"
		"QPushButton:hover{border-image: url(res/icons/search-hover.png);}"
		"QPushButton:pressed{border-image: url(res/icons/search-press.png);}"
	);
	connect(this, &QLineEdit::returnPressed, this, &GlobalSearchWidget::onSearch);
	connect(btnSearch, &QPushButton::clicked, this, &GlobalSearchWidget::onSearch);

	QMargins textMargins = this->textMargins();
	this->setTextMargins(textMargins.left(), textMargins.top(),
		btnSearch->width() + btnClearText->width(), textMargins.bottom());

	QHBoxLayout* layout = new QHBoxLayout();
	layout->addStretch();
	layout->addWidget(btnClearText);
	layout->addWidget(btnSearch);
	layout->setSpacing(0);
	layout->setContentsMargins(1, 1, 1, 1);
	this->setLayout(layout);

	// 没有焦点时透明显示
	this->setStyleSheet(
		"background-color: rgba(255, 255, 255, 0%);"
		"border:1px solid rgb(200, 200, 200);"
	);
}


void GlobalSearchWidget::onClearText()
{
	this->clear();
}

// 执行搜索
// 整个地图中搜索
void GlobalSearchWidget::onSearch()
{
	QString input = this->text();

	GeoLayer* layer = nullptr;

	// 剔除【左右中括号之间的内容（所属的图层名）】
	int leftBracket = input.indexOf("[");
	int rightBracket = input.indexOf("]");
	if (leftBracket != -1 && rightBracket != -1) {
		// 中括号中至少一个字符
		if (rightBracket - leftBracket > 1) {
			QString layerName = input.mid(leftBracket + 1, rightBracket - leftBracket - 1);
			qDebug() << layerName;
			if (!layerName.isEmpty()) {
				layer = map->getLayerByName(layerName);
			}
		}
		input = input.left(leftBracket) + input.right(input.length() - rightBracket - 1);
		input = input.trimmed();
	}

	// 只允许有一个searchResultDialog
	if (!searchResultDialog) {
		searchResultDialog = new GlobalSearchResult(map, openglWidget, this->parentWidget());
		QPoint pos = this->mapToGlobal(this->pos());
		searchResultDialog->setGeometry(pos.x(), pos.y() + 25, 800, 300);
		connect(searchResultDialog, &GlobalSearchResult::closed,
			this, &GlobalSearchWidget::onSearchResultDialogClose);
	}
	else {
		searchResultDialog->clear();
	}

	// 指定了搜索图层
	if (layer) {
		std::vector<GeoFeature*> features;
		if (layer->getLayerType() == kFeatureLayer) {
			GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
			if (searchInLayer(input, featureLayer, features)) {
				searchResultDialog->addSearchResult(featureLayer->getLID(), features);
			}
		}
	}
	
	// 未指定搜索图层，在整个地图中搜索
	else {
		int layersCount = map->getNumLayers();
		for (int iLayer = 0; iLayer < layersCount; ++iLayer) {
			layer = map->getLayerById(iLayer);
			std::vector<GeoFeature*> features;
			if (layer->getLayerType() == kFeatureLayer) {
				GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
				if (searchInLayer(input, featureLayer, features)) {
					searchResultDialog->addSearchResult(featureLayer->getLID(), features);
				}
			}
		}
	}

	searchResultDialog->setupLayout();
	searchResultDialog->show();
}


void GlobalSearchWidget::onTextChanged()
{
	Log("Changed");
	if (this->text().indexOf("icrystal") != -1) {
		Log("Here");
	}
	else {
		Log("Not Here");
	}
}

void GlobalSearchWidget::onSearchResultDialogClose()
{
	searchResultDialog = nullptr;
}

// 有焦点时
void GlobalSearchWidget::focusInEvent(QFocusEvent* event)
{
	this->setStyleSheet(
		"background-color: rgba(255, 255, 255, 80%);"
		"border:2px solid rgb(128, 0, 128);"
	);
	// 显示按钮
	btnClearText->show();
	btnSearch->show();
	QLineEdit::focusInEvent(event);
}


// 失去焦点时
void GlobalSearchWidget::focusOutEvent(QFocusEvent* event)
{
	// 没有焦点时透明显示
	this->setStyleSheet(
		"background-color: rgba(255, 255, 255, 0%);"
		"border:1px solid rgb(200, 200, 200);"
	);

	// 隐藏按钮
	btnClearText->hide();
	btnSearch->hide();
	QLineEdit::focusOutEvent(event);
}

// 更新 匹配文字
// 智能提示 + 自动补全
// 暂时使用字段名符合 "*name*" 的字段（大小写不敏感）
void GlobalSearchWidget::updateCompleterList()
{
	QStringListModel* model = (QStringListModel*)(completer->model());
	if (!model)
		return;

	stringList.clear();
	int layersCount = map->getNumLayers();
	GeoLayer* layer;
	GeoFeature* feature;
	QString fieldName;
	for (int iLayer = 0; iLayer < layersCount; ++iLayer) {
		layer = map->getLayerById(iLayer);
		if (layer->getLayerType() != kFeatureLayer)
			continue;

		GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
		int searchFieldIndex = featureLayer->getFieldIndex("name", Qt::CaseSensitive);
		if (searchFieldIndex == -1) {
			searchFieldIndex = featureLayer->getFieldIndexLike("name", Qt::CaseSensitive);
			if (searchFieldIndex == -1) {
				continue;
			}
		}

		if (featureLayer->getFieldDefn(searchFieldIndex)->getType() != kFieldText)
			continue;

		QString layerName = featureLayer->getName();
		int featuresCount = featureLayer->getFeatureCount();
		for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
			feature = featureLayer->getFeature(iFeature);
			QString nameValue;
			feature->getField(searchFieldIndex, &nameValue);
			nameValue = nameValue + " [" + layerName + "]";
			stringList << nameValue;
		}
	}

	model->setStringList(stringList);
}

// 在指定图层中执行搜索
bool GlobalSearchWidget::searchInLayer(const QString& fieldValue, GeoFeatureLayer* layerIn, std::vector<GeoFeature*>& featuresOut)
{
	int fieldsCount = layerIn->getNumFields();
	int featuresCount = layerIn->getFeatureCount();

	bool ret = false;
	for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
		GeoFeature* feature = layerIn->getFeature(iFeature);
		// 遍历字段的每一列
		for (int iField = 0; iField < fieldsCount; ++iField) {
			GeoFieldDefn* fieldDefn = layerIn->getFieldDefn(iField);
			if (fieldDefn->getType() == kFieldText) {
				QString value;
				feature->getField(iField, &value);
				if (value.contains(fieldValue, Qt::CaseInsensitive)) {
					featuresOut.push_back(feature);
					ret = true;
					break;
				}
			}
		} // end for iField
	} // end for iFeature

	return ret;
}


