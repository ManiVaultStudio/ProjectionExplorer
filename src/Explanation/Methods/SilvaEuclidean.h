#pragma once

#include "ExplanationMethod.h"

namespace
{
    float sqrMag(const DataTable& dataset, int a, int b);

    float sqrMag(const DataTable& dataset, const std::vector<float>& a, int b);

    float distance(const DataTable& dataset, int a, int b);

    float distContrib(const DataTable& dataset, int p, int r, int dim);

    float distContrib(const DataTable& dataset, const std::vector<float>& p, int r, int dim);

    float localDistContrib(const DataTable& dataset, int p, int dim, const Neighbourhood& neighbourhood);

    std::vector<float> computeNDCentroid(const DataTable& dataset);

    float globalDistContrib(const DataTable& dataset, const std::vector<float>& centroid, int dim);

    float dimensionRank(const DataTable& dataset, int p, int dim, const Neighbourhood& neighbourhood, const std::vector<float>& globalDistContribs);
}

class EuclideanMethod : public Explanation::Method
{
public:
    void recompute(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix) override;
    float computeDimensionRank(const DataTable& dataset, int i, int j) override;
    void computeDimensionRank(const DataTable& dataset, const std::vector<unsigned int>& selection, std::vector<float>& dimRanking) override;

private:
    void computeCentroid(const DataTable& dataset);
    void computeGlobalContribs(const DataTable& dataset);
    void computeLocalContribs(const DataTable& dataset, NeighbourhoodMatrix& neighbourhoodMatrix);

    std::vector<float> _centroid;
    std::vector<float> _globalDistContribs;
    DataMatrix _localDistContribs;
};
