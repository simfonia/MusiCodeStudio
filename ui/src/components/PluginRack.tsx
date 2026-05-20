import React from 'react';
import { EngineService } from '../services/EngineService';

interface PluginRackProps {
  filterFreq: number;
  onFilterChange: (val: number) => void;
}

const PluginRack: React.FC<PluginRackProps> = ({ filterFreq, onFilterChange }) => {
  return (
    <aside className="plugin-rack">
      <div style={{ fontSize: '11px', fontWeight: 700, color: 'var(--text-dim)', marginBottom: '5px' }}>PLUGINS</div>
      <div className="plugin-slot instrument">🎹 4OSC</div>
      
      <div style={{ padding: '10px', background: 'var(--bg-app)', borderRadius: '6px', marginTop: '10px' }}>
        <div style={{ fontSize: '10px', color: 'var(--text-dim)', marginBottom: '5px' }}>FILTER CUTOFF ({Math.round(filterFreq * 100)}%)</div>
        <input 
          type="range" 
          min="0" 
          max="1" 
          step="0.01" 
          value={filterFreq}
          style={{ width: '100%', cursor: 'pointer' }}
          onInput={(e) => onFilterChange(parseFloat((e.target as HTMLInputElement).value))}
          onDragStart={(e) => e.preventDefault()}
        />
      </div>

      <div className="plugin-slot effect">✨ Reverb</div>
      <div className="plugin-slot empty">+ Add FX</div>
      
      <div style={{ marginTop: 'auto', borderTop: '1px solid var(--border)', paddingTop: '10px' }}>
        <div style={{ fontSize: '11px', fontWeight: 700, color: 'var(--text-dim)' }}>MASTER</div>
        <div style={{ height: '100px', background: 'var(--bg-paper)', border: '1px solid var(--border)', borderRadius: '4px', marginTop: '5px', position: 'relative', overflow: 'hidden' }}>
          <div style={{ position: 'absolute', bottom: 0, left: 0, width: '100%', height: '60%', background: 'linear-gradient(to top, #4cd964, #ffcc00)' }}></div>
        </div>
      </div>
    </aside>
  );
};

export default PluginRack;
