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

## 2026-05-10 (插件控制與效能優化)

### 1. 插件生命週期管理 (核心發現)
- **索引偏移 (Index Offset)**：`AudioTrack` 建立時會自動附加 `VolumeAndPan` 與 `LevelMeter` 插件。這些預設插件的位置在不同版本或 Edit 加載過程中會變動，導致依賴 `trackIndex` 或 `pluginIndex` 的控制極不穩定。
- **解決方案**：實作全域搜尋機制。透過遍歷 `te::getAudioTracks(*edit)` 並匹配 `plugin->getName()`，實現與索引無關的強健控制。

### 2. 參數同步與執行緒安全
- **執行緒鎖定**：`AutomatableParameter` 與 `ValueTree` 的變更嚴格要求在 **Message Thread** 執行。在背景 HTTP 執行緒操作會觸發 `TRACKTION_ASSERT_MESSAGE_THREAD` 斷言。
- **手勢衝突 (Gesture Assert)**：頻繁呼叫 `parameterChangeGestureBegin/End` 會導致 `gestureCount` 衝突。
- **最佳實踐**：
    1. 使用 `juce::MessageManager::callAsync` 包裹參數操作。
    2. 直接修改 `plugin->state.setProperty` (ValueTree) 是最穩定觸發 DSP 更新的方式。

### 3. 通訊效能優化
- **指令節流 (Throttling)**：前端滑桿產生的高頻指令會造成後端 Message Queue 堆積，導致 Transport 指令 (Stop) 被延遲處理。
- **解決方案**：在 `EngineService.ts` 實作 32ms (30fps) 的指令節流，將請求量減少 90% 以上。

### 4. UI 互動修復
- **禁止符號 (Prohibited Drag)**：瀏覽器誤將滑桿滑動判定為 Drag & Drop 操作。透過 `onDragStart={(e) => e.preventDefault()}` 解決。

