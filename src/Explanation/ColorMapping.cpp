#include "ColorMapping.h"

#include <QFile>
#include <QTextStream>

#include <unordered_set>
#include <algorithm>

#include <iostream>
#include <QDebug>

namespace
{
    QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio)
    {
        int r = color1.red() * (1 - ratio) + color2.red() * ratio;
        int g = color1.green() * (1 - ratio) + color2.green() * ratio;
        int b = color1.blue() * (1 - ratio) + color2.blue() * ratio;

        return QColor(r, g, b, 255);
    }

    void loadPalette(std::vector<QColor>& palette, QString resourceName)
    {
        QFile inputFile(resourceName);
        if (inputFile.open(QIODevice::ReadOnly))
        {
            QTextStream in(&inputFile);
            while (!in.atEnd())
            {
                QString line = in.readLine();
                
                QColor color;
                color.setNamedColor(line);
                blendColors(color, QColor(255, 255, 255), 0.35);
                palette.push_back(color);
            }
            inputFile.close();
        }
    }
}

ColorMapping::ColorMapping()
{

    std::vector<QString> paletteNames = {
        ":/projection_explorer/colors/kelly20.colors",
        ":/projection_explorer/colors/vis_dist_40.colors",
        ":/projection_explorer/colors/vis_dist_60.colors",
        ":/projection_explorer/colors/vis_dist_100.colors",
    };
    _palettes.resize(paletteNames.size());

    for (int i = 0; i < paletteNames.size(); i++)
    {
        loadPalette(_palettes[i], paletteNames[i]);
    }

    switchPalettes(0);
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

void ColorMapping::switchPalettes(int index)
{
    if (index < 0)
    {
        qWarning() << "Asked to switch to negative palette index " << index << ". Enabling palette 0..";
        index = 0;
    }
    if (index >= _palettes.size())
    {
        index = _palettes.size() - 1;
        qWarning() << "Asked to switch to palette " << index << " but only " << _palettes.size() << " are loaded..";
    }

    _palette = _palettes[index];
}
