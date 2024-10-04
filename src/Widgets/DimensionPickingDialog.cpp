#include "DimensionPickingDialog.h"

#include "ProjectionExplorerPlugin.h"

DimensionPickingDialog::DimensionPickingDialog(QWidget* parent) :
    QDialog(parent),
    _groupAction(this, "Group action"),
    _firstDimensionAction(this, "First dimension"),
    _secondDimensionAction(this, "Second dimension"),
    _okAction(this, "Load Projection")
{
    setWindowTitle("Projection dimensions");

    QStringList list = { "Dim 1", "Dim 2", "Dim 3" };

    _firstDimensionAction.setOptions(list);
    _secondDimensionAction.setOptions(list);

    _firstDimensionAction.setCurrentIndex(0);
    _secondDimensionAction.setCurrentIndex(1);

    _groupAction.addAction(&_firstDimensionAction);
    _groupAction.addAction(&_secondDimensionAction);
    _groupAction.addAction(&_okAction);

    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_groupAction.createWidget(this));

    setLayout(layout);

    connect(&_okAction, &TriggerAction::triggered, this, [this]() {
        accept();
    });
}
