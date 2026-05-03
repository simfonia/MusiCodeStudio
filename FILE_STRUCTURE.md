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

## [2026-05-03 更新]
C:\Workspace\MusiCodeStudio\
├── GEMINI.md (新增專案開發規範：表驅動管理)
├── log\
│   └── mappings\ (功能與檔案對照表 - HTML 格式)
│       ├── AudioEngine_FunctionsTable.html
│       ├── UI_FunctionsTable.html
│       └── Framework_API_Index.html (新增：框架 API 知識庫)
│   └── work\
│       └── 2026-05-03.html (今日工作摘要)
