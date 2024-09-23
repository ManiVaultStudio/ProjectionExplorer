#include "DataMatrix.h"

#include <PointData/PointData.h>
#include <PointData/DimensionsPickerAction.h>

#include <QDebug> //////////////////
#include "util/Timer.h" ////////////
#include <iostream> ////////////

void DataMatrix::fromDataset(mv::Dataset<Points> dataset, DataMatrix& dataMatrix)
{
    Timer t("Copy");

    int numPoints = dataset->getNumPoints();
    int numDimensions = dataset->getNumDimensions();

    dataMatrix._data.resize(numPoints, numDimensions);

    // Copy the full data to the data matrix
    dataset->visitFromBeginToEnd([&dataMatrix, &dataset, numPoints, numDimensions](auto begin, auto end)
        {
            //std::copy(begin, end, dataMatrix._data.data());
            for (int i = 0; i < numPoints; i++)
            {
                for (int d = 0; d < numDimensions; d++)
                {
                    dataMatrix(i, d) = *(begin + (i * numDimensions + d));
                    //std::cout << *(begin + (i * numDimensions + d)) << std::endl;
                }
            }

            //dataMatrix = Eigen::Map<Eigen::Array<float, Eigen::Dynamic, Eigen::Dynamic, Eigen::ColMajor>>(dataMatrix._data.data(), dataMatrix.getNumRows(), dataMatrix.getNumCols());
        });

    // Find the list of enabled dimension indices
    std::vector<bool> enabledDimBools = dataset->getDimensionsPickerAction().getEnabledDimensions();
    int numEnabledDims = std::count(enabledDimBools.begin(), enabledDimBools.end(), true);
    std::vector<int> enabledDims(numEnabledDims);
    int d = 0;
    for (int i = 0; i < enabledDimBools.size(); i++)
    {
        if (enabledDimBools[i])
            enabledDims[d++] = i;
    }
    //std::cout << "PRE: " << dataMatrix._data << std::endl;
    // Only retain the enabled dimensions in the data matrix
    //if (numDimensions == 3)
    //{
    //    ArrayXXfc newData = dataMatrix._data(Eigen::all, { 2, 1 });
    //    dataMatrix._data = newData;
    //}
    //else dataMatrix._data = dataMatrix._data(Eigen::all, enabledDims);
    //std::cout << "POST: " <<  dataMatrix._data << std::endl;
}
