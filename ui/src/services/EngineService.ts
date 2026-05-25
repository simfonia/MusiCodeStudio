/**
 * MusiCodeStudio Engine Service
 * 支援 Hybrid 通訊模式：
 * 1. Native IPC (WebView2): 極低延遲，適用於積木參數連動。
 * 2. HTTP POST (Legacy/Fallback): 適用於瀏覽器開發環境。
 */

export type EngineCommand = 
  | { action: 'transport_play' }
  | { action: 'transport_stop' }
  | { action: 'transport_record' }
  | { action: 'set_bpm', value: number }
  | { action: 'get_tracks' }
  | { action: 'get_clip_notes', clipID: string }
  | { action: 'show_plugin_window', trackID?: string, track?: number }
  | { action: 'set_plugin_param', pluginName: string, paramID: string, value: number }
  | { action: 'show_audio_settings' }
  | { action: 'get_midi_inputs' }
  | { action: 'set_track_input', trackID?: string, trackIndex?: number, deviceName: string };

export class EngineService {
  private static instance: EngineService;
  private engineUrl = 'http://127.0.0.1:9001';
  private isNativeMode = false;

  private constructor() {
    this.isNativeMode = typeof (window as any).__JUCE__ !== 'undefined';
    console.log(`[EngineService] Initialized in ${this.isNativeMode ? 'NATIVE' : 'HTTP'} Mode`);

    // 監聽來自 C++ 的事件
    window.addEventListener('MusiCodeEngineEvent' as any, (event: CustomEvent) => {
      this.handleEngineEvent(event.detail);
    });
  }

  private handleEngineEvent(event: { type: string, detail: any }) {
    // 僅對重要事件進行 Log，避免 tracks_list 與 playhead_sync 灌爆 Console
    if (event.type !== 'tracks_list' && event.type !== 'midi_signal' && event.type !== 'playhead_sync') {
      console.log(`[EngineService] Received Event: ${event.type}`, event.detail);
    }
    
    // 這裡可以觸發自定義的回呼或 RxJS Subject
    if (event.type === 'midi_signal') {
      window.dispatchEvent(new CustomEvent('MusiCode_MidiSignal', { detail: event.detail }));
    } else if (event.type === 'midi_inputs_list') {
      window.dispatchEvent(new CustomEvent('MusiCode_MidiInputsList', { detail: event.detail }));
    } else if (event.type === 'tracks_list') {
      window.dispatchEvent(new CustomEvent('MusiCode_TracksList', { detail: event.detail }));
    } else if (event.type === 'clip_notes_list') {
      window.dispatchEvent(new CustomEvent('MusiCode_ClipNotesList', { detail: event.detail }));
    } else if (event.type === 'playhead_sync') {
      window.dispatchEvent(new CustomEvent('MusiCode_PlayheadSync', { detail: event.detail }));
    } else if (event.type === 'engine_message') {
      const { messageType, message } = event.detail;
      const style = messageType === 'warning' ? 'color: orange; font-weight: bold;' : 'color: cyan;';
      console.log(`%c[Tracktion Engine ${messageType.toUpperCase()}] ${message}`, style);
    }
  }

  public static getInstance(): EngineService {
    if (!EngineService.instance) {
      EngineService.instance = new EngineService();
    }
    return EngineService.instance;
  }

  public async sendCommand(command: EngineCommand) {
    const win = window as any;
    const commandStr = JSON.stringify(command);

    // 1. 嘗試最直接的 JUCE 8 全域函數 (withNativeFunction 預設行為)
    if (typeof win.postToBackend === 'function') {
      win.postToBackend(command);
      return;
    }

    // 2. 嘗試 JUCE 內部的 backend 物件路徑
    const juce = win.__JUCE__;
    if (juce && juce.backend) {
      if (typeof juce.backend.postToBackend === 'function') {
        juce.backend.postToBackend(command);
        return;
      }
      // 嘗試透過 emitEvent 呼叫 (某些版本的 JUCE 實作)
      if (typeof juce.backend.emitEvent === 'function') {
        try {
          juce.backend.emitEvent("__juce__invoke", {
            name: "postToBackend",
            params: [command],
            resultId: 0
          });
          return;
        } catch (e) { console.error('JUCE emitEvent failed', e); }
      }
    }

    // 3. 最終回退：HTTP POST (開發環境或 Native 失敗時)
    try {
      await fetch(this.engineUrl, {
        method: 'POST',
        mode: 'no-cors', // 避免 CORS 預檢失敗
        headers: { 'Content-Type': 'application/json' },
        body: commandStr,
      });
    } catch (error) {
      // 靜默處理
    }
  }

  public play() {
    this.sendCommand({ action: 'transport_play' });
  }

  public stop() {
    this.sendCommand({ action: 'transport_stop' });
  }

  public record() {
    this.sendCommand({ action: 'transport_record' });
  }

  public setBPM(bpm: number) {
    this.sendCommand({ action: 'set_bpm', value: bpm });
  }

  public getTracks() {
    this.sendCommand({ action: 'get_tracks' });
  }

  public getClipNotes(clipID: string) {
    this.sendCommand({ action: 'get_clip_notes', clipID });
  }

  public showPluginWindow(trackID: string | number) {
    if (typeof trackID === 'string') {
      this.sendCommand({ action: 'show_plugin_window', trackID });
    } else {
      this.sendCommand({ action: 'show_plugin_window', track: trackID });
    }
  }

  public showAudioSettings() {
    this.sendCommand({ action: 'show_audio_settings' });
  }

  public getMidiInputs() {
    this.sendCommand({ action: 'get_midi_inputs' });
  }

  public setTrackInput(trackID: string | number, deviceName: string) {
    if (typeof trackID === 'string') {
      this.sendCommand({ action: 'set_track_input', trackID, deviceName });
    } else {
      this.sendCommand({ action: 'set_track_input', trackIndex: trackID, deviceName });
    }
  }

  private lastParamUpdateTimes: Record<string, number> = {};
  private paramThrottleMs = 32; // 約 30fps

  public setPluginParameter(pluginName: string, paramID: string, value: number) {
    const key = `${pluginName}_${paramID}`;
    const now = Date.now();
    const lastTime = this.lastParamUpdateTimes[key] || 0;

    if (now - lastTime < this.paramThrottleMs) {
      return; // 節流
    }

    this.lastParamUpdateTimes[key] = now;
    this.sendCommand({ action: 'set_plugin_param', pluginName, paramID, value });
  }
}
