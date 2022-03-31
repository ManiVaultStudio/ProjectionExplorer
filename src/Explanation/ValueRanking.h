#pragma once

#include <Eigen/Eigen>

#include <vector>

class ValueRanking
{
public:
    void recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);
    float computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j);

private:
    void precomputeGlobalValues(const Eigen::ArrayXXf& dataset);
    void precomputeLocalValues(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);

    std::vector<float> _globalValues;

    Eigen::ArrayXXf _localValues;
};
