# MusiCodeStudio 任務清單

## [初期任務]
### 程式碼架構與管理
- [x] 建立「表驅動管理」開發規範與 `GEMINI.md`
- [x] 建立首批 HTML 功能對照表 (`AudioEngine`, `UI`)
- [x] 依據功能對照表進行 `Main.cpp` 模組化拆解 (HttpServer, CommandParser, AudioEngine)
- [ ] 持續維護 `log/mappings/` 下的 HTML 功能對照表
- [x] VS Code 與 VS 2026 環境隔離設定 (build_vscode)
- [x] **粹取 TE 官方範例知識庫**：建立 `docs/TE_Cookbook.html` (2026-05-17 完成)
- [x] **同步更新 API 索引**：更新 `log/mappings/Framework_API_Index.html` (2026-05-17 完成)


### 引擎核心與播放器
- [x] HTTP POST 通訊協定實作與 CORS 處理
- [x] 前端 EngineService 基本指令對接 (Play/Stop)


## [中期任務] 引擎核心與播放器
- [x] 解決 te::Edit 私有建構子問題，重新掛載音訊引擎 (核心優先)
- [x] 調查內建插件建立失敗原因 (已改用 PluginCache::createNewPlugin 與 FourOscPlugin::xmlTypeName 確保正確初始化)
- [x] 實作 C++ 端 JSON 指令解析器，對齊前端 EngineService (支援 transport_play, transport_stop, set_bpm)
- [x] 整合系統預設音訊輸出 (AudioDeviceSelector) (2026-05-16 完成)

## [核心開發] 積木錄音室
- [x] **軌道輸入路由**：實作軌道頭輸入選擇 (MIDI / MIC) (2026-05-17 完成)
- [x] **即時監聽機制**：實作 MIDI 鍵盤彈奏發聲 (預設路由至 4OSC) (2026-05-18 完成)
- [x] **4OSC 濾波器校正**：實作雙重更新同步模式 (Dual Update Pattern)，解決掃頻無效問題 (2026-05-18 完成)
- [x] **架構模組化重構**：抽離 <code>ParameterDispatcher</code>、<code>TrackManager</code>、<code>TransportController</code> 與 <code>RecordingController</code> (2026-05-23 完成)
- [x] **錄音功能**：實作 MIDI Clip 的錄製流程與指令對接 (2026-05-23 完成)
- [x] **錄音 UI 整合**：實作 Toolbar 錄音鈕、軌道武裝 (R) 按鈕與持久化 ID 渲染 (2026-05-23 完成)
- [x] **C++ Web 通訊模組化**：建立 <code>WebController</code> 封裝 WebView2 與 JS 橋接邏輯 (2026-05-24 完成)
- [ ] **自定義 DSP 插件架構**：在 C++ 端建立可動態串接的 Plugin 容器
- [ ] **多軌錄音管理**：支援多軌同時錄音與路由切換
- [ ] **Undo/Redo**： 系統初步架構 (基於 Command 模式)。
- [ ] **DSP 模組庫**：實作基礎合成器元件 (VCO, VCF, VCA, ADSR)
- [ ] **積木轉譯層**：設計合成器積木並產生聲音接線圖 (JSON)
- [ ] **實作合成器積木**：#wavecode 功能轉移
- [x] **即時參數同步**：實現積木滑桿與 C++ DSP 參數的低延遲連動 (2026-05-16 完成)
- [x] **持久化 ID 系統**：將音軌索引遷移至 EditItemID (2026-05-20 完成)
- [x] **專業拍號系統**：支援動態分母 (6/8, 5/4) 與 BPM 自動換算 (2026-05-24 完成)
- [x] **錄音預點功能 (Count-in)**：實作 1 小節 Native Count-in 與模式自動協調 (2026-05-24 完成)

## [UI/UX] 介面精修
- [x] **編曲視圖渲染**：實作 Konva.js 繪製 Clip 色塊與音符預覽雲圖 (2026-05-24 完成)
- [x] **拍點網格系統**：實作 Bar.Beat 標尺與自動對齊網格 (2026-05-24 完成)
- [x] **鋼琴捲軸與 C++ 軌道數據同步**：實作 500ms 自動同步機制與 ArrangementView Clip 渲染 (2026-05-23 完成)
- [x] **鋼琴捲軸音符載入**：對接 <code>get_clip_notes</code> 指令，點擊 Clip 後在 PianoRoll 顯示音符 (2026-05-24 完成)
- [x] **鋼琴捲軸自動捲動**：切換視圖時自動垂直居中首個音符 (2026-05-24 完成)
- [x] **Loop 播放穩定化**：修復 1.1 處 Loop 死鎖，實作 I/O 標籤拖動 (2026-05-24 完成)
- [x] **Clip 管理**：實作 Clip 刪除功能與二次確認 (2026-05-24 完成)
- [ ] **混音器面板 (Mixer)**：實作軌道 Volume Fader、Pan 與 Mute/Solo 功能
- [ ] 五線譜 OSMD 渲染與播放指標追蹤
- [x] **仿 Waveform 軌道頭與訊號監控**：實作 30Hz MIDI Signal LED，支援精確力度感應 (2026-05-18 完成)
- [x] **前端組件模組化重構**：拆分 Toolbar, TrackList, PluginRack (2026-05-17 完成)
- [ ] **自定義音軌名稱編輯**：支援在 UI 直接修改軌道名稱並同步至後端
