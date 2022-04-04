#pragma once

#include <PointData.h>

#include "Explanation/ExplanationModel.h"

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QImage>
#include <QSlider>
#include <QComboBox>

#include <Eigen/Eigen>

#include <vector>

class DataMetrics
{
public:
    void compute(const DataMatrix& dataset, const std::vector<unsigned int>& selection, const DataMatrix& dataRanges);

    std::vector<float> averageValues;
    std::vector<float> variances;
};

class BarChart : public QWidget
{
    Q_OBJECT
public:
    enum class SortingType
    {
        NONE,
        VARIANCE,
        VALUE
    };

    enum class SortingDirection
    {
        LOW_TO_HIGH,
        HIGH_TO_LOW,
    };

    BarChart(ExplanationModel& explanationModel);

    void setRanking(DataMatrix& ranking, const std::vector<unsigned int>& selection);
    void computeOldMetrics(const std::vector<unsigned int>& oldSelection);

    void showDifferentialValues(bool on) { _differentialRanking = on; }

public slots:
    void sortByDefault();
    void sortByVariance();
    void sortByValue();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    ExplanationModel& _explanationModel;

    std::vector<float> _dimAggregation;
    std::vector<int> _sortIndices;

    DataMetrics _newMetrics;
    DataMetrics _oldMetrics;

    std::vector<unsigned int> _selection;

    bool _differentialRanking;

    SortingType _sortingType;
    SortingDirection _sortingDirection;
};

class ImageViewWidget : public QWidget
{
    Q_OBJECT
public:
    ImageViewWidget();
    void setImage(QImage image);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QImage _image;
};

class ExplanationWidget : public QWidget
{
    Q_OBJECT
public:
    ExplanationWidget(ExplanationModel& explanationModel);
    ~ExplanationWidget();

    void update();

    BarChart& getBarchart() { return *_barChart; }
    ImageViewWidget& getImageWidget() { return *_imageViewWidget; }
    QSlider* getRadiusSlider() { return _radiusSlider; }
    QComboBox* getRankingComboBox() { return _rankingCombobox; }

public slots:
    void neighbourhoodRadiusValueChanged(int value);

private:
    QLabel* _rankLabel;
    BarChart* _barChart;
    ImageViewWidget* _imageViewWidget;

    // UI Elements
    QLabel* _radiusSliderValueLabel;
    QSlider* _radiusSlider;
    QComboBox* _rankingCombobox;
};
