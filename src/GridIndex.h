#pragma once

#include "graphics/Bounds.h"

#include <vector>

class GridIndex
{
public:
    GridIndex(mv::Bounds bounds, int resolution);

    void addPoint(int id, float x, float y);
    std::vector<int>& getBucket(float x, float y);

private:
    std::vector<std::vector<std::vector<int>>> _grid;

    mv::Bounds _bounds;
    int _resolution;
};
