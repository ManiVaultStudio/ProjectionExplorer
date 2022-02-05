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
    BarChart();

    void setDataset(hdps::Dataset<Points> dataset);
    void setRanking(Eigen::ArrayXXf& ranking);
    void setImportantDims(const std::vector<float>& importantDims);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //QPainter painter;
    hdps::Dataset<Points> _dataset;
    std::vector<float> _dimAggregation;
    std::vector<float> _importantDims;
    std::vector<int> _sortIndices;

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
