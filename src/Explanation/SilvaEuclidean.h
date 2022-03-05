#pragma once

#include <Eigen/Eigen>

namespace
{
    float sqrMag(const Eigen::ArrayXXf& dataset, int a, int b);

    float sqrMag(const Eigen::ArrayXXf& dataset, const std::vector<float>& a, int b);

    float distance(const Eigen::ArrayXXf& dataset, int a, int b);

    float distContrib(const Eigen::ArrayXXf& dataset, int p, int r, int dim);

    float distContrib(const Eigen::ArrayXXf& dataset, const std::vector<float>& p, int r, int dim);

    float localDistContrib(const Eigen::ArrayXXf& dataset, int p, int dim, const std::vector<int>& neighbourhood);

    std::vector<float> computeNDCentroid(const Eigen::ArrayXXf& dataset);

    float globalDistContrib(const Eigen::ArrayXXf& dataset, const std::vector<float>& centroid, int dim);

    float dimensionRank(const Eigen::ArrayXXf& dataset, int p, int dim, const std::vector<int>& neighbourhood, const std::vector<float>& globalDistContribs);
}

class EuclideanMetric
{
public:
    void recompute(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);
    float computeDimensionRank(const Eigen::ArrayXXf& dataset, int i, int j);

private:
    void computeCentroid(const Eigen::ArrayXXf& dataset);
    void computeGlobalContribs(const Eigen::ArrayXXf& dataset);
    void computeLocalContribs(const Eigen::ArrayXXf& dataset, std::vector<std::vector<int>>& neighbourhoodMatrix);

    std::vector<float> _centroid;
    std::vector<float> _globalDistContribs;
    Eigen::ArrayXXf _localDistContribs;
};
