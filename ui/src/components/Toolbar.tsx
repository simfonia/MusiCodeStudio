import React, { useState, useRef, useEffect } from 'react';
import { Play, Square, Circle, Settings, Volume2, Bell, Repeat } from 'lucide-react';
import { EngineService } from '../services/EngineService';

interface ToolbarProps {
  bpm: number;
  setBpm: (bpm: number) => void;
  isRecording: boolean;
  setIsRecording: (rec: boolean) => void;
  isMetronomeOn: boolean;
  setIsMetronomeOn: (on: boolean) => void;
  isLoopOn: boolean;
  setIsLoopOn: (on: boolean) => void;
  timeSig: { n: number, d: number };
  setTimeSig: (sig: { n: number, d: number }) => void;
}

const Toolbar: React.FC<ToolbarProps> = ({ 
  bpm, setBpm, 
  isRecording, setIsRecording,
  isMetronomeOn, setIsMetronomeOn,
  isLoopOn, setIsLoopOn,
  timeSig, setTimeSig
}) => {
  const [showSettingsMenu, setShowSettingsMenu] = useState(false);
  const settingsMenuRef = useRef<HTMLDivElement>(null);
  const engine = EngineService.getInstance();
  
  // BPM 局部狀態 (防止輸入時自動補 0)
  const [localBpm, setLocalBpm] = useState(bpm.toString());

  useEffect(() => {
    setLocalBpm(bpm.toString());
  }, [bpm]);

  const handleBpmBlur = () => {
    const val = parseFloat(localBpm);
    if (!isNaN(val) && val >= 20 && val <= 400) {
      setBpm(val);
    } else {
      setLocalBpm(bpm.toString());
    }
  };

  const handleBpmKeyDown = (e: React.KeyboardEvent) => {
    if (e.key === 'Enter') handleBpmBlur();
  };

  const toggleMetronome = () => {
    const next = !isMetronomeOn;
    setIsMetronomeOn(next);
    engine.sendCommand({ action: 'set_click_enabled', enabled: next } as any);
  };

  const toggleLoop = () => {
    const next = !isLoopOn;
    setIsLoopOn(next);
    engine.sendCommand({ action: 'set_loop_enabled', enabled: next } as any);
  };

  const handleTimeSigNChange = (n: number) => {
    const next = { ...timeSig, n };
    setTimeSig(next);
    engine.sendCommand({ action: 'set_time_signature', numerator: n, denominator: timeSig.d } as any);
  };

  const handleTimeSigDChange = (d: number) => {
    const next = { ...timeSig, d };
    setTimeSig(next);
    engine.sendCommand({ action: 'set_time_signature', numerator: timeSig.n, denominator: d } as any);
  };

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

      <div style={{ marginLeft: '20px', display: 'flex', gap: '4px' }}>
        <button 
          className={`transport-btn ${isMetronomeOn ? 'active' : ''}`} 
          onClick={toggleMetronome}
          style={{ color: isMetronomeOn ? 'var(--accent)' : 'inherit' }}
          title="節拍器"
        >
          <Bell size={16} fill={isMetronomeOn ? 'currentColor' : 'none'} />
        </button>
        <button 
          className={`transport-btn ${isLoopOn ? 'active' : ''}`} 
          onClick={toggleLoop}
          style={{ color: isLoopOn ? 'var(--accent)' : 'inherit' }}
          title="循環播放"
        >
          <Repeat size={16} />
        </button>
      </div>

      <div style={{ marginLeft: '40px', display: 'flex', alignItems: 'center', gap: '15px' }}>
        <div style={{ fontSize: '13px', color: 'var(--text-dim)', display: 'flex', alignItems: 'center' }}>
          BPM: <input 
            type="text" 
            value={localBpm} 
            onChange={(e) => setLocalBpm(e.target.value.replace(/[^0-9.]/g, ''))}
            onBlur={handleBpmBlur}
            onKeyDown={handleBpmKeyDown}
            style={{ width: '45px', background: 'none', border: 'none', marginLeft: '5px', fontWeight: 600, fontSize: '14px', color: 'var(--accent)' }}
          />
        </div>
        <div style={{ fontSize: '13px', color: 'var(--text-dim)', display: 'flex', alignItems: 'center' }}>
          SIG: 
          <input 
            type="number" 
            value={timeSig.n} 
            onChange={(e) => handleTimeSigNChange(Number(e.target.value))}
            style={{ width: '25px', background: 'none', border: 'none', marginLeft: '5px', fontWeight: 600, fontSize: '14px', color: 'var(--accent)', textAlign: 'center' }}
          />
          <span style={{ margin: '0 2px' }}>/</span>
          <input 
            type="number" 
            value={timeSig.d} 
            onChange={(e) => handleTimeSigDChange(Number(e.target.value))}
            style={{ width: '25px', background: 'none', border: 'none', fontWeight: 600, fontSize: '14px', color: 'var(--accent)', textAlign: 'center' }}
          />
        </div>
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
