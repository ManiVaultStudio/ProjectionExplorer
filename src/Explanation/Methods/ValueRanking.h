#pragma once

#include "ExplanationMethod.h"

class ValueMethod : public Explanation::Method
{
public:
    void recompute(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) override;
    float computeDimensionRank(const DataTable& dataset, int i, int j) override;
    void computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) override;

private:
    void precomputeGlobalValues(const DataTable& dataset);
    void precomputeLocalValues(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix);

    std::vector<float> _globalValues;

    DataMatrix _localValues;

    std::vector<float> _dataRanges;
};
