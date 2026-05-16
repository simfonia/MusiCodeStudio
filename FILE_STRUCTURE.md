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

## [2026-05-16 更新]
- CMakeLists.txt (更新：連結 juce_gui_extra, 整合 WebView2 SDK)
- Source/CommandRouter.h/cpp (新增：模組化指令路由)
- Source/MainWindow.h/cpp (重構：引入 MainComponent 與音訊優先初始化)
- Source/HttpServer.h/cpp (重構：委託 CommandRouter 處裡指令)
- ui/src/services/EngineService.ts (更新：實作 Hybrid Native IPC 模式)
- log/work/2026-05-16.md (更新：記錄手術式重構與 Native IPC 實作細節)
- log/mappings/Engine_Modules_Mapping.html (更新：加入 CommandRouter 與 Hybrid UI 說明)
