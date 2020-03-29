/*******************************************************
** class name:  GlobalSearchResult
**
** description: Shwo the result of global search
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

class GeoMap;
class GeoFeature;
class OpenGLWidget;


class GlobalSearchResult : public QDialog
{
    Q_OBJECT

public:
    GlobalSearchResult(QWidget *parent = nullptr);
    ~GlobalSearchResult();

signals:
    void sigClosed();
    void sigUpdateOpengl();

public slots:
    void onDoubleClicked(QTableWidgetItem* item);

public:
    void clear();
    void setupLayout();
    void addSearchResult(int nLID, const std::vector<GeoFeature*>& features)
        { results.emplace_back(nLID, features); }

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    void clearLayout(QLayout* layout);

    struct CustomData : QObjectUserData {
        CustomData(int nLID) :LID(nLID) {}
        int LID;
    };

    struct SearchResult {
        SearchResult(int nLID, const std::vector<GeoFeature*>& features)
            : LID(nLID), features(features) {}
        int LID;
        std::vector<GeoFeature*> features;
    };

private:
    std::vector<SearchResult> results;

    GeoMap*& map;

    QVBoxLayout* mainLayout = nullptr;
    QVBoxLayout* scrollAreaLayout = nullptr;
    QScrollArea* scrollArea = nullptr;
    QWidget* scrollAreaCenterWidget = nullptr;
};
