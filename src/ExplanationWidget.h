#pragma once

#include <PointData.h>

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QImage>
#include <QSlider>

#include <Eigen/Eigen>

#include <vector>

class DataMetrics
{
public:
    void compute(const hdps::Dataset<Points>& dataset, const std::vector<unsigned int>& selection, const std::vector<float>& minRanges, const std::vector<float>& maxRanges);

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

    BarChart();

    void setDataset(hdps::Dataset<Points> dataset);
    void setRanking(Eigen::ArrayXXf& ranking, const std::vector<unsigned int>& selection);
    void computeOldMetrics(const std::vector<unsigned int>& oldSelection);
    void setImportantDims(const std::vector<float>& importantDims);

    void showDifferentialValues(bool on) { _differentialRanking = on; }

public slots:
    void sortByDefault();
    void sortByVariance();
    void sortByValue();

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //QPainter painter;
    hdps::Dataset<Points> _dataset;
    std::vector<float> _dimAggregation;
    float _maxValue;
    std::vector<float> _importantDims;
    std::vector<int> _sortIndices;

    DataMetrics _newMetrics;
    DataMetrics _oldMetrics;

    std::vector<float> _minRanges;
    std::vector<float> _maxRanges;
    std::vector<unsigned int> _selection;

    bool _differentialRanking = false;

    SortingType _sortingType = SortingType::NONE;

    std::vector<QColor> _colors;
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
    ExplanationWidget();
    ~ExplanationWidget();

    void update();
    void setRanking(Eigen::ArrayXXf& ranking);

    BarChart& getBarchart() { return *_barChart; }
    ImageViewWidget& getImageWidget() { return *_imageViewWidget; }
    QSlider* getRadiusSlider() { return _radiusSlider; }
    QPushButton* getVarianceColoringButton() { return _varianceColoringButton; }
    QPushButton* getValueColoringButton() { return _valueColoringButton; }

private:
    QLabel* _rankLabel;
    BarChart* _barChart;
    ImageViewWidget* _imageViewWidget;
    QSlider* _radiusSlider;

    QPushButton* _varianceColoringButton;
    QPushButton* _valueColoringButton;
};
