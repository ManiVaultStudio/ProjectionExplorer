#pragma once

#include <Eigen/Eigen>

#include <vector>

class VarianceMetric
{
public:
    void recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix, std::vector<std::vector<int>>& confidenceNeighbourhoodMatrix);
    float computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j);
    float computeConfidence(const Eigen::ArrayXXf& dataset, int i, int j);

private:
    void precomputeGlobalVariances(const Eigen::ArrayXXf& dataset);
    void precomputeLocalVariances(Eigen::ArrayXXf& localVariances, const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);

    std::vector<float> _globalVariances;

    Eigen::ArrayXXf _localVariances;
};
