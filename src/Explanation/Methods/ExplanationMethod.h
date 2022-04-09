#pragma once

#include <Eigen/Eigen>

#include <vector>

using DataMatrix = Eigen::ArrayXXf;
using Neighbourhood = std::vector<int>;
using NeighbourhoodMatrix = std::vector<Neighbourhood>;

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
        virtual void  recompute(const DataMatrix& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) = 0;
        virtual float computeDimensionRank(const DataMatrix& dataset, int i, int j) = 0;
        virtual void computeDimensionRank(const DataMatrix& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) = 0;
    };
}
