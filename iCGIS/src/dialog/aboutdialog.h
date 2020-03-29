/*******************************************************
** Dialog: AboutDialog
**
** description: About the program and the author
**
** last change: 2020-03-26
*******************************************************/
#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

class AboutDialog : public QDialog
{
    Q_OBJECT
public:
    AboutDialog(QWidget* parent = nullptr);

public:
    void setupLayout();
};

#endif // ABOUTDIALOG_H
