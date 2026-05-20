cmake_minimum_required(VERSION 3.15)
project(MidiTo4OscTestApp VERSION 1.0.0)

# 定義基礎路徑
set(TE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/tracktion_engine)
set(JUCE_PATH ${TE_PATH}/modules/juce)

# 加入 JUCE 與 Tracktion 模組
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/juce)
add_subdirectory(${TE_PATH}/modules ${CMAKE_BINARY_DIR}/tracktion_modules)

# 建立 GUI App (為了 Message Loop)
juce_add_gui_app(MidiTo4OscTestApp 
    PRODUCT_NAME "MidiTo4OscTest"
)

target_sources(MidiTo4OscTestApp PRIVATE 
    Main.cpp
)

# 連結模組
target_link_libraries(MidiTo4OscTestApp PRIVATE
    tracktion_engine
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_audio_devices
)

# 設定 C++20 標準
target_compile_features(MidiTo4OscTestApp PRIVATE cxx_std_20)

juce_generate_juce_header(MidiTo4OscTestApp)

if (MSVC)
    target_compile_options(MidiTo4OscTestApp PRIVATE /bigobj /FS)
endif()
