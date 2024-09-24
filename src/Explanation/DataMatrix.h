#pragma once

#include <Eigen/Eigen>

#include <vector>
#include <QString>

using ArrayXXfr = Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;
using ArrayXXfc = Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>;

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

    void resize(int rows, int cols) { _data.resize(rows, cols); }

    const std::vector<QString>& getDimensionNames() { return _dimNames; }
    int getNumRows() const { return _data.rows(); }
    int getNumCols() const { return _data.cols(); }

    ArrayXXfc& getData() { return _data; }

    float& operator()(int row, int col)
    {
        return _data(row, col);
    }

    //float operator()(int row, int col)
    //{
    //    return _data(row, col);
    //}

private:
    ArrayXXfc _data;
    std::vector<QString> _dimNames;
};
