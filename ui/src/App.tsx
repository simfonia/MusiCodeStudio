import React, { useState, useEffect } from 'react';
import { 
  Play, 
  Square, 
  Circle, 
  Settings, 
  Music, 
  Keyboard, 
  Layout, 
  Waves,
  Plus
} from 'lucide-react';
import './App.css';

// 引入服務與視圖
import { EngineService } from './services/EngineService';
import BlocklyView from './views/BlocklyView';
import PianoRollView from './views/PianoRollView';
import StaffView from './views/StaffView';

type ViewMode = 'staff' | 'piano' | 'blockly' | 'audio';

function App() {
  const [viewMode, setViewMode] = useState<ViewMode>('piano');
  const [bpm, setBpm] = useState(120);
  const [isRecording, setIsRecording] = useState(false);
  
  const engine = EngineService.getInstance();

  useEffect(() => {
    engine.setBPM(bpm);
  }, [bpm]);

  const handlePlay = () => engine.play();
  const handleStop = () => engine.stop();

  return (
    <div id="root">
      {/* Top Toolbar */}
      <header className="toolbar">
        <div className="logo">MusiCode Studio</div>
        
        <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
          <button className="transport-btn" onClick={handlePlay}><Play size={18} fill="currentColor" /></button>
          <button className="transport-btn" onClick={handleStop}><Square size={18} fill="currentColor" /></button>
          <button 
            className={`transport-btn record ${isRecording ? 'active' : ''}`}
            onClick={() => setIsRecording(!isRecording)}
          >
            <Circle size={14} fill="currentColor" />
          </button>
        </div>

        <div style={{ marginLeft: '40px', display: 'flex', alignItems: 'center', gap: '15px' }}>
          <div style={{ fontSize: '14px', color: 'var(--text-dim)' }}>
            BPM: <input 
              type="number" 
              value={bpm} 
              onChange={(e) => setBpm(Number(e.target.value))}
              style={{ width: '45px', border: 'none', background: 'none', fontWeight: 600, fontSize: '15px', color: 'var(--accent)' }}
            />
          </div>
          <div style={{ fontSize: '14px', color: 'var(--text-dim)' }}>
            4 / 4
          </div>
        </div>

        <div style={{ marginLeft: 'auto', display: 'flex', gap: '10px' }}>
          <button className="view-btn"><Settings size={18} /></button>
        </div>
      </header>

      {/* Main Content */}
      <main className="main-container">
        {/* Track List */}
        <aside className="track-list">
          <div className="track-header">
            <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '10px' }}>
              <span style={{ fontWeight: 600, fontSize: '14px' }}>Track 1</span>
              <div style={{ display: 'flex', gap: '4px' }}>
                <button className="view-btn" style={{ padding: '2px 6px', fontSize: '10px' }}>M</button>
                <button className="view-btn" style={{ padding: '2px 6px', fontSize: '10px' }}>S</button>
                <button className="view-btn" style={{ padding: '2px 6px', fontSize: '10px', color: 'var(--danger)' }}>R</button>
              </div>
            </div>
            <div style={{ background: 'var(--bg-app)', padding: '6px', borderRadius: '4px', fontSize: '12px', color: 'var(--text-dim)' }}>
              🎹 VSTi: Serum
            </div>
          </div>
          
          <button style={{ margin: '10px', padding: '10px', border: '1px dashed var(--border)', background: 'none', borderRadius: '6px', cursor: 'pointer', color: 'var(--text-dim)' }}>
            <Plus size={16} style={{ verticalAlign: 'middle', marginRight: '5px' }} />
            Add Track
          </button>
        </aside>

        {/* Editor Area */}
        <section className="editor-container">
          <nav className="view-selector">
            <button 
              className={`view-btn ${viewMode === 'staff' ? 'active' : ''}`}
              onClick={() => setViewMode('staff')}
            >
              <Music size={14} style={{ marginRight: '5px' }} /> 五線譜
            </button>
            <button 
              className={`view-btn ${viewMode === 'piano' ? 'active' : ''}`}
              onClick={() => setViewMode('piano')}
            >
              <Layout size={14} style={{ marginRight: '5px' }} /> 鋼琴捲軸
            </button>
            <button 
              className={`view-btn ${viewMode === 'blockly' ? 'active' : ''}`}
              onClick={() => setViewMode('blockly')}
            >
              <Keyboard size={14} style={{ marginRight: '5px' }} /> 積木
            </button>
            <button 
              className={`view-btn ${viewMode === 'audio' ? 'active' : ''}`}
              onClick={() => setViewMode('audio')}
            >
              <Waves size={14} style={{ marginRight: '5px' }} /> 波形
            </button>
          </nav>

          <div className="canvas-area">
            {viewMode === 'piano' && <PianoRollView />}
            {viewMode === 'blockly' && <BlocklyView />}
            {viewMode === 'staff' && <StaffView />}
            {viewMode === 'audio' && (
              <div style={{ padding: '40px', color: 'var(--text-dim)', textAlign: 'center' }}>
                AUDIO WAVEFORM VIEW (WAITING FOR WAVESURFER)
              </div>
            )}
          </div>
        </section>

        {/* Plugin Rack (Right Side) */}
        <aside className="plugin-rack">
          <div style={{ fontSize: '11px', fontWeight: 700, color: 'var(--text-dim)', marginBottom: '5px' }}>PLUGINS</div>
          <div className="plugin-slot instrument">🎹 Serum</div>
          <div className="plugin-slot effect">✨ Reverb</div>
          <div className="plugin-slot empty">+ Add FX</div>
          
          <div style={{ marginTop: 'auto', borderTop: '1px solid var(--border)', paddingTop: '10px' }}>
            <div style={{ fontSize: '11px', fontWeight: 700, color: 'var(--text-dim)' }}>MASTER</div>
            <div style={{ height: '100px', background: 'var(--bg-paper)', border: '1px solid var(--border)', borderRadius: '4px', marginTop: '5px', position: 'relative', overflow: 'hidden' }}>
              <div style={{ position: 'absolute', bottom: 0, left: 0, width: '100%', height: '60%', background: 'linear-gradient(to top, #4cd964, #ffcc00)' }}></div>
            </div>
          </div>
        </aside>
      </main>
    </div>
  );
}

export default App;
