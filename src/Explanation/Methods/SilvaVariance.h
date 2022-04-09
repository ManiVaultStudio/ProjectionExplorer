#pragma once

#include "ExplanationMethod.h"

class VarianceMethod : public Explanation::Method
{
public:
    void recompute(const DataMatrix& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) override;
    float computeDimensionRank(const DataMatrix& dataset, int i, int j) override;
    void computeDimensionRank(const DataMatrix& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) override;

private:
    void precomputeGlobalVariances(const DataMatrix& dataset);
    void precomputeLocalVariances(DataMatrix& localVariances, const DataMatrix& dataset, NeighbourhoodMatrix& neighbourhoodMatrix);

    std::vector<float> _globalVariances;

    Eigen::ArrayXXf _localVariances;
};
