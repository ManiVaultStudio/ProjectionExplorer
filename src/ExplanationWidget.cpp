#include "ExplanationWidget.h"

#include <QVBoxLayout>

#include <string>
#include <iostream>

BarChart::BarChart()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);
}

void BarChart::setRanking(Eigen::ArrayXXf& ranking)
{
    if (ranking.rows() == 0)
        return;

    std::vector<float> dimAggregation(ranking.cols(), 0);
    for (int i = 0; i < ranking.rows(); i++)
    {
        for (int j = 0; j < ranking.cols(); j++)
        {
            dimAggregation[j] += ranking(i, j);
        }
    }
    for (int i = 0; i < dimAggregation.size(); i++)
    {
        dimAggregation[i] /= (float) ranking.rows();
        std::cout << "Agg: " << dimAggregation[i] << std::endl;
    }

    _dimAggregation = dimAggregation;
}

void BarChart::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);

    painter.drawRect(0, 0, 200, 250);

    for (int i = 0; i < _dimAggregation.size(); i++)
    {
        painter.drawText(10, 16 * i, QString::number(i));
        painter.fillRect(30, 16 * i, _dimAggregation[i] * 600, 14, QColor(255, 0, 0));
    }
}

ExplanationWidget::ExplanationWidget()
{
    _rankLabel = new QLabel("No ranking");
    _barChart = new BarChart();

    QVBoxLayout* layout = new QVBoxLayout();
    layout->addWidget(_barChart);
    layout->addWidget(_rankLabel);

    setLayout(layout);

    setMaximumWidth(300);
}

ExplanationWidget::~ExplanationWidget()
{

}

void ExplanationWidget::update()
{
    _barChart->repaint();
}

void ExplanationWidget::setRanking(Eigen::ArrayXXf& ranking)
{
    if (ranking.rows() == 0)
        return;

    std::string rankText;

    std::vector<float> dimAggregation(ranking.cols(), 0);
    for (int i = 0; i < ranking.rows(); i++)
    {
        for (int j = 0; j < ranking.cols(); j++)
        {
            dimAggregation[j] += ranking(i, j);
        }
    }
    for (int i = 0; i < dimAggregation.size(); i++)
    {
        dimAggregation[i] /= (float)ranking.rows();
    }

    for (int i = 0; i < dimAggregation.size(); i++)
        rankText += std::to_string(dimAggregation[i]) + " ";

    _rankLabel->setText(rankText.c_str());

    _barChart->setRanking(ranking);
}
