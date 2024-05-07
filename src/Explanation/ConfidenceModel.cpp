#include "ConfidenceModel.h"

#include <chrono>
#include <iostream>

namespace
{
    void computeTopRankedDimensions(Explanation::Metric metric, DataTable& dataset, const DataMatrix& dimRanks, std::vector<int>& topDimensions)
    {
        int numPoints = dimRanks.rows();
        int numDimensions = dimRanks.cols();

        // Compute the top-ranked dimension for every point
        topDimensions.resize(numPoints);
        for (int i = 0; i < numPoints; i++)
        {
            float minRank = std::numeric_limits<float>::max();
            float maxRank = -std::numeric_limits<float>::max();
            int topRank = 0;
            for (int j = 0; j < numDimensions; j++)
            {
                if (dataset.isExcluded(j)) continue;

                float rank = dimRanks(i, j);

                if (metric == Explanation::Metric::VARIANCE)
                {
                    if (rank < minRank) { minRank = rank; topRank = j; }
                }
                else if (metric == Explanation::Metric::VALUE)
                {
                    if (rank > maxRank) { maxRank = rank; topRank = j; }
                }
            }
            topDimensions[i] = topRank;
        }
    }
}

void ConfidenceModel::silvaConfidence(const std::vector<int>& topDimensions, const DataMatrix& dimRanks, std::vector<float>& confidences)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dimRanks.rows();
    int numDimensions = dimRanks.cols();

    // Compute confidences
    confidences.resize(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = _confidenceNeighbourhoodMatrix[i];

        // Add top-1 rankings over all neighbouring points in vector
        std::vector<float> topRankings(numDimensions, 0);
        for (const int ni : neighbourhood)
        {
            int topDim = topDimensions[ni];

            topRankings[topDim] += abs(dimRanks(ni, topDim));
        }

        // Compute total ranking
        int topDim = topDimensions[i];
        float totalRank = 0;
        for (const int ni : neighbourhood)
            totalRank += abs(dimRanks(ni, topDim));

        // Divide all rankings by total rank values to get confidences
        float topRanking = topRankings[topDim];

        confidences[i] = topRanking / totalRank;

        if (std::isnan(confidences[i]))
            confidences[i] = 0;

        if (neighbourhood.size() == 0)
            confidences[i] = 0;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Confidence Elapsed time: " << elapsed.count() << " s\n";
}

void ConfidenceModel::simplifiedConfidence(const std::vector<int>& topDimensions, const DataMatrix& dimRanks, std::vector<float>& confidences)
{
    auto start = std::chrono::high_resolution_clock::now();

    int numPoints = dimRanks.rows();
    int numDimensions = dimRanks.cols();

    // Compute confidences
    confidences.resize(numPoints);
    for (int i = 0; i < numPoints; i++)
    {
        const std::vector<int>& neighbourhood = _confidenceNeighbourhoodMatrix[i];

        int topDim = topDimensions[i];

        int count = 0;
        for (const int ni : neighbourhood)
        {
            int nTopDim = topDimensions[ni];

            if (nTopDim == topDim) count++;
        }

        confidences[i] = ((float)count) / neighbourhood.size();

        if (neighbourhood.size() == 0)
            confidences[i] = 0;
    }

    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "Confidence Elapsed time: " << elapsed.count() << " s\n";
}

void ConfidenceModel::normalizeConfidences(std::vector<float>& confidences)
{
    // Normalize confidences
    float minVal = std::numeric_limits<float>::max();
    float maxVal = -std::numeric_limits<float>::max();
    for (int i = 0; i < confidences.size(); i++)
    {
        if (confidences[i] < minVal) minVal = confidences[i];
        if (confidences[i] > maxVal) maxVal = confidences[i];
    }
    std::cout << "Min val: " << minVal << " Max val: " << maxVal << std::endl;
    for (int i = 0; i < confidences.size(); i++)
    {
        confidences[i] = (confidences[i] - minVal) / (maxVal - minVal);
    }
}

void ConfidenceModel::computeConfidences(Explanation::Metric metric, DataTable& dataset, const DataMatrix& dimRanks, std::vector<float>& confidences)
{
    // Compute the top-ranked dimension for every point
    std::vector<int> topDimensions;
    computeTopRankedDimensions(metric, dataset, dimRanks, topDimensions);

    if (_method == ConfidenceMethod::SIMPLIFIED)
    {
        simplifiedConfidence(topDimensions, dimRanks, confidences);
    }
    else if (_method == ConfidenceMethod::SILVA)
    {
        silvaConfidence(topDimensions, dimRanks, confidences);
    }

    normalizeConfidences(confidences);
}
