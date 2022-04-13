#pragma once

#include "ExplanationMethod.h"

class VarianceMethod : public Explanation::Method
{
public:
    void recompute(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) override;
    float computeDimensionRank(const DataTable& dataset, int i, int j) override;
    void computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) override;

private:
    void precomputeGlobalVariances(const DataTable& dataset);
    void precomputeLocalVariances(DataMatrix& localVariances, const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix);

    std::vector<float> _globalVariances;

    Eigen::ArrayXXf _localVariances;
};
