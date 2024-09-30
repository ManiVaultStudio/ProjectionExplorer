#include "GridIndex.h"

#include <QDebug>

GridIndex::GridIndex(mv::Bounds bounds, int resolution) :
    _bounds(bounds),
    _resolution(resolution)
{
    _grid.resize(resolution, std::vector<std::vector<int>>(resolution, std::vector<int>()));
}

void GridIndex::addPoint(int id, float x, float y)
{
    float nx = (x - _bounds.getLeft()) / _bounds.getWidth();
    float ny = (y - _bounds.getBottom()) / _bounds.getHeight();

    int gridX = (int)(nx * _resolution);
    int gridY = (int)(ny * _resolution);

    _grid[gridX][gridY].push_back(id);
}

std::vector<int>& GridIndex::getBucket(float x, float y)
{
    float nx = (x - _bounds.getLeft()) / _bounds.getWidth();
    float ny = (y - _bounds.getBottom()) / _bounds.getHeight();

    int gridX = (int)(nx * _resolution);
    int gridY = (int)(ny * _resolution);

    return _grid[gridX][gridY];
}
