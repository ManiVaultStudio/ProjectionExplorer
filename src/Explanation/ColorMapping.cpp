#include "ColorMapping.h"

#include <unordered_set>
#include <algorithm>
#include <iostream>

static QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio)
{
    int r = color1.red() * (1 - ratio) + color2.red() * ratio;
    int g = color1.green() * (1 - ratio) + color2.green() * ratio;
    int b = color1.blue() * (1 - ratio) + color2.blue() * ratio;

    return QColor(r, g, b, 255);
}

ColorMapping::ColorMapping()
{
    // Initialize color palette
    _palette.resize(20); // "#31a09a", "#59a14f", "#A13237"
    const char* kelly_colors[] = { "#F3C300", "#875692", "#F38400", "#A1CAF1", "#BE0032", "#C2B280", "#59a14f", "#008856", "#E68FAC", "#0067A5", "#F99379", "#604E97", "#F6A600", "#B3446C", "#DCD300", "#882D17", "#8DB600", "#654522", "#E25822", "#2B3D26" };
    const char* visdist_colors_40[] = {
        "#696969",
        "#556b2f",
        "#a0522d",
        "#800000",
        "#808000",
        "#483d8b",
        "#008000",
        "#008080",
        "#4682b4",
        "#9acd32",
        "#00008b",
        "#daa520",
        "#8fbc8f",
        "#800080",
        "#d2b48c",
        "#ff4500",
        "#00ced1",
        "#ff8c00",
        "#c71585",
        "#0000cd",
        "#00ff00",
        "#00ff7f",
        "#dc143c",
        "#00bfff",
        "#f4a460",
        "#a020f0",
        "#adff2f",
        "#da70d6",
        "#ff00ff",
        "#1e90ff",
        "#db7093",
        "#f0e68c",
        "#fa8072",
        "#ffff54",
        "#dda0dd",
        "#90ee90",
        "#add8e6",
        "#7b68ee",
        "#7fffd4",
        "#ffc0cb"
    };

    const char* visdist_colors_60[] = {
    "#D72638",
    "#2f4f4f",
    "#556b2f",
    "#a0522d",
    "#228b22",
    "#7f0000",
    "#191970",
    "#708090",
    "#808000",
    "#3cb371",
    "#bc8f8f",
    "#663399",
    "#008080",
    "#bdb76b",
    "#cd853f",
    "#4682b4",
    "#d2691e",
    "#9acd32",
    "#cd5c5c",
    "#00008b",
    "#32cd32",
    "#daa520",
    "#7f007f",
    "#8fbc8f",
    "#b03060",
    "#48d1cc",
    "#ff4500",
    "#ff8c00",
    "#ffd700",
    "#c71585",
    "#0000cd",
    "#deb887",
    "#00ff00",
    "#ba55d3",
    "#8a2be2",
    "#00ff7f",
    "#4169e1",
    "#dc143c",
    "#00ffff",
    "#00bfff",
    "#9370db",
    "#0000ff",
    "#adff2f",
    "#ff6347",
    "#d8bfd8",
    "#ff00ff",
    "#1e90ff",
    "#db7093",
    "#eee8aa",
    "#ffff54",
    "#dda0dd",
    "#90ee90",
    "#87ceeb",
    "#ff1493",
    "#ffa07a",
    "#afeeee",
    "#ee82ee",
    "#7fffd4",
    "#ff69b4",
    "#ffb6c1"
    };
    for (int i = 0; i < _palette.size(); i++)
    {
        _palette[i].setNamedColor(kelly_colors[i]);
        _palette[i] = blendColors(_palette[i], QColor(255, 255, 255), 0.35);
        //_palette[i].setHsvF(_palette[i].hueF(), _palette[i].saturationF() * 0.8, _palette[i].lightnessF());
    }
}

void ColorMapping::recreate(DataMatrix& dataset)
{
    // Create color mapping
    _colorMapping.resize(dataset.getNumCols());
    for (int i = 0; i < _colorMapping.size(); i++)
    {
        QColor color = (i < _palette.size()) ? _palette[i] : QColor(30, 30, 30);
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

void ColorMapping::recompute(DataMatrix& dataset, DataMatrix& dimRanking)
{
    //bool lowRankBest = metric == Explanation::Metric::VARIANCE ? true : false;

    const int numDimensions = dimRanking.getNumCols();

    // Compute top ranked dimensions to assign colors to
    std::vector<int> topCount(numDimensions, 0);

    for (int i = 0; i < dimRanking.getNumRows(); i++)
    {
        float topRank = -std::numeric_limits<float>::max();

        for (int j = 0; j < dimRanking.getNumCols(); j++)
        {
            //if (dataset.isExcluded(j)) continue;

            float rank = dimRanking(i, j);

            if (rank > topRank) { topRank = rank; }
        }

        // Count every dimension that has the same rank
        for (int j = 0; j < dimRanking.getNumCols(); j++)
        {
            //if (dataset.isExcluded(j)) continue;

            float rank = dimRanking(i, j);

            if (rank >= topRank) topCount[j]++;
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
