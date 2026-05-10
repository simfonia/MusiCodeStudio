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
- **HttpServer (h/cpp)**：封裝 `juce::StreamingSocket` 監聽器。
- **MainWindow (h/cpp)**：管理 UI 佈局與 AudioEngine 實例。
- **AudioEngine (h/cpp)**：封裝 `te::Engine` 與 `te::Edit`，處理 Transport 邏輯。

### 3. Tracktion Engine API 修復 (核心)
- **初始化**：必須顯式設置 `Edit::Options::editProjectItemID` 否則觸發斷言。
- **執行緒**：Transport 控制（Play/Stop）必須回傳 Message Thread 執行。
- **類型系統**：新版 TE 使用 `tracktion::TimePosition` 與 `tracktion::BeatPosition` 強型別。
- **BPM 設定**：透過 `edit->tempoSequence.getTempoAt(TimePosition()).setBpm(value)` 進行。

### 4. 環境配置
- **VS Code**：`.vscode/launch.json` 配置 `Launch UI (npm dev)`。
- **知識庫**：建立 `Framework_API_Index.html` 紀錄實測成功的 API Snippets。
