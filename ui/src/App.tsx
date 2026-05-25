import { useState, useEffect } from 'react';
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
import ArrangementView from './views/ArrangementView';

type ViewMode = 'arrangement' | 'staff' | 'piano' | 'blockly' | 'audio';

function App() {
  const [viewMode, setViewMode] = useState<ViewMode>('arrangement');
  const [bpm, setBpm] = useState(120);
  const [isRecording, setIsRecording] = useState(false);
  const [isMetronomeOn, setIsMetronomeOn] = useState(false);
  const [isLoopOn, setIsLoopOn] = useState(false);
  const [timeSig, setTimeSig] = useState({ n: 4, d: 4 });
  const [filterFreq, setFilterFreq] = useState(0.5);
  
  // 模擬軌道資料 (後續可由 C++ 同步)
  const [tracks, setTracks] = useState<any[]>([
    { id: '1001', name: 'Track 1 (4OSC)', clips: [] }
  ]);
  const [selectedClipNotes, setSelectedClipNotes] = useState<any[]>([]);

  const engine = EngineService.getInstance();

  useEffect(() => {
    engine.setBPM(bpm);

    // 監聽來自 C++ 的軌道列表同步
    const handleTracksSync = (e: any) => {
      if (e.detail && Array.isArray(e.detail)) {
        setTracks(e.detail);
      }
    };

    // 監聽來自 C++ 的音符詳情 (點擊 Clip 後觸發)
    const handleClipNotesSync = (e: any) => {
      if (e.detail && e.detail.notes && Array.isArray(e.detail.notes)) {
        setSelectedClipNotes(e.detail.notes);
        setViewMode('piano'); // 自動切換到鋼琴捲軸
      }
    };

    window.addEventListener('MusiCode_TracksList' as any, handleTracksSync);
    window.addEventListener('MusiCode_ClipNotesList' as any, handleClipNotesSync);
    
    return () => {
      window.removeEventListener('MusiCode_TracksList' as any, handleTracksSync);
      window.removeEventListener('MusiCode_ClipNotesList' as any, handleClipNotesSync);
    };
  }, [bpm]);

  const handleSelectClip = (clipID: string) => {
    console.log('[App] Selecting Clip:', clipID);
    engine.getClipNotes(clipID);
  };

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
        isMetronomeOn={isMetronomeOn}
        setIsMetronomeOn={setIsMetronomeOn}
        isLoopOn={isLoopOn}
        setIsLoopOn={setIsLoopOn}
        timeSig={timeSig}
        setTimeSig={setTimeSig}
      />

      <main className="main-container">
        <TrackList tracks={tracks} />

        <section className="editor-container">
          <nav className="view-selector">
            <button className={`view-btn ${viewMode === 'arrangement' ? 'active' : ''}`} onClick={() => setViewMode('arrangement')}>
              <Layout size={14} style={{ marginRight: '5px' }} /> 編曲
            </button>
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
            {viewMode === 'arrangement' && (
              <ArrangementView 
                tracks={tracks} 
                bpm={bpm} 
                timeSigNumerator={timeSig.n}
                timeSigDenominator={timeSig.d}
                onSelectClip={handleSelectClip} 
              />
            )}
            {viewMode === 'piano' && <PianoRollView notes={selectedClipNotes} />}
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
