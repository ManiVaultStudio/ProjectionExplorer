#pragma once

#include "PointData.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include <Eigen/Eigen>

using namespace hdps;

class Explanation
{
public:
    enum class Metric
    {
        EUCLIDEAN,
        VARIANCE
    };

    void setDataset(Dataset<Points> dataset, Dataset<Points> projection);

    void computeDimensionRanking(Eigen::ArrayXXi& dimRanking, std::vector<unsigned int> selection);
    void computeDimensionRanking(Eigen::ArrayXXi& dimRanking);
    void computeDimensionRanks(Eigen::ArrayXXf& dimRanks, std::vector<unsigned int>& selection, Metric metric = Metric::VARIANCE);
    void computeDimensionRanks(Eigen::ArrayXXf& dimRanks, Metric metric = Metric::VARIANCE);

private:
    // Precomputation
    void computeNeighbourhoodMatrix();
    void computeCentroid();
    void computeGlobalContribs();
    void computeLocalContribs();
    void computeGlobalVariances();
    void computeLocalVariances();

private:
    Eigen::ArrayXXf _dataset;
    Eigen::ArrayXXf _projection;

    // Precomputed values
    std::vector<std::vector<int>> _neighbourhoodMatrix;
    std::vector<float> _centroid;
    std::vector<float> _globalDistContribs;
    Eigen::ArrayXXf _localDistContribs;
    std::vector<float> _globalVariance;
    Eigen::ArrayXXf _localVariances;
};
