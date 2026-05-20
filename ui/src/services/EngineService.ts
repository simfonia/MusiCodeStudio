/**
 * MusiCodeStudio Engine Service
 * 支援 Hybrid 通訊模式：
 * 1. Native IPC (WebView2): 極低延遲，適用於積木參數連動。
 * 2. HTTP POST (Legacy/Fallback): 適用於瀏覽器開發環境。
 */

export type EngineCommand = 
  | { action: 'transport_play' }
  | { action: 'transport_stop' }
  | { action: 'set_bpm', value: number }
  | { action: 'show_plugin_window', track: number }
  | { action: 'set_plugin_param', pluginName: string, paramID: string, value: number }
  | { action: 'show_audio_settings' }
  | { action: 'get_midi_inputs' }
  | { action: 'set_track_input', trackIndex: number, deviceName: string };

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
    console.log(`[EngineService] Received Event: ${event.type}`, event.detail);
    
    // 這裡可以觸發自定義的回呼或 RxJS Subject
    if (event.type === 'midi_signal') {
      window.dispatchEvent(new CustomEvent('MusiCode_MidiSignal', { detail: event.detail }));
    } else if (event.type === 'midi_inputs_list') {
      window.dispatchEvent(new CustomEvent('MusiCode_MidiInputsList', { detail: event.detail }));
    }
  }

  public static getInstance(): EngineService {
    if (!EngineService.instance) {
      EngineService.instance = new EngineService();
    }
    return EngineService.instance;
  }

  /**
   * 發送指令到 C++ 引擎 (優先使用 Native IPC，無則降級為 HTTP)
   */
  public async sendCommand(command: EngineCommand) {
    if (this.isNativeMode) {
      try {
        const juce = (window as any).__JUCE__;
        if (juce && juce.backend && typeof juce.backend.emitEvent === 'function') {
          juce.backend.emitEvent("__juce__invoke", {
            name: "postToBackend",
            params: [command],
            resultId: 0
          });
          return;
        }
      } catch (error) {
        console.error('[EngineService] Native IPC failed, falling back to HTTP', error);
      }
    }

    try {
      await fetch(this.engineUrl, {
        method: 'POST',
        mode: 'cors',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(command),
      });
    } catch (error) {
      // 靜默處理 HTTP 錯誤，避免在離線開發時產生過多 Log
    }
  }

  public play() {
    this.sendCommand({ action: 'transport_play' });
  }

  public stop() {
    this.sendCommand({ action: 'transport_stop' });
  }

  public setBPM(bpm: number) {
    this.sendCommand({ action: 'set_bpm', value: bpm });
  }

  public showPluginWindow(trackIndex: number) {
    this.sendCommand({ action: 'show_plugin_window', track: trackIndex });
  }

  public showAudioSettings() {
    this.sendCommand({ action: 'show_audio_settings' });
  }

  public getMidiInputs() {
    this.sendCommand({ action: 'get_midi_inputs' });
  }

  public setTrackInput(trackIndex: number, deviceName: string) {
    this.sendCommand({ action: 'set_track_input', trackIndex, deviceName });
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
