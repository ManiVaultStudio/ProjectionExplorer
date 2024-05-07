#pragma once

#include <vector>
#include <QDebug>

class Histogram
{
public:
    Histogram(int numBins) :
        _minRange(0),
        _maxRange(1),
        _range(1),
        _numDataPoints(0),
        _highestBinValue(0),
        _bins(numBins, 0)
    {

    }

    int getNumDataPoints()
    {
        return _numDataPoints;
    }

    int getHighestBinValue()
    {
        return _highestBinValue;
    }

    const std::vector<int>& getBins()
    {
        return _bins;
    }

    void setRange(float min, float max)
    {
        _minRange = min;
        _maxRange = max;
        _range = _maxRange - _minRange;
    }

    void addData(const std::vector<float>& values)
    {
        for (int i = 0; i < values.size(); i++)
            addDataValue(values[i]);
    }

    void addDataValue(float value)
    {
        float nv = (value - _minRange) / _range;
        int bin = std::min((int)(nv * _bins.size()), (int) _bins.size() - 1);

        _bins[bin]++;

        _numDataPoints++;

        if (_bins[bin] > _highestBinValue) _highestBinValue = _bins[bin];
    }

private:
    float _minRange;
    float _maxRange;
    float _range;

    int _numDataPoints;
    int _highestBinValue;
    std::vector<int> _bins;
};
