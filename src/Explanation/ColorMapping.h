#pragma once

#include "Explanation/DataMatrix.h"
#include "Methods/ExplanationMethod.h"

#include <QColor>

#include <vector>

class ColorMapping
{
public:
    ColorMapping();

    const std::vector<QColor>& getPalette() { return _palette; }
    const std::vector<QColor>& getColors() const { return _colorMapping; }

    void recreate(DataMatrix& dataset);
    void recompute(DataMatrix& dataset, DataMatrix& dimRanking);

private:
    std::vector<QColor> _palette;
    std::vector<QColor> _colorMapping;

    std::vector<int> _dimAssignment;
};
