import React, { useState, useEffect } from 'react';
import { Music, Keyboard, Layout, Waves } from 'lucide-react';
import './App.css';

// 引入服務與組件
import { EngineService } from './services/EngineService';
import Toolbar from './components/Toolbar';
import TrackList from './components/TrackList';
import PluginRack from './components/PluginRack';
import BlocklyView from './views/BlocklyView';
import PianoRollView from './views/PianoRollView';
import StaffView from './views/StaffView';

type ViewMode = 'staff' | 'piano' | 'blockly' | 'audio';

function App() {
  const [viewMode, setViewMode] = useState<ViewMode>('piano');
  const [bpm, setBpm] = useState(120);
  const [isRecording, setIsRecording] = useState(false);
  const [filterFreq, setFilterFreq] = useState(0.5);
  
  // 模擬軌道資料 (後續可由 C++ 同步)
  const [tracks] = useState([
    { id: 1, name: 'Track 1 (4OSC)' }
  ]);

  const engine = EngineService.getInstance();

  useEffect(() => {
    engine.setBPM(bpm);
  }, [bpm]);

  const handleFilterChange = (val: number) => {
    setFilterFreq(val);
    engine.setPluginParameter('4OSC', 'filterFreq', val);
  };

  return (
    <div id="root">
      <Toolbar 
        bpm={bpm} 
        setBpm={setBpm} 
        isRecording={isRecording} 
        setIsRecording={setIsRecording} 
      />

      <main className="main-container">
        <TrackList tracks={tracks} />

        <section className="editor-container">
          <nav className="view-selector">
            <button className={`view-btn ${viewMode === 'staff' ? 'active' : ''}`} onClick={() => setViewMode('staff')}>
              <Music size={14} style={{ marginRight: '5px' }} /> 五線譜
            </button>
            <button className={`view-btn ${viewMode === 'piano' ? 'active' : ''}`} onClick={() => setViewMode('piano')}>
              <Layout size={14} style={{ marginRight: '5px' }} /> 鋼琴捲軸
            </button>
            <button className={`view-btn ${viewMode === 'blockly' ? 'active' : ''}`} onClick={() => setViewMode('blockly')}>
              <Keyboard size={14} style={{ marginRight: '5px' }} /> 積木
            </button>
            <button className={`view-btn ${viewMode === 'audio' ? 'active' : ''}`} onClick={() => setViewMode('audio')}>
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

        <PluginRack 
          filterFreq={filterFreq} 
          onFilterChange={handleFilterChange} 
        />
      </main>
    </div>
  );
}

export default App;
