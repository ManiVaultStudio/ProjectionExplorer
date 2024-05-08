cmake_minimum_required(VERSION 3.17)

# -----------------------------------------------------------------------------
# Projection Explanation Plugin
# -----------------------------------------------------------------------------
set(PROJECT "ExplanationScatterplot")
PROJECT(${PROJECT})

# -----------------------------------------------------------------------------
# CMake Options
# -----------------------------------------------------------------------------
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /DWIN32 /EHsc /MP")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:LIBCMT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
endif(MSVC)

# -----------------------------------------------------------------------------
# Set install directory
# -----------------------------------------------------------------------------
# Check if the directory to the ManiVault installation has been provided
if(NOT DEFINED MV_INSTALL_DIR)
    set(MV_INSTALL_DIR "" CACHE PATH "Directory where ManiVault is installed")
    message(FATAL_ERROR "Please set MV_INSTALL_DIR to the directory where ManiVault is installed")
endif()
file(TO_CMAKE_PATH ${MV_INSTALL_DIR} MV_INSTALL_DIR)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
find_package(Qt6 COMPONENTS Widgets WebEngineWidgets OpenGL OpenGLWidgets REQUIRED)

# -----------------------------------------------------------------------------
# Source files
# -----------------------------------------------------------------------------
set(PLUGIN
    src/Common.h
    src/ScatterplotPlugin.h
    src/ScatterplotPlugin.cpp
)

set(PLUGIN_MOC_HEADERS
    src/ScatterplotPlugin.h
)

set(EXPLANATION
    src/Explanation/DataTypes.h
    src/Explanation/ExplanationModel.h
    src/Explanation/ExplanationModel.cpp
    src/Explanation/ColorMapping.h
    src/Explanation/ColorMapping.cpp
    src/Explanation/ConfidenceModel.h
    src/Explanation/ConfidenceModel.cpp
    src/Explanation/Histogram.h
    src/Explanation/Histogram.cpp
    #src/Explanation/Explanation.h
    #src/Explanation/Explanation.cpp
    src/Explanation/Methods/ExplanationMethod.h
    src/Explanation/Methods/SilvaEuclidean.h
    src/Explanation/Methods/SilvaEuclidean.cpp
    src/Explanation/Methods/SilvaVariance.h
    src/Explanation/Methods/SilvaVariance.cpp
    src/Explanation/Methods/ValueRanking.h
    src/Explanation/Methods/ValueRanking.cpp
)

set(UI
    src/ScatterplotWidget.h
    src/ScatterplotWidget.cpp
    src/ExplanationWidget.h
    src/ExplanationWidget.cpp
)

set(Actions
    src/ColoringAction.h
    src/ColoringAction.cpp
    src/MiscellaneousAction.h
    src/MiscellaneousAction.cpp
    src/PlotAction.h
    src/PlotAction.cpp
    src/PointPlotAction.h
    src/PointPlotAction.cpp
    src/PositionAction.h
    src/PositionAction.cpp
    src/RenderModeAction.h
    src/RenderModeAction.cpp
    src/ScalarAction.h
    src/ScalarAction.cpp
    src/ScalarSourceAction.h
    src/ScalarSourceAction.cpp
    src/SelectionAction.h
    src/SelectionAction.cpp
    src/SettingsAction.h
    src/SettingsAction.cpp
    src/SubsetAction.h
    src/SubsetAction.cpp
    src/ExportAction.h
    src/ExportAction.cpp
    src/DatasetsAction.h
    src/DatasetsAction.cpp
)

set(Models
    src/ColorSourceModel.h
    src/ColorSourceModel.cpp
    src/ScalarSourceModel.h
    src/ScalarSourceModel.cpp
)

set(SHADERS
    res/shaders/SelectionTool.frag
    res/shaders/SelectionTool.vert
)

set(AUX
    res/Resources.qrc
    src/ScatterplotPlugin.json
)

set(SOURCES ${PLUGIN} ${EXPLANATION} ${UI} ${Actions} ${Models})

source_group(Plugin FILES ${PLUGIN})
source_group(Explanation FILES ${EXPLANATION})
source_group(UI FILES ${UI})
source_group(Actions FILES ${Actions})
source_group(Models FILES ${Models})
source_group(Shaders FILES ${SHADERS})
source_group(Aux FILES ${AUX})

# -----------------------------------------------------------------------------
# CMake Target
# -----------------------------------------------------------------------------
add_library(${PROJECT} SHARED ${SOURCES} ${SHADERS} ${AUX})

qt_wrap_cpp(SCATTERPLOT_MOC ${PLUGIN_MOC_HEADERS} TARGET ${PROJECT})
target_sources(${PROJECT} PRIVATE ${SCATTERPLOT_MOC})

# -----------------------------------------------------------------------------
# Target include directories
# -----------------------------------------------------------------------------
target_include_directories(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/src)
target_include_directories(${PROJECT} PRIVATE "${MV_INSTALL_DIR}/$<CONFIGURATION>/include/")
target_include_directories(${PROJECT} PRIVATE ${PROJECT_SOURCE_DIR}/thirdparty/Eigen/include)

# -----------------------------------------------------------------------------
# Target properties
# -----------------------------------------------------------------------------
target_compile_features(${PROJECT} PRIVATE cxx_std_17)

# -----------------------------------------------------------------------------
# Target library linking
# -----------------------------------------------------------------------------
set(MV_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/lib")
set(PLUGIN_LINK_PATH "${MV_INSTALL_DIR}/$<CONFIGURATION>/$<IF:$<CXX_COMPILER_ID:MSVC>,lib,Plugins>")
set(MV_LINK_SUFFIX $<IF:$<CXX_COMPILER_ID:MSVC>,${CMAKE_LINK_LIBRARY_SUFFIX},${CMAKE_SHARED_LIBRARY_SUFFIX}>)

set(MV_LINK_LIBRARY "${MV_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}MV_Public${MV_LINK_SUFFIX}")
set(POINTDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}PointData${MV_LINK_SUFFIX}") 
set(CLUSTERDATA_LINK_LIBRARY "${PLUGIN_LINK_PATH}/${CMAKE_SHARED_LIBRARY_PREFIX}ClusterData${MV_LINK_SUFFIX}") 

target_link_libraries(${PROJECT} Qt6::Widgets)
target_link_libraries(${PROJECT} Qt6::WebEngineWidgets)
target_link_libraries(${PROJECT} Qt6::OpenGL)
target_link_libraries(${PROJECT} Qt6::OpenGLWidgets)
target_link_libraries(${PROJECT} "${MV_LINK_LIBRARY}")
target_link_libraries(${PROJECT} "${POINTDATA_LINK_LIBRARY}")
target_link_libraries(${PROJECT} "${CLUSTERDATA_LINK_LIBRARY}")

find_package(OpenMP)
if(OpenMP_CXX_FOUND)
    target_link_libraries(${PROJECT} OpenMP::OpenMP_CXX)
endif()

# -----------------------------------------------------------------------------
# Target installation
# -----------------------------------------------------------------------------
install(TARGETS ${PROJECT}
    RUNTIME DESTINATION Plugins COMPONENT PLUGINS # Windows .dll
    LIBRARY DESTINATION Plugins COMPONENT PLUGINS # Linux/Mac .so
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND "${CMAKE_COMMAND}"
        --install ${CMAKE_CURRENT_BINARY_DIR}
        --config $<CONFIGURATION>
        --prefix ${MV_INSTALL_DIR}/$<CONFIGURATION>
)

# -----------------------------------------------------------------------------
# Miscellaneous
# -----------------------------------------------------------------------------
if(MSVC)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<IF:$<CONFIG:DEBUG>,${MV_INSTALL_DIR}/debug,${MV_INSTALL_DIR}/release>)
    set_property(TARGET ${PROJECT} PROPERTY VS_DEBUGGER_COMMAND $<IF:$<CONFIG:DEBUG>,"${MV_INSTALL_DIR}/debug/ManiVault Studio.exe","${MV_INSTALL_DIR}/release/ManiVault Studio.exe">)
endif()
