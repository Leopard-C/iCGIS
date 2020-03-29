#include "widget/toolbar.h"
#include "util/env.h"
#include "util/appevent.h"
#include "geo/map/geomap.h"

#include <QRegExpValidator>


ToolBar::ToolBar(QWidget* parent)
    : QToolBar(parent), map(Env::map)
{
    this->setFixedHeight(35);

    createWidgets();
    createActions();
    setupLayout();

    connect(AppEvent::getInstance(), &AppEvent::sigStartEditing,
            this, &ToolBar::onStartEditing);
    connect(this, &ToolBar::sigUpdateOpengl,
            AppEvent::getInstance(), &AppEvent::onUpdateOpengl);
}

void ToolBar::createWidgets() {
    editRotate = new QLineEdit(this);
    editRotate->setFixedWidth(60);
    editRotate->setAlignment(Qt::AlignRight);
    // [-360.00, 360.00]
    QRegExp re("^-?(360|[1-2]?\\d?\\d|[1-3]?[0-5]?\\d(\\.\\d{1,2})?)$");
    editRotate->setValidator(new QRegExpValidator(re, editRotate));
    editRotate->setDisabled(true);
    connect(editRotate, &QLineEdit::returnPressed, this, &ToolBar::onRotate);
}

void ToolBar::createActions() {
    // cursor type
    // palm
    cursorPalmAction = new QAction(this);
    cursorPalmAction->setIcon(QIcon("res/icons/palm.ico"));
    cursorPalmAction->setCheckable(true);
    connect(cursorPalmAction, &QAction::triggered, this, &ToolBar::onCursorPalm);
    // normal
    cursorNormalAction = new QAction(this);
    cursorNormalAction->setCheckable(true);
    cursorNormalAction->setChecked(true);
    cursorNormalAction->setIcon(QIcon("res/icons/normal-arrow.ico"));
    connect(cursorNormalAction, &QAction::triggered, this, &ToolBar::onCursorNormal);
    // editing
    cursorEditingAction = new QAction(this);
    cursorEditingAction->setCheckable(true);
    cursorEditingAction->setChecked(false);
    cursorEditingAction->setIcon(QIcon("res/icons/editing-arrow.ico"));
    connect(cursorEditingAction, &QAction::triggered, this, &ToolBar::onCursorEditing);
    // what is this
    cursorWhatIsThisAction = new QAction(this);
    cursorWhatIsThisAction->setIcon(QIcon("res/icons/what-is-this.ico"));
    cursorWhatIsThisAction->setCheckable(true);
    connect(cursorWhatIsThisAction, &QAction::triggered, this, &ToolBar::onCursorWhatIsThis);

    // rotate
    rotateAction = new QAction(this);
    rotateAction->setIcon(QIcon("res/icons/rotate.ico"));
    rotateAction->setDisabled(true);
    connect(rotateAction, &QAction::triggered, this, &ToolBar::onRotate);
}

void ToolBar::setupLayout() {
    addSeparator();
    addAction(cursorPalmAction);
    addAction(cursorNormalAction);
    addAction(cursorEditingAction);
    addAction(cursorWhatIsThisAction);
    addSeparator();

    addWidget(editRotate);
    addAction(rotateAction);
}

/***************************************************
 *
 *      Change Mouse Cursor Type
 *          1. normal
 *          2. palm shape
 *          3. editing
 *          4. What is this
 *
****************************************************/

void ToolBar::onStartEditing(bool on) {
    if (on) {
        Env::cursorType = Env::CursorType::Editing;
        // Editing
        cursorEditingAction->setEnabled(true);
        cursorEditingAction->setChecked(true);
        // Normal
        cursorNormalAction->setEnabled(false);
        cursorNormalAction->setChecked(false);
        // Palm
        cursorPalmAction->setChecked(false);
        // What Is This
        cursorWhatIsThisAction->setChecked(false);
        // rotate
        editRotate->setEnabled(true);
        rotateAction->setEnabled(true);
    }
    else {
        cursorNormalAction->setEnabled(true);
        cursorEditingAction->setEnabled(false);
        editRotate->setEnabled(false);
        rotateAction->setEnabled(false);
        if (cursorEditingAction->isChecked()) {
            Env::cursorType = Env::CursorType::Normal;
            cursorEditingAction->setChecked(false);
            cursorNormalAction->setChecked(true);
        }
    }
}

void ToolBar::onCursorNormal() {
    if (!cursorNormalAction->isChecked()) {
        if (Env::cursorType == Env::CursorType::Normal) {
            cursorNormalAction->setChecked(true);
        }
    }
    else {
        Env::cursorType = Env::CursorType::Normal;
        cursorPalmAction->setChecked(false);
        cursorEditingAction->setChecked(false);
        cursorWhatIsThisAction->setChecked(false);
    }
}

void ToolBar::onCursorEditing() {
    if (!cursorEditingAction->isChecked()) {
        if (Env::cursorType == Env::CursorType::Editing) {
            cursorEditingAction->setChecked(true);
        }
    }
    else {
        Env::cursorType = Env::CursorType::Editing;
        cursorNormalAction->setChecked(false);
        cursorPalmAction->setChecked(false);
        cursorWhatIsThisAction->setChecked(false);
    }
}

void ToolBar::onCursorPalm() {
    if (!cursorPalmAction->isChecked()) {
        if (Env::cursorType == Env::CursorType::Palm) {
            cursorWhatIsThisAction->setChecked(true);
        }
    }
    else {
        Env::cursorType = Env::CursorType::Palm;
        cursorNormalAction->setChecked(false);
        cursorEditingAction->setChecked(false);
        cursorWhatIsThisAction->setChecked(false);
    }
}

void ToolBar::onCursorWhatIsThis() {
    if (!cursorWhatIsThisAction->isChecked()) {
        if (Env::cursorType == Env::CursorType::WhatIsThis) {
            cursorWhatIsThisAction->setChecked(true);
        }
    }
    else {
        Env::cursorType = Env::CursorType::WhatIsThis;
        cursorNormalAction->setChecked(false);
        cursorPalmAction->setChecked(false);
        cursorEditingAction->setChecked(false);
    }
}

void ToolBar::onRotate() {
    if (editRotate->text().isEmpty())
        return;
    double angle = editRotate->text().toDouble();
    std::map<GeoFeatureLayer*, std::vector<GeoFeature*>> selectedFeatures;
    map->getAllSelectedFeatures(selectedFeatures);
    Env::opList.addRotateOperation(selectedFeatures, angle)->operate();
    emit sigUpdateOpengl();
}
