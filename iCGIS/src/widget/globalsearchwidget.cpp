#include "widget/globalsearchwidget.h"
#include "dialog/globalsearchresult.h"

#include "util/env.h"
#include "util/logger.h"
#include "geo/map/geomap.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QStringListModel>

GlobalSearchWidget::GlobalSearchWidget(QWidget *parent)
    : QLineEdit(parent), map(Env::map)
{
    setupLayout();

    this->setFont(QFont("Microsoft YaHei", 10, QFont::Normal));

    connect(this, &QLineEdit::textChanged, this,
            &GlobalSearchWidget::onTextChanged);
}

GlobalSearchWidget::~GlobalSearchWidget()
{
}

// Layout
void GlobalSearchWidget::setupLayout()
{
    stringList << "icrystal" << "error" << "icrystal0825";
    completer = new QCompleter(stringList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    this->setCompleter(completer);

    // Button: clear text in serach box
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

    // Button: search
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

    // Transparent when no foucs
    this->setStyleSheet(
        "background-color: rgba(255, 255, 255, 0%);"
        "border:1px solid rgb(200, 200, 200);"
        );
}


void GlobalSearchWidget::onClearText()
{
    this->clear();
}

void GlobalSearchWidget::onSearch()
{
    QString input = this->text();

    GeoLayer* layer = nullptr;

    int leftBracket = input.indexOf("[");
    int rightBracket = input.indexOf("]");
    if (leftBracket != -1 && rightBracket != -1) {
        // There should be at lease one character inside the brackets
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

    // Only one search result dialot
    if (!searchResultDialog) {
        searchResultDialog = new GlobalSearchResult(this->parentWidget());
        QPoint pos = this->mapToGlobal(this->pos());
        searchResultDialog->setGeometry(pos.x(), pos.y() + 25, 800, 300);
        connect(searchResultDialog, &GlobalSearchResult::sigClosed,
                this, [this]{ searchResultDialog = nullptr; });
    }
    else {
        searchResultDialog->clear();
    }

    // Layer was specified
    if (layer) {
        std::vector<GeoFeature*> features;
        if (layer->getLayerType() == kFeatureLayer) {
            GeoFeatureLayer* featureLayer = layer->toFeatureLayer();
            if (searchInLayer(input, featureLayer, features)) {
                searchResultDialog->addSearchResult(featureLayer->getLID(), features);
            }
        }
    }

    // Not specify any layer
    // Serach in the whole map
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
}

// Get focus
void GlobalSearchWidget::focusInEvent(QFocusEvent* event)
{
    this->setStyleSheet(
        "background-color: rgba(255, 255, 255, 80%);"
        "border:2px solid rgb(128, 0, 128);"
        );
    // show button in search box
    btnClearText->show();
    btnSearch->show();
    QLineEdit::focusInEvent(event);
}


// Lose focus
void GlobalSearchWidget::focusOutEvent(QFocusEvent* event)
{
    // Transparent display
    this->setStyleSheet(
        "background-color: rgba(255, 255, 255, 0%);"
        "border:1px solid rgb(200, 200, 200);"
        );

    // hide button in search box
    btnClearText->hide();
    btnSearch->hide();
    QLineEdit::focusOutEvent(event);
}

// intellisense and auto-completion
// fuzzy search: "*name*"
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

// Search in specified layer
bool GlobalSearchWidget::searchInLayer(const QString& fieldValue, GeoFeatureLayer* layerIn, std::vector<GeoFeature*>& featuresOut)
{
    int fieldsCount = layerIn->getNumFields();
    int featuresCount = layerIn->getFeatureCount();

    bool ret = false;
    for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
        GeoFeature* feature = layerIn->getFeature(iFeature);
        // Traverse all fields
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

