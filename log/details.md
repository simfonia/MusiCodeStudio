# MusiCode Studio 技術細節 (details.md)

## 2026-05-01 (專案發想與架構確立)
... (內容省略)

## 2026-05-02 (通訊層與環境穩定化)
... (內容省略)

## 2026-05-03 (管理架構升級與核心重構)

### 1. 表驅動管理 (Table-Driven Management)
- **目的**：應對大型專案的複雜性，防止功能耦合。
- **工具**：使用 HTML 對照表，提供清晰的「功能-檔案-函數」映射。

### 2. C++ 核心模組化
- **HttpServer (h/cpp)**：將通訊邏輯從 `Main.cpp` 抽離，封裝 `juce::StreamingSocket` 監聽器。
- **MainWindow (h/cpp)**：將 UI 佈局與視窗管理抽離，並負責啟動 HttpServer。
- **Main.cpp**：僅保留 `JUCEApplication` 類別與應用程式 entry point，作為全域生命週期管理。

### 3. 編譯優化
- 更新 `CMakeLists.txt`，採用明確的 `target_sources` 宣告，便於後續擴充。
