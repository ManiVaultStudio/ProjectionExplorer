#pragma once

#include "Methods/ExplanationMethod.h"

#include <QColor>

#include <vector>

class ColorMapping
{
public:
    ColorMapping();

    const std::vector<QColor>& getPalette() { return _palette; }
    const std::vector<QColor>& getColors() { return _colorMapping; }

    void recreate(const DataMatrix& dataset);
    void recompute(const DataMatrix& dimRanking, Explanation::Metric metric);

private:
    std::vector<QColor> _palette;
    std::vector<QColor> _colorMapping;

    std::vector<int> _dimAssignment;
};
