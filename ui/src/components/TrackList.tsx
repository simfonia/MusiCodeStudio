import React from 'react';
import { Plus } from 'lucide-react';
import TrackHeader from './TrackHeader';
import { EngineService } from '../services/EngineService';

interface TrackListProps {
  tracks: { id: string | number, name: string }[];
}

const TrackList: React.FC<TrackListProps> = ({ tracks }) => {
  const engine = EngineService.getInstance();

  return (
    <aside className="track-list">
      {tracks.map((track) => (
        <TrackHeader 
          key={track.id}
          trackID={track.id.toString()}
          trackName={track.name}
          onShowPlugin={() => engine.showPluginWindow(track.id)}
        />
      ))}
      
      <button style={{ 
        margin: '10px', padding: '10px', border: '1px dashed var(--border)', 
        background: 'none', borderRadius: '6px', cursor: 'pointer', color: 'var(--text-dim)',
        display: 'flex', alignItems: 'center', justifyContent: 'center'
      }}>
        <Plus size={16} style={{ marginRight: '5px' }} />
        Add Track
      </button>
    </aside>
  );
};

export default TrackList;
