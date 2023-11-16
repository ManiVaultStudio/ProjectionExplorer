#pragma once

#include "DataTypes.h"
#include "Methods/ExplanationMethod.h"

#include <vector>

enum class ConfidenceMethod
{
    SILVA,
    SIMPLIFIED
};

class ConfidenceModel
{
public:
    void silvaConfidence(const std::vector<int>& topDimensions, const DataMatrix& dimRanks, std::vector<float>& confidences);

    void simplifiedConfidence(const std::vector<int>& topDimensions, const DataMatrix& dimRanks, std::vector<float>& confidences);

    void normalizeConfidences(std::vector<float>& confidences);

    void computeConfidences(Explanation::Metric metric, DataTable& dataset, const DataMatrix& dimRanks, std::vector<float>& confidences);

public:
    ConfidenceMethod        _method = ConfidenceMethod::SILVA;

    NeighbourhoodMatrix     _confidenceNeighbourhoodMatrix;
};
