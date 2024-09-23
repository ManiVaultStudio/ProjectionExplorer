set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(PLUGIN_SOURCES
    ${DIR}/ProjectionExplorerPlugin.h
    ${DIR}/ProjectionExplorerPlugin.cpp
    ${DIR}/InputEventHandler.h
    ${DIR}/InputEventHandler.cpp
    ${DIR}/Lens.h
    ${DIR}/LocalValueComputation.h
    ${DIR}/LocalValueComputation.cpp
    ${DIR}/OffscreenBuffer.h
    ${DIR}/OffscreenBuffer.cpp
    PARENT_SCOPE
)

set(EXPLANATION_SOURCES
    ${DIR}/Explanation/ExplanationModel.h
    ${DIR}/Explanation/ExplanationModel.cpp
    ${DIR}/Explanation/DataMatrix.h
    ${DIR}/Explanation/DataMatrix.cpp
    ${DIR}/Explanation/ColorMapping.h
    ${DIR}/Explanation/ColorMapping.cpp
    ${DIR}/Explanation/Histogram.h
    ${DIR}/Explanation/Histogram.cpp
    ${DIR}/Explanation/Methods/ExplanationMethod.h
    ${DIR}/Explanation/Methods/ValueMethod.h
    ${DIR}/Explanation/Methods/ValueMethod.cpp
    PARENT_SCOPE
)

set(WIDGET_SOURCES
    ${DIR}/Widgets/ScatterplotWidget.h
    ${DIR}/Widgets/ScatterplotWidget.cpp
    ${DIR}/Widgets/ExplanationWidget.h
    ${DIR}/Widgets/ExplanationWidget.cpp
    PARENT_SCOPE
)

set(ACTION_SOURCES
    ${DIR}/Actions/SettingsAction.h
    ${DIR}/Actions/SettingsAction.cpp
    PARENT_SCOPE
)

set(PLUGIN_MOC_HEADERS
    ${DIR}/ProjectionExplorerPlugin.h
    PARENT_SCOPE
)

set(AUX
    #res/Resources.qrc
    ${DIR}/ProjectionExplorerPlugin.json
    PARENT_SCOPE
)
