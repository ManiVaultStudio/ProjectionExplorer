#pragma once

#include <PointData.h>

#include <QWidget>
#include <QLabel>
#include <QPainter>
#include <QColor>
#include <QImage>

#include <Eigen/Eigen>

#include <vector>

class BarChart : public QWidget
{
    Q_OBJECT
public:
    enum class SortingType
    {
        VARIANCE,
        VALUE
    };

    BarChart();

    void setDataset(hdps::Dataset<Points> dataset);
    void setRanking(Eigen::ArrayXXf& ranking, const std::vector<unsigned int>& selection);
    void setImportantDims(const std::vector<float>& importantDims);

public slots:
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
    std::vector<float> _averageValues;
    std::vector<float> _minRanges;
    std::vector<float> _maxRanges;

    SortingType _sortingType = SortingType::VARIANCE;

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

private:
    QLabel* _rankLabel;
    BarChart* _barChart;
    ImageViewWidget* _imageViewWidget;
};
