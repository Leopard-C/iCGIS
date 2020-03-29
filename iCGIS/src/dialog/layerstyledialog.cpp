#include "dialog/layerstyledialog.h"
#include "util/logger.h"
#include "util/utility.h"
#include "util/appevent.h"

#include "widget/colorblockwidget.h"
#include "geo/utility/filereader.h"

#include <QCheckBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QIcon>
#include <QLabel>
#include <QLinearGradient>
#include <QMessageBox>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QSpacerItem>
#include <QString>
#include <QStringList>
#include <QStyledItemDelegate>


LayerStyleDialog::LayerStyleDialog(GeoFeatureLayer* layerIn, QWidget *parent)
    : QDialog(parent), layer(layerIn)
{
    this->setWindowTitle(tr("Styles"));
    this->setAttribute(Qt::WA_DeleteOnClose, true);

    // color pairs
    colorPairs.reserve(4);
    colorPairs.emplace_back(QColor(230, 190, 255), QColor(255, 0, 255));
    colorPairs.emplace_back(QColor(200, 0, 0 ),	   QColor(125, 25, 25));
    colorPairs.emplace_back(QColor(170, 255, 170), QColor(0, 255, 0));
    colorPairs.emplace_back(QColor(0, 255, 0),     QColor(255, 0, 0));

    setupLayout();

    connect(this, &LayerStyleDialog::sigUpdateOpengl,
            AppEvent::getInstance(), &AppEvent::onUpdateOpengl);
}


LayerStyleDialog::~LayerStyleDialog()
{
}

// Layout
void LayerStyleDialog::setupLayout()
{
    mainLayout = new QVBoxLayout(this);

    modeComboBox = new QComboBox(this);
    QStyledItemDelegate* delegate = new QStyledItemDelegate(this);
    modeComboBox->setItemDelegate(delegate);
    QStringList modeList;
    modeList << "Single symbol" << "Categorized" << "Rule-Based";
    modeComboBox->setEditable(false);
    modeComboBox->addItems(modeList);
    mainLayout->addWidget(modeComboBox);

    // create pages for differnt style-type
    createPageSingleStyle();
    createPageCategorizedStyle();
    createPageRuleBasedStyle();

    stackedLayout = new QStackedLayout();
    stackedLayout->addWidget(singleStyleWidget);
    stackedLayout->addWidget(categorizedStyleWidget);
    stackedLayout->addWidget(ruleBaseStyleWidget);
    mainLayout->addLayout(stackedLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLoadSLD = new QPushButton(tr("Load Style"), this);
    btnOk = new QPushButton(tr("OK"), this);
    btnCancel = new QPushButton(tr("Cancel"), this);
    btnApply = new QPushButton(tr("Apply"), this);
    btnLayout->addWidget(btnLoadSLD);
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnApply);
    mainLayout->addLayout(btnLayout);
    // respond to the enter key
    btnOk->setFocus();
    btnOk->setDefault(true);

    connect(modeComboBox, SIGNAL(currentIndexChanged(int)),
            stackedLayout, SLOT(setCurrentIndex(int)));
    connect(btnLoadSLD, &QPushButton::clicked,
            this, &LayerStyleDialog::onBtnLoadSldClicked);
    connect(btnOk, &QPushButton::clicked,
            this, &LayerStyleDialog::onBtnOkClicked);
    connect(btnApply, &QPushButton::clicked,
            this, &LayerStyleDialog::onBtnApplyClicked);
    connect(btnCancel, &QPushButton::clicked,
            this, &LayerStyleDialog::close);

    modeComboBox->setCurrentIndex(layer->getStyleMode());
}

// Single style
void LayerStyleDialog::createPageSingleStyle()
{
    singleStyleWidget = new QWidget(this);
    QHBoxLayout* layout = new QHBoxLayout(singleStyleWidget);
    colorSelectWidget = new QColorDialog(this);
    colorSelectWidget->setWindowFlag(Qt::Widget);
    colorSelectWidget->setOptions(QColorDialog::DontUseNativeDialog | QColorDialog::NoButtons);
    layout->addWidget(colorSelectWidget);
}

// Classify style
void LayerStyleDialog::createPageCategorizedStyle()
{
    categorizedStyleWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(categorizedStyleWidget);
    QFormLayout* formLayout = new QFormLayout();
    //formLayout->setLabelAlignment(Qt::AlignRight);

    QLabel* label1 = new QLabel(tr("Field"), categorizedStyleWidget);
    classifyFieldComboBox = new QComboBox(categorizedStyleWidget);
    classifyFieldComboBox->setEditable(false);
    classifyFieldComboBox->addItems(layer->getFieldList());
    formLayout->addRow(label1, classifyFieldComboBox);

    QLabel* label2 = new QLabel(tr("Color ramp"), categorizedStyleWidget);
    colorRampComboBox = new QComboBox(categorizedStyleWidget);
    createColorRampItems();
    colorRampComboBox->setEditable(false);
    QStyledItemDelegate* delegate = new QStyledItemDelegate(this);
    colorRampComboBox->setItemDelegate(delegate);
    formLayout->addRow(label2, colorRampComboBox);

    classifyResultWidget = new QTableWidget(categorizedStyleWidget);
    classifyResultWidget->setShowGrid(false);
    classifyResultWidget->horizontalHeader()->setFixedHeight(20);
    classifyResultWidget->horizontalHeader()->setStretchLastSection(true);
    classifyResultWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    classifyResultWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    classifyResultWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section{background:rgb(200,200,200);}"
        );
    classifyResultWidget->setColumnCount(3);
    QStringList header;
    header << "Symbol" << "Value" << "legend";
    classifyResultWidget->setHorizontalHeaderLabels(header);
    classifyResultWidget->setColumnWidth(0, 50);
    classifyResultWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    layout->addLayout(formLayout);
    layout->addWidget(classifyResultWidget);

    QHBoxLayout* hLayout = new QHBoxLayout();
    btnClassify = new QPushButton(tr("Classify"), categorizedStyleWidget);
    connect(btnClassify, &QPushButton::clicked, this, &LayerStyleDialog::onClassify);
    hLayout->addWidget(btnClassify);
    layout->addLayout(hLayout);
}


// Rule-based (read .sld file)
void LayerStyleDialog::createPageRuleBasedStyle()
{
    ruleBaseStyleWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(ruleBaseStyleWidget);
    QHBoxLayout* hLayout = new QHBoxLayout();
    QLabel* label = new QLabel(tr("Field: "), ruleBaseStyleWidget);
    filedNameLabel = new QLabel(ruleBaseStyleWidget);
    hLayout->addWidget(label);
    hLayout->addWidget(filedNameLabel);
    hLayout->addStretch();
    layout->addLayout(hLayout);

    loadSldResultWidget = new QTableWidget(ruleBaseStyleWidget);
    loadSldResultWidget->setShowGrid(false);
    loadSldResultWidget->horizontalHeader()->setFixedHeight(20);
    loadSldResultWidget->horizontalHeader()->setStretchLastSection(true);
    loadSldResultWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    loadSldResultWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    loadSldResultWidget->horizontalHeader()->setStyleSheet(
        "QHeaderView::section{background:rgb(200,200,200);}"
        );
    loadSldResultWidget->setColumnCount(3);
    QStringList header;
    header << "Symbol" << "Value" << "legend";
    loadSldResultWidget->setHorizontalHeaderLabels(header);
    loadSldResultWidget->setColumnWidth(0, 50);
    loadSldResultWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    layout->addWidget(loadSldResultWidget);
}


// create color ramp bar
void LayerStyleDialog::createColorRampItems()
{
    // random color
    addRandomColorRamp();

    // color ramp
    int colorPairsCount = colorPairs.size();
    for (int i = 0; i < colorPairsCount; ++i) {
        addColorRamp(colorPairs[i].first, colorPairs[i].second);
    }
}

void LayerStyleDialog::addColorRamp(const QColor& startColor, const QColor& endColor)
{
    int itemWidth = 470;
    int itemHeight = 20;

    QPixmap colorRamp(itemWidth, itemHeight);
    colorRamp.fill(QColor(Qt::white));
    QPainter p(&colorRamp);
    p.setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient grad(0, itemHeight / 2, itemWidth, itemHeight / 2);
    grad.setColorAt(1.0, endColor);
    grad.setColorAt(0.0, startColor);
    p.fillRect(QRect(0, 0, itemWidth, itemHeight), grad);

    QIcon icon;
    icon.addPixmap(colorRamp, QIcon::Normal, QIcon::On);

    colorRampComboBox->addItem(icon, nullptr);
    colorRampComboBox->setIconSize(QSize(itemWidth, itemHeight));
}

void LayerStyleDialog::addRandomColorRamp()
{
    int itemWidth = 470;
    int itemHeight = 20;

    QPixmap colorRamp(itemWidth, itemHeight);
    colorRamp.fill(QColor(Qt::white));
    QPainter p(&colorRamp);
    p.setRenderHint(QPainter::Antialiasing, true);
    QLinearGradient grad(0, itemHeight / 2, itemWidth, itemHeight / 2);
    for (double i = 0.0; i < 1.0; i += 0.02) {
        grad.setColorAt(i, QColor(rand() % 256, rand() % 256, rand() % 256));
    }
    p.fillRect(QRect(0, 0, itemWidth, itemHeight), grad);

    QIcon icon;
    icon.addPixmap(colorRamp, QIcon::Normal, QIcon::On);

    colorRampComboBox->addItem(icon, nullptr);
    colorRampComboBox->setIconSize(QSize(itemWidth, itemHeight));
}



/**************************************/
/*                                    */
/*            Slots                   */
/*                                    */
/**************************************/

// Button: OK
//  = Apply + Close
void LayerStyleDialog::onBtnOkClicked()
{
    onBtnApplyClicked();
    this->close();
}


// Button: Apply
void LayerStyleDialog::onBtnApplyClicked()
{
    int modeIndex = modeComboBox->currentIndex();
    switch (modeIndex) {
    default:
        break;
    case 0:
        processSingleStyle();
        break;
    case 1:
        processCategorizedStyle();
        break;
    case 2:
        processRuleBasedStyle();
        break;
    }

    layer->setStyleMode(LayerStyleMode(modeIndex));
}

// Do classify
void LayerStyleDialog::onClassify()
{
    int featuresCount = layer->getFeatureCount();
    classifyResultWidget->clearContents();
    classifyResultWidget->setRowCount(featuresCount);

    int fieldIndex = classifyFieldComboBox->currentIndex();
    GeoFieldDefn* fieldDefn = layer->getFieldDefn(fieldIndex);

    int colorRampIndex = colorRampComboBox->currentIndex();

    switch (fieldDefn->getType()) {
    default:
        break;
    case kFieldInt:
        if (colorRampIndex == 0)
            classifyRandomColor<int>(fieldIndex);
        else
            classify<int>(fieldIndex,
                          colorPairs[colorRampIndex - 1].first, colorPairs[colorRampIndex - 1].second);
        break;
    case kFieldDouble:
        if (colorRampIndex == 0)
            classifyRandomColor<double>(fieldIndex);
        else
            classify<double>(fieldIndex,
                             colorPairs[colorRampIndex - 1].first, colorPairs[colorRampIndex - 1].second);
        break;
    case kFieldText:
        if (colorRampIndex == 0)
            classifyRandomColor<QString>(fieldIndex);
        else
            classify<QString>(fieldIndex,
                              colorPairs[colorRampIndex - 1].first, colorPairs[colorRampIndex - 1].second);
        break;
    }
}

// 1. single style
void LayerStyleDialog::processSingleStyle()
{
    QColor color = colorSelectWidget->currentColor();

    int featuresCount = layer->getFeatureCount();

    int r = color.red();
    int g = color.green();
    int b = color.blue();

    for (int i = 0; i < featuresCount; ++i) {
        GeoFeature* feature = layer->getFeature(i);
        feature->setColor(r, g, b, true);
    }
    emit sigUpdateOpengl();
}

// 2. classify style
void LayerStyleDialog::processCategorizedStyle()
{
    int featuresCount = layer->getFeatureCount();

    if (classifyResultWidget->rowCount() != featuresCount)
        return;

    for (int iRow = 0; iRow < featuresCount; ++iRow) {
        GeoFeature* feature = layer->getFeature(iRow);

        const QWidget* widget = classifyResultWidget->cellWidget(iRow, 0);
        const auto& children  = widget->children();

        // 3 children: layout + checkbox + widget(color block)
        //QHBoxLayout* hLayout = (QHBoxLayout*)children.at(0);
        //QCheckBox* checkBox = (QCheckBox*)children.at(1);
        ColorBlockWidget* colorWidget = (ColorBlockWidget*)children.at(2);

        // color-block's color
        QColor color = colorWidget->getColor();

        // update feature's color
        feature->setColor(color.red(), color.green(), color.blue(), true);
    }

    emit sigUpdateOpengl();
}

// 3. rule-based
void LayerStyleDialog::processRuleBasedStyle()
{
    int rowCount = loadSldResultWidget->rowCount();

    for (int iRow = 0; iRow < rowCount; ++iRow) {
        GeoFeature* feature = layer->getFeature(iRow);

        const QWidget* widget = loadSldResultWidget->cellWidget(iRow, 0);
        const auto& children  = widget->children();

        // 3 children: layout + checkbox + widget(color block)
        //QHBoxLayout* hLayout = (QHBoxLayout*)children.at(0);
        //QCheckBox* checkBox = (QCheckBox*)children.at(1);
        ColorBlockWidget* colorWidget = (ColorBlockWidget*)children.at(2);

        QColor color = colorWidget->getColor();
        feature->setColor(color.red(), color.green(), color.blue(), true);
    }

    emit sigUpdateOpengl();
}

// Read .SLD file
void LayerStyleDialog::onBtnLoadSldClicked()
{
    QString filepath = QFileDialog::getOpenFileName(this, tr("Select SLD file"), ".", tr("SLD file(*.sld)"),
                                                    nullptr, QFileDialog::DontUseNativeDialog);

    if (filepath.isEmpty())
        return;

    SLDInfo* sldInfo = FileReader::readSLD(filepath, layer);
    if (!sldInfo) {
        LError("Read SLD file error");
        QMessageBox::critical(this, tr("Error"), tr("Read SLD file error"));
        return;
    }

    int rulesCount = sldInfo->rules.size();
    modeComboBox->setCurrentIndex(2);
    loadSldResultWidget->clearContents();
    loadSldResultWidget->setRowCount(rulesCount);
    QString fieldName = sldInfo->fieldName;
    filedNameLabel->setText(fieldName);
    int fieldIndex = layer->getFieldIndex(fieldName);
    int featuresCount = layer->getFeatureCount();

    for (int iFeature = 0; iFeature < featuresCount; ++iFeature) {
        GeoFeature* feature = layer->getFeature(iFeature);
        int nFID = feature->getFID();
        switch (sldInfo->fieldType) {
        default:
            break;
        case kFieldInt:
        {
            int value;
            feature->getField(fieldIndex, &value);
            auto rule = sldInfo->getRule(value);
            if (rule) {
                insertClassifyItem(loadSldResultWidget, iFeature, nFID, rule->fillColor,
                                   *(int*)(rule->fieldValue));
            }
            break;
        }
        case kFieldDouble:
        {
            double value;
            feature->getField(fieldIndex, &value);
            auto rule = sldInfo->getRule(value);
            if (rule) {
                insertClassifyItem(loadSldResultWidget, iFeature, nFID, rule->fillColor,
                                   *(double*)(rule->fieldValue));
            }
            break;
        }
        case kFieldText:
        {
            QString value;
            feature->getField(fieldIndex, &value);
            auto rule = sldInfo->getRule(value);
            if (rule) {
                insertClassifyItem(loadSldResultWidget, iFeature, nFID, rule->fillColor,
                                   *(QString*)(rule->fieldValue));
            }
            break;
        }
        }
    }

    delete sldInfo;
}
