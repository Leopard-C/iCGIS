/*******************************************************
** class name:  GlobalSearchWidget
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QLineEdit>
#include <QCompleter>
#include <QStringList>
#include <QPushButton>

#include "dialog/globalsearchresult.h"

class GeoMap;
class GeoFeatureLayer;


class GlobalSearchWidget : public QLineEdit
{
    Q_OBJECT

public:
    GlobalSearchWidget(QWidget *parent);
    ~GlobalSearchWidget();

public:
    void updateCompleterList();

public slots:
    void onClearText();
    void onSearch();
    void onTextChanged();
    void onSearchResultDialogClose();

    /* Override Event */
protected:
    virtual void focusInEvent(QFocusEvent* event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;

private:
    void setupLayout();
    bool searchInLayer(const QString& value, GeoFeatureLayer* layerIn, std::vector<GeoFeature*>& featuresOut);

    GeoMap*& map;

    QStringList stringList;
    QCompleter* completer;
    QPushButton* btnSearch;
    QPushButton* btnClearText;

    GlobalSearchResult* searchResultDialog = nullptr;
};
