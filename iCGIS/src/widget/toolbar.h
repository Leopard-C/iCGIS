#ifndef TOOLBAR_H
#define TOOLBAR_H

#include <QToolBar>
#include <QAction>
#include <QLineEdit>

class GeoMap;

class ToolBar : public QToolBar
{
    Q_OBJECT
public:
    ToolBar(QWidget* parent = nullptr);

signals:
    void sigUpdateOpengl();

public slots:
    void onStartEditing(bool on);

public slots:
    void onRotate();
    void onCursorNormal();
    void onCursorEditing();
    void onCursorPalm();
    void onCursorWhatIsThis();

private:
    void createWidgets();
    void createActions();
    void setupLayout();

private:
    GeoMap*& map;

    /* Widget */
    QLineEdit* editRotate;

    /* Action */
    QAction* cursorNormalAction;
    QAction* cursorEditingAction;
    QAction* cursorPalmAction;
    QAction* cursorWhatIsThisAction;
    QAction* rotateAction;
};

#endif // TOOLBAR_H
