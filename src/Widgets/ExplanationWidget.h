#pragma once

#include "Explanation/ExplanationModel.h"

#include <QWidget>

class HistogramChart : public QWidget
{
    Q_OBJECT
public:
    HistogramChart(QWidget* parent, Explanation::Model& explanationModel);

private:
    Explanation::Model& _explanationModel;
};

class ExplanationWidget : public QWidget
{
    Q_OBJECT
public:
    ExplanationWidget(Explanation::Model& explanationModel);

private:
    Explanation::Model& _explanationModel;

    HistogramChart* _histogramChart;
};
