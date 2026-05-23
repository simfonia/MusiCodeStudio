cmake_minimum_required(VERSION 3.15)

project(RecordingModuleTestApp VERSION 0.1.0)

# 定義基礎路徑
set(TE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/tracktion_engine)
set(JUCE_PATH ${TE_PATH}/modules/juce)
set(SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/MusiCodeEngine/Source)

# 加入 JUCE
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/juce)
add_subdirectory(${TE_PATH}/modules ${CMAKE_BINARY_DIR}/tracktion_modules)

# 建立 GUI App
juce_add_gui_app(RecordingModuleTestApp 
    PRODUCT_NAME "RecordingModuleTest"
)

target_sources(RecordingModuleTestApp PRIVATE 
    Main.cpp
    ${SOURCE_PATH}/TransportController.h
    ${SOURCE_PATH}/TransportController.cpp
    ${SOURCE_PATH}/RecordingController.h
    ${SOURCE_PATH}/RecordingController.cpp
    ${SOURCE_PATH}/TrackManager.h # 可能會用到
)

# 連結模組
target_link_libraries(RecordingModuleTestApp PRIVATE
    tracktion_engine
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# 設定 C++20 標準
target_compile_features(RecordingModuleTestApp PRIVATE cxx_std_20)

# 產生傳統 JUCE Header
juce_generate_juce_header(RecordingModuleTestApp)

if (MSVC)
    target_compile_options(RecordingModuleTestApp PRIVATE /bigobj /FS)
endif()
