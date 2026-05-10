# MusiCodeStudio 任務清單

## [初期任務]
### 程式碼架構與管理
- [x] 建立「表驅動管理」開發規範與 `GEMINI.md`
- [x] 建立首批 HTML 功能對照表 (`AudioEngine`, `UI`)
- [x] 依據功能對照表進行 `Main.cpp` 模組化拆解 (HttpServer, CommandParser, AudioEngine)
- [ ] 持續維護 `log/mappings/` 下的 HTML 功能對照表
- [x] VS Code 與 VS 2026 環境隔離設定 (build_vscode)

### 引擎核心與播放器
- [x] HTTP POST 通訊協定實作與 CORS 處理
- [x] 前端 EngineService 基本指令對接 (Play/Stop)


## [中期任務] 引擎核心與播放器
- [x] 解決 te::Edit 私有建構子問題，重新掛載音訊引擎 (核心優先)
- [x] 調查內建插件建立失敗原因 (已改用 PluginCache::createNewPlugin 與 FourOscPlugin::xmlTypeName 確保正確初始化)
- [x] 實作 C++ 端 JSON 指令解析器，對齊前端 EngineService (支援 transport_play, transport_stop, set_bpm)

- [ ] 整合系統預設音訊輸出 (AudioDeviceSelector)

## [核心開發] 積木合成器 (原 #wavecode 功能轉移)
- [ ] **自定義 DSP 插件架構**：在 C++ 端建立可動態串接的 Plugin 容器
- [ ] **DSP 模組庫**：實作基礎合成器元件 (VCO, VCF, VCA, ADSR)
- [ ] **積木轉譯層**：設計合成器積木並產生聲音接線圖 (JSON)
- [ ] **即時參數同步**：實現積木滑桿與 C++ DSP 參數的低延遲連動

## [UI/UX] 介面精修
- [ ] 鋼琴捲軸與 C++ 軌道數據同步
- [ ] 五線譜 OSMD 渲染與播放指標追蹤
