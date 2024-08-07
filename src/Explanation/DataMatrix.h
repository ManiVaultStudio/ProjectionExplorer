#pragma once

#include <Eigen/Eigen>

namespace mv
{
    template<class T>
    class Dataset;
}
class Points;

class DataMatrix
{
public:
    static void fromDataset(mv::Dataset<Points> dataset, DataMatrix& dataMatrix);

    int getNumRows() { return _data.rows(); }
    int getNumCols() { return _data.cols(); }

    float operator()(int row, int col)
    {
        return _data(row, col);
    }

private:
    Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> _data;
};
