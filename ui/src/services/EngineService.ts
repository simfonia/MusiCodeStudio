/**
 * MusiCodeStudio Engine Service
 * 使用 HTTP POST 指令模式，穩定控制 C++ 引擎
 */

export type EngineCommand = 
  | { action: 'transport_play' }
  | { action: 'transport_stop' }
  | { action: 'set_bpm', value: number }
  | { action: 'show_plugin_window', track: number }
  | { action: 'set_plugin_param', pluginName: string, paramID: string, value: number };

export class EngineService {
  private static instance: EngineService;
  private engineUrl = 'http://127.0.0.1:9001';

  private constructor() {
    console.log('[EngineService] Initialized in HTTP Mode');
  }

  public static getInstance(): EngineService {
    if (!EngineService.instance) {
      EngineService.instance = new EngineService();
    }
    return EngineService.instance;
  }

  /**
   * 發送指令到 C++ 引擎 (使用 HTTP POST)
   */
  public async sendCommand(command: EngineCommand) {
    console.log('[EngineService] Sending Command:', command);
    
    try {
      const response = await fetch(this.engineUrl, {
        method: 'POST',
        mode: 'cors',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(command),
      });

      if (response.ok) {
        // console.log('[EngineService] Command delivered successfully');
      }
    } catch (error) {
      console.warn('[EngineService] Engine not reachable. Make sure MusiCodeEngine is running.');
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
