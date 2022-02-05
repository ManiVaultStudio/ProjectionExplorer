#pragma once

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

    void setRanking(Eigen::ArrayXXf& ranking);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //QPainter painter;
    std::vector<float> _dimAggregation;

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

    ImageViewWidget& getImageWidget() { return *_imageViewWidget; }

private:
    QLabel* _rankLabel;
    BarChart* _barChart;
    ImageViewWidget* _imageViewWidget;
};
