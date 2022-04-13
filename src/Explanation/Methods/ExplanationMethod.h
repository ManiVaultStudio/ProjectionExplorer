#pragma once

#include "Explanation/DataTypes.h"

#include <vector>

namespace Explanation
{
    enum class Metric
    {
        NONE,
        EUCLIDEAN,
        VARIANCE,
        VALUE
    };

    class Method
    {
    public:
        virtual void  recompute(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) = 0;
        virtual float computeDimensionRank(const DataTable& dataset, int i, int j) = 0;
        virtual void computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) = 0;
    };
}
