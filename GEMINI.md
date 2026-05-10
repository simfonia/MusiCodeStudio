# MusiCode Studio 開發規範

## 1. 程式碼管理：表驅動管理 (Table-Driven Management)
為了確保大型專案的穩定性，避免「修 A 壞 B」，所有開發必須遵循以下流程：

### 1.1 功能與檔案對照表 (Function Mapping Tables)
- 在 `log/mappings/` 目錄下建立並維護各模組的對照表 (如：`AudioEngine_FunctionsTable.html`, `UI_FunctionsTable.html`)。
- 每份對照表必須為 HTML 格式，包含：功能模組、實作檔案、關鍵函數/指令、外部依賴、備註。
- **異動即更新**：任何函數的新增、刪除或重大邏輯變更，必須同步更新 HTML 對照表。

### 1.2 模組化開發
- 每個功能應盡可能獨立為程式檔案模組。
- 嚴禁在未經對照表確認的情況下，將多個不相關的功能混雜在同一個檔案中。
- C++ 引擎端應逐步將 `Main.cpp` 中的邏輯拆分至專屬的類別與標頭檔。

### 1.3 操作規範
- **先查表再動刀**：進行任何修改前，必須先讀取相關的功能對照表，確認影響範圍。
- **先查證再實作**：涉及外部框架 (Tracktion Engine, JUCE, Blockly 等) 的 API 調用時，必須**隨時查閱並修正 `log/mappings/Framework_API_Index.html`**。若為新功能，應主動搜尋最新文檔並更新索引；若發現舊有索引錯誤，應立即修正。
- **完成後匯整**：各功能模組穩定後，視情況匯整為函式庫 (Library)。

## 2. 核心架構鐵律 (Architecture Mandates)
### 2.1 插件控制：不依賴索引
- **動態尋找**：嚴禁依賴 `trackIndex` 或 `pluginIndex` 進行控制。Tracktion 的插件列表在執行時會因為預設插件 (Volume/Pan) 或使用者操作而變動。
- **名稱/類型匹配**：一律使用 `pluginName` 或 `findFirstPluginOfType<T>` 來定位目標。

### 2.2 參數同步：ValueTree 優先
- **雙重更新**：更新參數時，應同時呼叫 `setParameter` 並直接修改 `plugin->state.setProperty`。這能確保 DSP 執行緒獲得最穩定的同步。
- **執行緒限制**：任何涉及 `AutomatableParameter` 或 `ValueTree` 的變更，必須包裹在 `juce::MessageManager::callAsync` 中執行。

### 2.3 前端效能：強制節流
- **高頻事件**：對於滑桿等高頻事件，前端 `EngineService` 必須實作 30Hz 以上的節流 (Throttling)，防止後端 HTTP 請求堆積導致播放卡頓。

## 3. 開發流程規範 (SOP)
- **階段性授權執行**：
  - 複雜任務必須拆分為三個階段，各階段需獲得使用者確認後方可繼續：
    1. **研究與提案 (Proposal)**：分析現狀與對照表，提出方案及其優缺點。
    2. **計畫確認 (Plan)**：列出具體修改檔案與邏輯細節。
    3. **實作與驗證 (Execution)**：執行修改。**嚴禁**在測試成功前更新進度日誌 (`todo.md`, `#log`)。
- **API 查證鐵律 (禁止猜測)**：
  - 涉及外部框架時，禁止憑直覺編寫。必須先在專案範例 (Examples) 或測試 (Tests) 中搜尋正確用法，必要時查閱官方文檔。

## 3. 技術與環境規範
- **C++ 引擎**：
  - 使用 Visual Studio 17 2022 (amd64)。
  - 編譯參數必須包含 `/FS` 與 `/bigobj`。
- **通訊協議**：
  - 使用 HTTP POST (Port 9001) 進行 IPC 指令傳遞。
  - C++ 端需處理 CORS `OPTIONS` 預檢請求。
- **視覺風格**：Studio Paper (淺色、紙張感、Color-coded)。

## 3. 日誌與文件
- 遵循全域 `GEMINI.md` 的追加原則，嚴禁覆蓋 `log/` 下的歷史紀錄。
- 每次重大變更後，更新 `log/todo.md` 與當日工作日誌。
