#pragma once

#include "PointData.h"

#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>

#include <Eigen/Eigen>

using namespace hdps;

void findNeighbourhood(Dataset<Points> projection, int centerId, float radius, std::vector<int>& neighbourhood);

float sqrMag(Dataset<Points> dataset, int a, int b);

float sqrMag(Dataset<Points> dataset, const std::vector<float>& a, int b);

float distance(Dataset<Points> dataset, int a, int b);

float distContrib(Dataset<Points> dataset, int p, int r, int dim);

float distContrib(Dataset<Points> dataset, const std::vector<float>& p, int r, int dim);

float localDistContrib(Dataset<Points> dataset, int p, int dim, const std::vector<int>& neighbourhood);

std::vector<float> computeNDCentroid(Dataset<Points> dataset);

float globalDistContrib(Dataset<Points> dataset, const std::vector<float>& centroid, int dim);

float dimensionRank(Dataset<Points> dataset, int p, int dim, const std::vector<int>& neighbourhood, const std::vector<float>& globalDistContribs);

void computeDimensionRanking(Dataset<Points> dataset, Dataset<Points> projection, std::vector<float>& dimRanking);

class Explanation
{
public:
    void setDataset(Dataset<Points> dataset, Dataset<Points> projection);

    void computeDimensionRanking(Eigen::ArrayXXi& dimRanking);

private:
    // Precomputation
    void computeNeighbourhoodMatrix();
    void computeCentroid();
    void computeGlobalContrib();

private:
    Dataset<Points> _dataset;
    Dataset<Points> _projection;

    // Precomputed values
    std::vector<std::vector<int>> _neighbourhoodMatrix;
    std::vector<float> _centroid;
    std::vector<float> _globalDistContribs;
};
