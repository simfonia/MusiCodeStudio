cmake_minimum_required(VERSION 3.15)

project(MusiCodeEngine VERSION 0.1.0)

# 設定 WebView2 SDK 路徑 (必須在 add_subdirectory 之前，或至少在 juce_add_gui_app 之前)
set(JUCE_WEBVIEW2_PACKAGE_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}")

# 定義基礎路徑
set(TE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../tracktion_engine)
set(JUCE_PATH ${TE_PATH}/modules/juce)

# 加入 JUCE
add_subdirectory(${JUCE_PATH} ${CMAKE_BINARY_DIR}/juce)
add_subdirectory(${TE_PATH}/modules ${CMAKE_BINARY_DIR}/tracktion_modules)

# 建立 GUI App
juce_add_gui_app(MusiCodeEngine 
    PRODUCT_NAME "MusiCodeEngine"
    NEEDS_WEBVIEW2 TRUE
)

target_sources(MusiCodeEngine PRIVATE 
    Main.cpp
    Source/HttpServer.h
    Source/HttpServer.cpp
    Source/CommandRouter.h
    Source/CommandRouter.cpp
    Source/MainWindow.h
    Source/MainWindow.cpp
    Source/AudioEngine.h
    Source/AudioEngine.cpp
    Source/PluginController.h
    Source/PluginController.cpp
)

# 連結模組
target_link_libraries(MusiCodeEngine PRIVATE
    tracktion_engine
    juce::juce_audio_utils
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_devices
)

# 編譯定義
target_compile_definitions(MusiCodeEngine PRIVATE
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1
)

# 設定 C++20 標準
target_compile_features(MusiCodeEngine PRIVATE cxx_std_20)

# 產生傳統 JUCE Header (對 VS 2026 最好用)
juce_generate_juce_header(MusiCodeEngine)

# 針對 Visual Studio 的大物件優化
if (MSVC)
    target_compile_options(MusiCodeEngine PRIVATE /bigobj /FS)
endif()
