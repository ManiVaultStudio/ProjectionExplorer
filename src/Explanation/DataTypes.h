#pragma once

#include <Eigen/Eigen>

using DataMatrix = Eigen::ArrayXXf;
using Neighbourhood = std::vector<int>;
using NeighbourhoodMatrix = std::vector<Neighbourhood>;

class DataTable
{
public:
    void setData(DataMatrix& data) { _data = data; _exclusionList.resize(_data.cols(), false); }

    int numDimensions() const { return _data.cols(); }
    int numPoints() const { return _data.rows(); }

    void excludeDimension(int dim) { _exclusionList[dim] = !_exclusionList[dim]; }
    bool isExcluded(int dim) const { return _exclusionList[dim]; }

    auto row(int row) const { return _data.row(row); }
    float operator()(int row, int col) const { return _data(row, col); }

private:
    DataMatrix _data;

    /** List of dimensions to exclude from analysis */
    std::vector<bool>       _exclusionList;
};
