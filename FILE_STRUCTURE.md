# MusiCodeStudio 專案結構 (FILE_STRUCTURE.md)

```text
MusiCodeStudio/
├── DESIGN.md                          # 專案設計計畫書
├── GEMINI.md                          # 開發規範與技術鐵律
├── MusiCodeStudio.code-workspace      # VS Code 工作區配置
├── package.json                       # 全域開發指令定義
│
├── engine/                            # 音訊引擎層 (C++)
│   ├── MusiCodeEngine/                # 專屬引擎實作
│   │   ├── Source/                    # 核心原始碼
│   │   │   ├── Main.cpp               # 入口程式
│   │   │   ├── MainWindow.cpp/h       # JUCE 視窗與佈局管理
│   │   │   ├── WebController.h        # [新增] WebView2 與 JS 事件通訊管理
│   │   │   ├── AudioEngine.cpp/h      # TE/Edit 核心管理 (委派中心)
│   │   │   ├── TransportController.cpp/h # 播放與傳輸控制
│   │   │   ├── RecordingController.cpp/h # 錄音狀態管理
│   │   │   ├── ParameterDispatcher.h  # [核心] 參數派發與 Dual Update
│   │   │   ├── TrackManager.h         # [核心] EditItemID 音軌定位與元數據
│   │   │   ├── MidiController.cpp/h   # MIDI 路由與訊號監控
│   │   │   ├── PluginController.cpp/h # 插件管理與視窗顯示
│   │   │   ├── CommandRouter.cpp/h    # JSON 指令解析中心
│   │   │   └── HttpServer.cpp/h       # 通訊與事件推送伺服器
│   │   └── CMakeLists.txt             # CMake 構建腳本
│   └── tracktion_engine/              # Tracktion Engine 原始碼 (子模組)
│
├── ui/                                # 前端介面層 (React/Vite)
│   └── src/
│       ├── App.tsx                    # 前端主入口框架
│       ├── services/
│       │   └── EngineService.ts       # 指令發送與事件監聽服務 (JUCE Native/HTTP Hybrid)
│       ├── components/                # 模組化 UI 組件
│       │   ├── TrackList.tsx          # 軌道列表容器
│       │   ├── TrackHeader.tsx        # 軌道頭 (含 MIDI LED 與路由選擇)
│       │   ├── PluginRack.tsx         # 插件控制面板
│       │   └── Toolbar.tsx            # 播放控制工具列 (含錄音、BPM、計時器)
│       └── views/                     # 專業視圖模組
│           ├── ArrangementView.tsx    # 編曲視圖 (Konva 繪製 Clip 與音符預覽)
│           ├── BlocklyView.tsx        # 積木編輯器
│           ├── PianoRollView.tsx      # 鋼琴捲軸 (Canvas 繪製詳細音符)
│           └── StaffView.tsx          # 五線譜視圖
│
├── log/                               # 開發紀錄與知識庫
│   ├── todo.md                        # 任務進度追蹤 (唯一入口)
│   ├── work/                          # 每日工作日誌 (HTML 診斷)
│   └── mappings/                      # 長期技術對照表
│       ├── Framework_API_Index.html   # API 範例索引
│       ├── ParameterDispatcher.html   # 參數模組手冊
│       ├── TrackManager.html          # 音軌模組手冊
│       └── MidiController.html        # MIDI 路由對照表
│
└── assets/                            # 靜態資源、圖示與備份
```

## 核心架構說明
- **後端引擎**：基於 JUCE + Tracktion Engine。採用 `CommandRouter` 模式處理非同步 JSON 指令。通訊邏輯已模組化至 `WebController`。
- **前端 UI**：基於 React + Konva。透過 `EngineService` 自動偵測 JUCE Native Bridge 或 HTTP Fallback 通訊。
- **通訊協議**：採用自定義 JSON 格式。所有音軌異動均透過 `EditItemID` 進行持久化連結。
- **開發規範**：所有重大變更需更新 `log/` 下的對應對照表與日誌。
