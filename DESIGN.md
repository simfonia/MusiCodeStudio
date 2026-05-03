# MusiCodeStudio 設計計畫書 (DESIGN.md)

## 1. 專案願景
MusiCode Studio 是一個結合了 **教育性**、**邏輯性** 與 **專業音訊處理** 的下一代數位音訊工作站 (DAW)。它旨在打破程式設計與音樂創作之間的藩籬，讓使用者能透過積木、鋼琴捲軸、五線譜與專業 VST 插件進行全方位的音樂創作。

## 2. 核心架構：雙層分離架構
為了兼顧開發效率 (Vite HMR) 與專業效能 (VST 低延遲)，專案採用「Web UI + C++ Engine」的分離設計。

### A. 前端 UI 層 (Vite + React + TS)
- **職責**：介面繪製、Blockly 邏輯、鋼琴捲軸互動、五線譜渲染、波形顯示。
- **關鍵庫**：
  - **Blockly**: 邏輯編曲。
  - **OpenSheetMusicDisplay (OSMD) & VexFlow**: 五線譜渲染。
  - **Konva.js**: 高效能 Canvas 鋼琴捲軸與音高條。
  - **Wavesurfer.js**: 波形顯示。
  - **Shadcn UI**: 簡潔、教育友善的淺色系介面。

### B. 後端引擎層 (C++ / JUCE / Tracktion Engine)
- **職責**：VST/VSTi 載入與管理、音訊排程 (Scheduling)、MIDI/Audio 錄音、ASIO/CoreAudio 驅動對接。
- **技術棧**：
  - **Tracktion Engine (TE)**: 核心 DAW 邏輯與訊號流管理。
  - **Tauri (Rust)**: 作為 Web 與 C++ 之間的橋樑（或直接使用 WebSocket 通訊）。

## 3. 核心功能規劃
### A. 三重視圖切換 (Tri-View Switching)
- 同一軌道的 MIDI 資訊可自由切換三種檢視模式：
  1. **五線譜模式 (Staff View)**: 紙張質感背景，支援 MusicXML 匯入/匯出。
  2. **鋼琴捲軸模式 (Piano Roll)**: 音高色彩同步，支援精確 MIDI 編輯。
  3. **積木模式 (Blockly)**: 演算法編曲，支援即時渲染至時間軸。

### B. 專業 VST 支援
- **類 Waveform 橫向訊號流**：[輸入] -> [Clip] -> [VSTi/VST 插件鏈] -> [Master]。
- **浮動視窗**：支援 VST 原生 GUI 調整。
- **參數映射**：Blockly 可直接控制 VST 參數，實現自動化 (Automation)。

### C. 錄音與時間系統
- **單軌錄音**：支援 MIC 音訊錄音與 MIDI 鍵盤錄音。
- **全域控制**：BPM 速度、拍號 (Time Signature)、節拍器 (Pre-count)。
- **延遲補償**：確保錄音與播放絕對對齊。

## 4. 視覺與教學設計 (UI/UX)
- **風格**：Studio Paper (淺色、極簡、蘋果風格)。
- **色彩編碼 (Color-coded)**：音高顏色在積木、捲軸、譜面間保持一致。
- **防呆設計**：限制 VSTi 必須放在插件鏈首位，視覺化訊號流顏色 (MIDI=黃, Audio=藍)。

## 5. 開發階段
1. **Phase 1**: 環境搭建 (Tauri + C++ Engine 原型)。
2. **Phase 2**: 核心通訊協定設計 (JSON IPC)。
3. **Phase 3**: UI 框架與多重視圖開發。
4. **Phase 4**: VST 整合與錄音功能實作。
5. **Phase 5**: 教育功能與範例專案開發。

---
*Created on: 2026-05-01*
