/*******************************************************
** class name:  GlobalSearchResult
**
** description: 全局搜索的搜索结果显示
**
** last change: 2020-01-02
*******************************************************/
#pragma once

#include <QDialog>
#include <QScrollArea>
#include <QTableWidget>
#include <QVBoxLayout>

#include <map>
#include <vector>

#include "geo/map/geomap.h"

class OpenGLWidget;


class GlobalSearchResult : public QDialog
{
	Q_OBJECT

public:
	GlobalSearchResult(GeoMap* mapIn, OpenGLWidget* openglWidgetIn, QWidget *parent);
	~GlobalSearchResult();

signals:
	void closed();

public slots:
	// 双击数据区
	void onDoubleClicked(QTableWidgetItem* item);

public:
	void clear();
	void setupLayout();
	void addSearchResult(int nLID, const std::vector<GeoFeature*>& features)
		{ results.emplace_back(nLID, features); }

protected:
	virtual void closeEvent(QCloseEvent* event) override;

private:
	struct SearchResult {
		SearchResult(int nLID, const std::vector<GeoFeature*>& features)
			: LID(nLID), features(features) {}
		int LID;
		std::vector<GeoFeature*> features;
	};

	struct CustomData : QObjectUserData {
		CustomData(int nLID) :LID(nLID) {}
		int LID;
	};

private:
	void clearLayout(QLayout* layout);

	std::vector<SearchResult> results;

	GeoMap* map;
	OpenGLWidget* openglWidget;

	QVBoxLayout* mainLayout = nullptr;
	QVBoxLayout* scrollAreaLayout = nullptr;
	QScrollArea* scrollArea = nullptr;
	QWidget* scrollAreaCenterWidget = nullptr;
};
