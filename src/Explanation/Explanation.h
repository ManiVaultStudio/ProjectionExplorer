#pragma once

#include "PointData.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include "SilvaVariance.h"
#include "SilvaEuclidean.h"
#include <Eigen/Eigen>

#include <QImage>

using namespace hdps;

void computeEigenImage();

class Explanation
{
public:
    enum class Metric
    {
        EUCLIDEAN,
        VARIANCE
    };

    void setDataset(Dataset<Points> dataset, Dataset<Points> projection);
    void updatePrecomputations(float neighbourhoodRadius);

    void computeDimensionRanks(Eigen::ArrayXXf& dimRanks, std::vector<unsigned int>& selection, Metric metric = Metric::VARIANCE);
    void computeDimensionRanks(Eigen::ArrayXXf& dimRanks, Metric metric = Metric::VARIANCE);

    QImage computeEigenImage(std::vector<unsigned int>& selection, std::vector<float>& importantDims);

private:
    // Precomputation
    void computeNeighbourhoodMatrix(float radius);

private:
    Eigen::ArrayXXf _dataset;
    Eigen::ArrayXXf _projection;

    Eigen::ArrayXXf _normDataset;
    float _projectionDiameter;

    // Precomputed values
    std::vector<std::vector<int>> _neighbourhoodMatrix;

    EuclideanMetric _euclideanMetric;
    VarianceMetric _varianceMetric;
};
