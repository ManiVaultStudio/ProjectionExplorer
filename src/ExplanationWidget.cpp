#include "ExplanationWidget.h"

#include <QVBoxLayout>

#include <string>

BarChart::BarChart()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setMinimumHeight(270);
}

void BarChart::setRanking(Eigen::ArrayXXi& ranking)
{
    std::string rankText;

    if (ranking.rows() == 0)
        return;

    std::vector<float> dimAggregation(ranking.cols());
    for (int i = 0; i < ranking.rows(); i++)
    {
        for (int j = 0; j < ranking.cols(); j++)
        {
            dimAggregation[ranking(i, j)] += ranking.cols() - j;
        }
    }
    for (int i = 0; i < dimAggregation.size(); i++)
    {
        dimAggregation[i] /= (float) ranking.rows();
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
        painter.fillRect(30, 16 * i, _dimAggregation[i] * 10, 14, QColor(255, 0, 0));
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

void ExplanationWidget::setRanking(Eigen::ArrayXXi& ranking)
{
    std::string rankText;

    if (ranking.rows() == 0)
        return;

    std::vector<int> dimAggregation(ranking.cols());
    for (int i = 0; i < ranking.rows(); i++)
    {
        for (int j = 0; j < ranking.cols(); j++)
        {
            dimAggregation[ranking(i, j)] += ranking.cols() - j;
        }
    }

    for (int i = 0; i < dimAggregation.size(); i++)
        rankText += std::to_string(dimAggregation[i]) + " ";

    _rankLabel->setText(rankText.c_str());

    _barChart->setRanking(ranking);
}
