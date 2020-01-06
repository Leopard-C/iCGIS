/*******************************************************
** class name:  GlobalSearchWidget
**
** description: 搜索框，支持全局搜索
**
** usage：
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QLineEdit>
#include <QCompleter>
#include <QStringList>
#include <QPushButton>

#include "geo/map/geomap.h"
#include "dialog/globalsearchresult.h"

class OpenGLWidget;


class GlobalSearchWidget : public QLineEdit
{
	Q_OBJECT

public:
	GlobalSearchWidget(GeoMap* mapIn, QWidget *parent);
	~GlobalSearchWidget();

public:
	void updateCompleterList();
	void setOpenGLWidget(OpenGLWidget* openglWidgetIn)
		{ openglWidget = openglWidgetIn; }
	
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

	GeoMap* map;
	OpenGLWidget* openglWidget;

	QStringList stringList;
	QCompleter* completer;
	QPushButton* btnSearch;
	QPushButton* btnClearText;

	GlobalSearchResult* searchResultDialog = nullptr;
};
