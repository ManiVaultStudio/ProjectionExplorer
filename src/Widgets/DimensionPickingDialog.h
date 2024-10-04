#pragma once

#include <actions/GroupAction.h>
#include <actions/OptionAction.h>
#include <actions/TriggerAction.h>

#include <QWidget>
#include <QDialog>

class ProjectionExplorerPlugin;

class DimensionPickingDialog : public QDialog
{
    Q_OBJECT
public:
    DimensionPickingDialog(QWidget* parent);

    int getFirstDimension() { qDebug() << _firstDimensionAction.getCurrentIndex(); return _firstDimensionAction.getCurrentIndex(); }
    int getSecondDimension() { qDebug() << _secondDimensionAction.getCurrentIndex(); return _secondDimensionAction.getCurrentIndex(); }

private:
    mv::gui::GroupAction        _groupAction;

    mv::gui::OptionAction       _firstDimensionAction;
    mv::gui::OptionAction       _secondDimensionAction;

    mv::gui::TriggerAction      _okAction;
};
