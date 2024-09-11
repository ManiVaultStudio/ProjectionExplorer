#include "ExplanationWidget.h"

#include <QVBoxLayout>

HistogramChart::HistogramChart(QWidget* parent, Explanation::Model& explanationModel) :
    QWidget(parent),
    _explanationModel(explanationModel)
{

}

ExplanationWidget::ExplanationWidget(Explanation::Model& explanationModel) :
    _explanationModel(explanationModel),
    _histogramChart(new HistogramChart(this, explanationModel))
{
    QVBoxLayout* layout = new QVBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(_histogramChart);

    setLayout(layout);
}
