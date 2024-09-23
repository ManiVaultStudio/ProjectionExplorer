#pragma once

#include "Explanation/ExplanationModel.h"
#include "Explanation/Histogram.h"

#include <QWidget>

class HistogramChart : public QWidget
{
    Q_OBJECT
public:
    HistogramChart(QWidget* parent, Explanation::Model& explanationModel);

    void computeGlobalHistograms();

    void setRanking(const std::vector<unsigned int>& selection);

    void paintEvent(QPaintEvent* event) override;

private:
    Explanation::Model& _explanationModel;

    std::vector<int>        _sortIndices;

    std::vector<Histogram>  _localHistograms;
    std::vector<Histogram>  _globalHistograms;
};

class ExplanationWidget : public QWidget
{
    Q_OBJECT
public:
    ExplanationWidget(Explanation::Model& explanationModel);

    HistogramChart& getHistogramChart() { return *_histogramChart; }

    void updateWidgets();

private:
    Explanation::Model& _explanationModel;

    HistogramChart* _histogramChart;
};
