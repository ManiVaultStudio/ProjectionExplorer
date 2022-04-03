#pragma once

#include "ExplanationMethod.h"

namespace
{
    float sqrMag(const DataMatrix& dataset, int a, int b);

    float sqrMag(const DataMatrix& dataset, const std::vector<float>& a, int b);

    float distance(const DataMatrix& dataset, int a, int b);

    float distContrib(const DataMatrix& dataset, int p, int r, int dim);

    float distContrib(const DataMatrix& dataset, const std::vector<float>& p, int r, int dim);

    float localDistContrib(const DataMatrix& dataset, int p, int dim, const Neighbourhood& neighbourhood);

    std::vector<float> computeNDCentroid(const DataMatrix& dataset);

    float globalDistContrib(const DataMatrix& dataset, const std::vector<float>& centroid, int dim);

    float dimensionRank(const DataMatrix& dataset, int p, int dim, const Neighbourhood& neighbourhood, const std::vector<float>& globalDistContribs);
}

class EuclideanMethod : public Explanation::Method
{
public:
    void recompute(const DataMatrix& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) override;
    float computeDimensionRank(const DataMatrix& dataset, int i, int j) override;

private:
    void computeCentroid(const DataMatrix& dataset);
    void computeGlobalContribs(const DataMatrix& dataset);
    void computeLocalContribs(const DataMatrix& dataset, NeighbourhoodMatrix& neighbourhoodMatrix);

    std::vector<float> _centroid;
    std::vector<float> _globalDistContribs;
    DataMatrix _localDistContribs;
};
