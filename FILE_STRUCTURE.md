# MusiCodeStudio 專案結構 (FILE_STRUCTURE.md)

- DESIGN.md (專案設計計畫書)
- MusiCodeStudio.code-workspace (VS Code 工作區檔案)
- engine/
  - tracktion_engine/ (Tracktion Engine 原始碼庫與子模組)
  - MusiCodeEngine/ (專屬 C++ 後端引擎專案)
    - CMakeLists.txt
    - Main.cpp
    - build/ (編譯輸出目錄)
- ui/ (前端 Vite + React 專案目錄)
  - src/
    - views/ (Staff, PianoRoll, Blockly 視圖)
    - services/ (EngineService 通訊服務)
- log/
  - todo.md (任務清單)
  - handover.md (任務交接)
  - details.md (技術細節)
  - work/ (每日工作日誌)

## [2026-05-02 更新]
... (內容省略)
└── .vscode/ (專案全域環境配置)
    ├── settings.json
    ├── tasks.json (新增：npm 指令任務)
    ├── launch.json (新增：F5 啟動 UI 配置)

## [2026-05-17 更新]
- engine/MusiCodeEngine/Source/
    - MidiController.cpp/h (新增：獨立 MIDI 管理與訊號監控模組)
    - PluginController.cpp/h (新增：插件生命週期與視圖管理)
    - CommandRouter.cpp/h (更新：整合 MIDI 指令與事件推送)
    - MainWindow.cpp/h (更新：整合 WebView2 與 MIDI 事件橋接)
- ui/src/
    - components/ (新增：模組化 UI 組件目錄)
        - TrackHeader.tsx (新增：仿 Waveform 軌道頭，含 MIDI 指示燈)
        - TrackList.tsx (新增：軌道管理佈局)
        - Toolbar.tsx (新增：頂部傳輸控制模組)
        - PluginRack.tsx (新增：右側插件控制鏈)
    - App.tsx (重構：全面組件化，提升可維護性)
- docs/TE_Cookbook.html (新增：Tracktion Engine 最佳實作指南)
- log/todo.md (更新：標註前端重構與 MIDI 監控完成)
