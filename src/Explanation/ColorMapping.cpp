#include "ColorMapping.h"

#include <unordered_set>
#include <algorithm>
#include <iostream>

ColorMapping::ColorMapping()
{
    // Initialize color palette
    _palette.resize(20); // "#31a09a", "#59a14f", "#A13237"
    const char* kelly_colors[] = { "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#59a14f", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26" };
    for (int i = 0; i < _palette.size(); i++)
    {
        _palette[i].setNamedColor(kelly_colors[i]);
    }
}

void ColorMapping::recreate(const DataMatrix& dataset)
{
    // Create color mapping
    _colorMapping.resize(dataset.cols());
    for (int i = 0; i < _colorMapping.size(); i++)
    {
        QColor color = (i < _palette.size()) ? _palette[i] : QColor(180, 180, 180);
        _colorMapping[i] = color;
    }

    _dimAssignment.resize(_palette.size());
    std::iota(_dimAssignment.begin(), _dimAssignment.end(), 0);
}

void computeNewColorAssignment(const std::vector<int> oldMapping, const std::vector<int>& topDims, std::vector<int>& newMapping)
{
    int size = std::min(topDims.size(), oldMapping.size());

    // Only consider top-K colors (K = min(size of palette, numDims)
    std::vector<int> kTopDims(size);
    for (int i = 0; i < size; i++)
    {
        kTopDims[i] = topDims[i];
    }

    std::unordered_set<int> topDimSet(kTopDims.begin(), kTopDims.end());

    std::vector<bool> dimsSet(size, false);
    for (int i = 0; i < size; i++)
    {
        if (topDimSet.find(oldMapping[i]) != topDimSet.end())
        {
            newMapping[i] = oldMapping[i];
            topDimSet.erase(oldMapping[i]);
            dimsSet[i] = true;
        }
    }
    for (int i = 0; i < size; i++)
    {
        if (!dimsSet[i])
        {
            newMapping[i] = *topDimSet.begin();
            topDimSet.erase(topDimSet.begin());
        }
    }
}

void ColorMapping::recompute(const DataMatrix& dimRanking, Explanation::Metric metric)
{
    bool lowRankBest = metric == Explanation::Metric::VARIANCE ? true : false;

    const int numDimensions = dimRanking.cols();

    // Compute top ranked dimensions to assign colors to
    std::vector<int> topCount(numDimensions, 0);

    for (int i = 0; i < dimRanking.rows(); i++)
    {
        float topRank = lowRankBest ? std::numeric_limits<float>::max() : -std::numeric_limits<float>::max();

        for (int j = 0; j < dimRanking.cols(); j++)
        {
            float rank = dimRanking(i, j);

            if (lowRankBest) { if (rank < topRank) { topRank = rank; } }
            else { if (rank > topRank) { topRank = rank; } }
        }

        // Count every dimension that has the same rank
        for (int j = 0; j < dimRanking.cols(); j++)
        {
            float rank = dimRanking(i, j);
            if (lowRankBest) { if (rank <= topRank) topCount[j]++; }
            else { if (rank >= topRank) topCount[j]++; }
        }
    }
    for (int i = 0; i < numDimensions; i++)
    {
        std::cout << "Top count: " << i << " " << topCount[i] << std::endl;
    }
    std::vector<int> indices(numDimensions);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&](int a, int b) {return topCount[a] > topCount[b]; });

    for (int i = 0; i < numDimensions; i++)
    {
        std::cout << "Top indices: " << indices[i] << std::endl;
    }

    computeNewColorAssignment(_dimAssignment, indices, _dimAssignment);

    std::vector<QColor> newMapping(numDimensions);
    for (int i = 0; i < newMapping.size(); i++)
    {
        newMapping[i] = QColor(180, 180, 180);
    }
    int numTopDimensions = std::min(_palette.size(), newMapping.size());
    for (int i = 0; i < numTopDimensions; i++)
    {
        newMapping[_dimAssignment[i]] = _palette[i];
    }
    _colorMapping = newMapping;
}
