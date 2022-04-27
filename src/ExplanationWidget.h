#pragma once

#include <QWidget>
#include <QLabel>
#include <QPainter>

#include <Eigen/Eigen>

#include <vector>

class BarChart : public QWidget
{
    Q_OBJECT
public:
    BarChart();

    void setRanking(Eigen::ArrayXXi& ranking);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    //QPainter painter;
    std::vector<float> _dimAggregation;
};

class ExplanationWidget : public QWidget
{
    Q_OBJECT
public:
    ExplanationWidget();
    ~ExplanationWidget();

    void setRanking(Eigen::ArrayXXi& ranking);

private:
    QLabel* _rankLabel;
    BarChart* _barChart;
};
