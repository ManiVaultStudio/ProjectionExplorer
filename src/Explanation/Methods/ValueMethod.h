#pragma once

#include "ExplanationMethod.h"
#include "LocalValueComputation.h"

#include <vector>

class ValueMethod : public Explanation::Method
{
public:
    ValueMethod();

    void setGlobalNeighbourhoodRadius(float radius);

    void recompute(DataMatrix& dataset) override;
    //void recompute(DataMatrix& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);
    void recompute(DataMatrix& dataset, DataMatrix& projection);

    float computeDimensionRank(DataMatrix& dataset, int i, int j);
    void computeDimensionRank(DataMatrix& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking);

private:
    void precomputeGlobalValues(DataMatrix& dataset);
    //void precomputeLocalValues(DataMatrix& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);
    void precomputeLocalValues(DataMatrix& dataset, DataMatrix& projection);

private:
    std::vector<float>  _globalValues;
    std::vector<std::vector<float>>  _localValues;

    LocalValueComputation _lvc;

    std::vector<float> _dataRanges;

    std::vector<float> _sums;

    float               _globalNeighbourhoodRadius = 0.02;
};
