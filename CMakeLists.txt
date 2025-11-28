cmake_minimum_required(VERSION 3.21)
project(DesktopSAM LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
# Libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# For multi-config generators (Visual Studio)
foreach(CONFIG_TYPE Debug Release RelWithDebInfo MinSizeRel)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${CONFIG_TYPE})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${CONFIG_TYPE})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${CMAKE_BINARY_DIR}/${CONFIG_TYPE})
endforeach()

add_executable(DesktopSAM src/main.cpp
								 src/Algorithm.h src/Algorithm.cpp
                                 src/MainFrame.h src/MainFrame.cpp
                                 src/QtImageCanvas.h src/QtImageCanvas.cpp
                                 src/MainWindowGUI.h src/MainWindowGUI.cpp
                                 resources/app_icon.rc)
                         
set_target_properties(DesktopSAM PROPERTIES WIN32_EXECUTABLE TRUE)

# --- Dependencies ---
set(ONNXRUNTIME_DIR "${CMAKE_SOURCE_DIR}/libraries/install/onnxruntime")
set(OCV_DIR "${CMAKE_SOURCE_DIR}/libraries/install/OpenCV")
set(QT_DIR "${CMAKE_SOURCE_DIR}/libraries/install/Qt")

# Include directories
target_include_directories(DesktopSAM PRIVATE "${ONNXRUNTIME_DIR}/include" "${OCV_DIR}/include" "${QT_DIR}/include")

list(APPEND CMAKE_PREFIX_PATH "${QT_DIR}")
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

# Resources
qt_add_resources(RESOURCES resources.qrc)
target_sources(DesktopSAM PRIVATE ${RESOURCES})

# Link libraries
file(GLOB OCV_DEBUG "${OCV_DIR}/lib/Debug/*.lib")
file(GLOB OCV_RELEASE "${OCV_DIR}/lib/Release/*.lib")

file(GLOB ONNX_DEBUG "${ONNXRUNTIME_DIR}/lib/Debug/*.lib")
file(GLOB ONNX_RELEASE "${ONNXRUNTIME_DIR}/lib/Release/*.lib")

file(GLOB QT_DEBUG "${QT_DIR}/lib/debug/*.lib")
file(GLOB QT_RELEASE "${QT_DIR}/lib/*.lib")

target_link_libraries(DesktopSAM PRIVATE
    $<$<CONFIG:Debug>:${OCV_DEBUG} ${ONNX_DEBUG}>
    $<$<CONFIG:Release>:${OCV_RELEASE} ${ONNX_RELEASE}>
    $<$<CONFIG:RelWithDebInfo>:${OCV_RELEASE} ${ONNX_RELEASE}>
    $<$<CONFIG:MinSizeRel>:${OCV_RELEASE} ${ONNX_RELEASE}>
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)