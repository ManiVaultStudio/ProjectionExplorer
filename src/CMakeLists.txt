set(DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(PLUGIN_SOURCES
    ${DIR}/ProjectionExplorerPlugin.h
    ${DIR}/ProjectionExplorerPlugin.cpp
    PARENT_SCOPE
)

set(WIDGET_SOURCES
    ${DIR}/Widgets/ScatterplotWidget.h
    ${DIR}/Widgets/ScatterplotWidget.cpp
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
