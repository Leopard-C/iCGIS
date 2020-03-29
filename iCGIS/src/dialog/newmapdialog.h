/****************************************************************
** class name:  NewMapDialog
**
** description: Dialog: Create new map
**
** last change: 2020-03-29
***************************************************************/
#ifndef NEWMAPDIALOG_H
#define NEWMAPDIALOG_H

#include <QDialog>
#include <QLineEdit>


class NewMapDialog : public QDialog
{
    Q_OBJECT
public:
    NewMapDialog(QWidget* parent = nullptr);

signals:
    void sigNewMap(const QString& name, const QString& path);

public:
    void setupLayout();

public slots:
    void onChooseFolder();
    void onBtnOk();

private:
    QLineEdit* editMapName;
    QLineEdit* editPath;
};

#endif // NEWMAPDIALOG_H
