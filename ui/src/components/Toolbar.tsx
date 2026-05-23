import React, { useState, useRef, useEffect } from 'react';
import { Play, Square, Circle, Settings, Volume2 } from 'lucide-react';
import { EngineService } from '../services/EngineService';

interface ToolbarProps {
  bpm: number;
  setBpm: (bpm: number) => void;
  isRecording: boolean;
  setIsRecording: (rec: boolean) => void;
}

const Toolbar: React.FC<ToolbarProps> = ({ bpm, setBpm, isRecording, setIsRecording }) => {
  const [showSettingsMenu, setShowSettingsMenu] = useState(false);
  const settingsMenuRef = useRef<HTMLDivElement>(null);
  const engine = EngineService.getInstance();

  const handleShowAudioSettings = () => {
    engine.showAudioSettings();
    setShowSettingsMenu(false);
  };

  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (settingsMenuRef.current && !settingsMenuRef.current.contains(event.target as Node)) {
        setShowSettingsMenu(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const handleToggleRecord = () => {
    if (isRecording) {
      engine.stop();
      setIsRecording(false);
    } else {
      engine.record();
      setIsRecording(true);
    }
  };

  const handleStop = () => {
    engine.stop();
    setIsRecording(false);
  };

  return (
    <header className="toolbar">
      <div className="logo">MusiCode Studio</div>
      
      <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
        <button className="transport-btn" onClick={() => engine.play()}><Play size={18} fill="currentColor" /></button>
        <button className="transport-btn" onClick={handleStop}><Square size={18} fill="currentColor" /></button>
        <button 
          className={`transport-btn record ${isRecording ? 'active' : ''}`}
          onClick={handleToggleRecord}
          style={{ position: 'relative' }}
        >
          <Circle size={14} fill="currentColor" />
          {isRecording && (
            <span className="record-blink" style={{
              position: 'absolute', top: '-2px', right: '-2px', 
              width: '8px', height: '8px', borderRadius: '50%', 
              backgroundColor: '#ff2d55', border: '2px solid white'
            }}></span>
          )}
        </button>
      </div>

      <div style={{ marginLeft: '40px', display: 'flex', alignItems: 'center', gap: '15px' }}>
        <div style={{ fontSize: '14px', color: 'var(--text-dim)' }}>
          BPM: <input 
            type="number" 
            value={bpm} 
            onChange={(e) => setBpm(Number(e.target.value))}
            style={{ width: '45px', background: 'none', border: 'none', borderBottom: '1px solid transparent', fontWeight: 600, fontSize: '15px', color: 'var(--accent)' }}
          />
        </div>
        <div style={{ fontSize: '14px', color: 'var(--text-dim)' }}>4 / 4</div>
      </div>

      <div style={{ marginLeft: 'auto', display: 'flex', gap: '10px', position: 'relative' }} ref={settingsMenuRef}>
        <button 
          className={`view-btn ${showSettingsMenu ? 'active' : ''}`} 
          onClick={() => setShowSettingsMenu(!showSettingsMenu)}
        >
          <Settings size={18} />
        </button>

        {showSettingsMenu && (
          <div className="dropdown-menu" style={{
            position: 'absolute', top: '100%', right: 0, marginTop: '8px',
            backgroundColor: 'var(--bg-paper)', border: '1px solid var(--border)',
            borderRadius: '6px', boxShadow: '0 4px 12px rgba(0,0,0,0.2)', zIndex: 1000,
            minWidth: '180px', overflow: 'hidden'
          }}>
            <div className="dropdown-item" onClick={handleShowAudioSettings} 
                 style={{ padding: '10px 15px', fontSize: '13px', cursor: 'pointer', display: 'flex', alignItems: 'center', gap: '10px' }}>
              <Volume2 size={16} /> Audio Settings
            </div>
          </div>
        )}
      </div>
    </header>
  );
};

export default Toolbar;
