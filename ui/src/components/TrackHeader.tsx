import React, { useState, useEffect, useRef } from 'react';
import { ChevronDown, Zap } from 'lucide-react';
import { EngineService } from '../services/EngineService';

interface TrackHeaderProps {
  trackID: string;
  trackName: string;
  onShowPlugin: () => void;
}

const TrackHeader: React.FC<TrackHeaderProps> = ({ trackID, trackName, onShowPlugin }) => {
  const [selectedInput, setSelectedInput] = useState<string>('No Input');
  const [isMenuOpen, setIsInputMenuOpen] = useState(false);
  const [isArmed, setIsArmed] = useState(false);
  const [midiDevices, setMidiDevices] = useState<{name: string, id: string}[]>([]);
  const [signalLevel, setSignalLevel] = useState(0);
  
  const engine = EngineService.getInstance();
  const menuRef = useRef<HTMLDivElement>(null);

  // 監聽來自 C++ 的事件
  useEffect(() => {
    const handleMidiSignal = (e: any) => {
      const { trackID: id, level } = e.detail;
      if (id === trackID) {
        setSignalLevel(level);
      }
    };

    const handleMidiInputs = (e: any) => {
      const devices = e.detail;
      setMidiDevices([{ name: 'All MIDI Ins', id: '__all_midi_ins__' }, ...devices]);
    };

    window.addEventListener('MusiCode_MidiSignal' as any, handleMidiSignal);
    window.addEventListener('MusiCode_MidiInputsList' as any, handleMidiInputs);

    return () => {
      window.removeEventListener('MusiCode_MidiSignal' as any, handleMidiSignal);
      window.removeEventListener('MusiCode_MidiInputsList' as any, handleMidiInputs);
    };
  }, [trackID]);

  // 點擊外部關閉選單
  useEffect(() => {
    const handleClickOutside = (event: MouseEvent) => {
      if (menuRef.current && !menuRef.current.contains(event.target as Node)) {
        setIsInputMenuOpen(false);
      }
    };
    document.addEventListener('mousedown', handleClickOutside);
    return () => document.removeEventListener('mousedown', handleClickOutside);
  }, []);

  const toggleInputMenu = () => {
    if (!isMenuOpen) {
      engine.getMidiInputs(); // 請求最新列表
    }
    setIsInputMenuOpen(!isMenuOpen);
  };

  const handleSelectDevice = (deviceName: string) => {
    setSelectedInput(deviceName);
    engine.setTrackInput(trackID, deviceName);
    setIsInputMenuOpen(false);
    if (deviceName !== 'No Input') setIsArmed(true);
  };

  const toggleArm = () => {
    const newArmedState = !isArmed;
    setIsArmed(newArmedState);
    if (newArmedState) {
      // 預設武裝到 All MIDI Ins
      setSelectedInput('All MIDI Ins');
      engine.setTrackInput(trackID, 'All MIDI Ins');
    } else {
      setSelectedInput('No Input');
      engine.setTrackInput(trackID, 'No Input');
    }
  };

  return (
    <div className="track-header" style={{ position: 'relative' }}>
      <div style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '8px' }}>
        <span style={{ fontWeight: 600, fontSize: '14px' }}>{trackName}</span>
        <div style={{ display: 'flex', gap: '4px' }}>
          <button className="view-btn" style={{ padding: '2px 6px', fontSize: '10px' }}>M</button>
          <button className="view-btn" style={{ padding: '2px 6px', fontSize: '10px' }}>S</button>
          <button 
            className={`view-btn ${isArmed ? 'active' : ''}`} 
            style={{ 
              padding: '2px 6px', 
              fontSize: '10px', 
              color: isArmed ? 'white' : 'var(--danger)',
              backgroundColor: isArmed ? 'var(--danger)' : 'transparent'
            }}
            onClick={toggleArm}
          >
            R
          </button>
        </div>
      </div>

      {/* Waveform-style Input Selector & Meter */}
      <div style={{ display: 'flex', gap: '6px', marginBottom: '8px', alignItems: 'stretch' }}>
        {/* Signal Meter (仿 Waveform) */}
        <div style={{ 
          width: '6px', 
          background: 'var(--border)', 
          borderRadius: '3px', 
          position: 'relative',
          overflow: 'hidden'
        }}>
          <div style={{ 
            position: 'absolute', 
            bottom: 0, 
            left: 0, 
            width: '100%', 
            height: `${signalLevel * 100}%`,
            background: 'var(--note-f)',
            transition: 'height 0.05s ease-out',
            boxShadow: signalLevel > 0.1 ? '0 0 5px var(--note-f)' : 'none'
          }}></div>
        </div>

        {/* Input Dropdown */}
        <div style={{ flex: 1, position: 'relative' }} ref={menuRef}>
          <div 
            onClick={toggleInputMenu}
            style={{ 
              background: 'var(--bg-app)', 
              padding: '4px 8px', 
              borderRadius: '4px', 
              fontSize: '11px', 
              color: 'var(--text-main)', 
              cursor: 'pointer',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'space-between',
              border: '1px solid var(--border)'
            }}
          >
            <div style={{ overflow: 'hidden', textOverflow: 'ellipsis', whiteSpace: 'nowrap' }}>
              <Zap size={10} style={{ marginRight: '4px', display: 'inline' }} />
              {selectedInput}
            </div>
            <ChevronDown size={12} />
          </div>

          {isMenuOpen && (
            <div style={{
              position: 'absolute',
              top: '100%',
              left: 0,
              width: '100%',
              marginTop: '4px',
              backgroundColor: 'var(--bg-paper)',
              border: '1px solid var(--border)',
              borderRadius: '4px',
              boxShadow: '0 4px 8px rgba(0,0,0,0.1)',
              zIndex: 100,
              maxHeight: '150px',
              overflowY: 'auto'
            }}>
              {midiDevices.map((dev) => (
                <div 
                  key={dev.id}
                  onClick={() => handleSelectDevice(dev.name)}
                  style={{ 
                    padding: '6px 10px', 
                    fontSize: '11px', 
                    cursor: 'pointer' 
                  }}
                  className="dropdown-item"
                >
                  {dev.name}
                </div>
              ))}
              {midiDevices.length === 0 && (
                <div style={{ padding: '6px 10px', fontSize: '11px', color: 'var(--text-dim)' }}>
                  No MIDI Devices found
                </div>
              )}
            </div>
          )}
        </div>
      </div>
      
      {/* Plugin Indicator */}
      <div 
        style={{ 
          background: 'var(--bg-app)', 
          padding: '6px', 
          borderRadius: '4px', 
          fontSize: '12px', 
          color: 'var(--text-dim)', 
          cursor: 'pointer', 
          border: '1px solid transparent',
          display: 'flex',
          alignItems: 'center',
          gap: '6px'
        }}
        onClick={onShowPlugin}
        onMouseEnter={(e) => e.currentTarget.style.borderColor = 'var(--accent)'}
        onMouseLeave={(e) => e.currentTarget.style.borderColor = 'transparent'}
      >
        <span>🎹</span>
        <span style={{ fontWeight: 500 }}>4OSC</span>
      </div>
    </div>
  );
};

export default TrackHeader;
