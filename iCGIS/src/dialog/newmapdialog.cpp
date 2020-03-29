#include "newmapdialog.h"

#include <QLabel>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QMessageBox>

#include "util/env.h"
#include "util/appevent.h"
#include "geo/map/geomap.h"


NewMapDialog::NewMapDialog(QWidget* parent)
    : QDialog(parent)
{
    setupLayout();
    connect(this, &NewMapDialog::sigNewMap,
            AppEvent::getInstance(), &AppEvent::onNewMap);
}

void NewMapDialog::setupLayout() {
    editMapName = new QLineEdit(this);
    editPath = new QLineEdit(this);
    QPushButton* btnChooseFolder = new QPushButton(this);
    btnChooseFolder->setIcon(QIcon("res/icons/open.ico"));
    connect(btnChooseFolder, &QPushButton::clicked,
            this, &NewMapDialog::onChooseFolder);
    QHBoxLayout* pathLayout = new QHBoxLayout();
    pathLayout->addWidget(editPath);
    pathLayout->addWidget(btnChooseFolder);

    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(new QLabel("Map Name:", this), editMapName);
    formLayout->addRow(new QLabel("Folder:", this), pathLayout);

    QPushButton* btnOk = new QPushButton("OK", this);
    QPushButton* btnCancel = new QPushButton("Cancel", this);
    connect(btnOk, &QPushButton::clicked, this, &NewMapDialog::onBtnOk);
    connect(btnCancel, &QPushButton::clicked, this, &NewMapDialog::close);
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    btnLayout->addWidget(btnOk);
    btnLayout->addWidget(btnCancel);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(btnLayout);
}


/*   SLOTs    */

void NewMapDialog::onChooseFolder() {
    QString path = QFileDialog::getExistingDirectory(this, "Choose folder", "");
    if (!path.isEmpty()) {
        editPath->setText(path);
    }
}

void NewMapDialog::onBtnOk() {
    if (editMapName->text().isNull()) {
        QMessageBox::critical(this, "Error", "Please input map's name", QMessageBox::Close);
        return;
    }
    if (editPath->text().isNull()) {
        QMessageBox::critical(this, "Error", "Please select project's path", QMessageBox::Close);
        return;
    }
    this->hide();
    emit sigNewMap(editMapName->text(), editPath->text());
    this->close();
}
