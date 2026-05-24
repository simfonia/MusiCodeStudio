import React, { useRef, useEffect, useState } from 'react';
import { Stage, Layer, Rect, Line, Text, Group } from 'react-konva';
import { EngineService } from '../services/EngineService';

interface Note {
  pitch: number;
  start: number; // in beats
  len: number;   // in beats
}

interface Clip {
  id: string;
  start: number;      // in seconds
  length: number;     // in seconds
  startBeat: number;  // in beats
  lengthBeat: number; // in beats
  type: string;
  notes?: Note[];
}

interface Track {
  id: string | number;
  name: string;
  clips?: Clip[];
}

interface ArrangementViewProps {
  tracks: Track[];
  bpm: number;
  onSelectClip?: (clipID: string) => void;
}

const ArrangementView: React.FC<ArrangementViewProps> = ({ tracks, bpm, onSelectClip }) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const engine = EngineService.getInstance();
  const [size, setSize] = useState({ width: 800, height: 600 });
  const [timelineMode, setTimelineMode] = useState<'beats' | 'seconds'>('beats');
  
  // 佈局常量
  const headerHeight = 30; 
  const rowHeight = 120;
  const rowMargin = 2;
  const pixelsPerSecond = 40; 
  const pixelsPerBeat = 40; // 在 Beats 模式下，一拍佔 40px

  // 顏色常量
  const colors = {
    accent: '#007aff',
    accentSoft: '#e1f0ff',
    textDim: '#86868b',
    border: '#e1e1e3',
    gridMajor: 'rgba(0,0,0,0.1)',
    gridMinor: 'rgba(0,0,0,0.03)',
    rulerBg: '#f5f5f7'
  };

  useEffect(() => {
    const updateSize = () => {
      if (containerRef.current) {
        setSize({
          width: containerRef.current.offsetWidth,
          height: containerRef.current.offsetHeight
        });
      }
    };

    updateSize();
    window.addEventListener('resize', updateSize);
    return () => window.removeEventListener('resize', updateSize);
  }, []);

  // 繪製標尺與網格
  const renderRulerAndGrid = () => {
    const elements = [];
    
    if (timelineMode === 'beats') {
      const beatsVisible = Math.ceil(size.width / pixelsPerBeat) + 16;
      for (let i = 0; i <= beatsVisible; i++) {
        const x = i * pixelsPerBeat;
        const isBar = i % 4 === 0; // 假設 4/4 拍

        elements.push(
          <Line
            key={`v-line-beat-${i}`}
            points={[x, 0, x, size.height]}
            stroke={isBar ? colors.gridMajor : colors.gridMinor}
            strokeWidth={isBar ? 1.5 : 1}
          />
        );

        if (isBar) {
          const barNumber = (i / 4) + 1;
          elements.push(
            <Text
              key={`bar-text-${i}`}
              x={x + 4}
              y={8}
              text={`${barNumber}.1`}
              fontSize={10}
              fill={colors.accent}
              fontStyle="bold"
            />
          );
        } else {
          elements.push(
            <Text
              key={`beat-text-${i}`}
              x={x + 4}
              y={10}
              text={`${Math.floor(i / 4) + 1}.${(i % 4) + 1}`}
              fontSize={9}
              fill={colors.textDim}
            />
          );
        }
      }
    } else {
      // 秒數模式 (略，保持原有邏輯)
      const secondsVisible = Math.ceil(size.width / pixelsPerSecond) + 10;
      for (let i = 0; i <= secondsVisible; i++) {
        const x = i * pixelsPerSecond;
        const isMajor = i % 5 === 0;
        elements.push(
          <Line key={`v-line-sec-${i}`} points={[x, 0, x, size.height]} stroke={isMajor ? colors.gridMajor : colors.gridMinor} strokeWidth={isMajor ? 1.5 : 1} />
        );
        if (isMajor) {
          elements.push(<Text key={`time-text-${i}`} x={x + 4} y={8} text={`${i}s`} fontSize={10} fill={colors.textDim} />);
        }
      }
    }

    elements.unshift(<Rect key="ruler-bg" width={size.width} height={headerHeight} fill={colors.rulerBg} stroke={colors.border} strokeWidth={1} />);
    tracks.forEach((_, index) => {
      const y = index * (rowHeight + rowMargin) + rowHeight + headerHeight;
      elements.push(<Line key={`h-line-${index}`} points={[0, y, size.width, y]} stroke={colors.border} strokeWidth={2} />);
    });
    elements.push(<Line key="ruler-bottom-line" points={[0, headerHeight, size.width, headerHeight]} stroke={colors.border} strokeWidth={1} />);

    return elements;
  };

  const renderClips = () => {
    return tracks.map((track, trackIndex) => {
      const rowY = trackIndex * (rowHeight + rowMargin) + headerHeight;
      
      return (
        <Group key={`track-group-${track.id}`} y={rowY}>
          {track.clips?.map((clip) => {
            // 根據模式計算 X 與 Width
            let clipX, clipWidth;
            if (timelineMode === 'beats') {
              clipX = (clip.startBeat || 0) * pixelsPerBeat;
              clipWidth = (clip.lengthBeat || 0) * pixelsPerBeat;
            } else {
              clipX = clip.start * pixelsPerSecond;
              clipWidth = clip.length * pixelsPerSecond;
            }

            const clipHeight = rowHeight - 20;

            return (
              <Group 
                key={clip.id} 
                x={clipX} 
                onClick={() => onSelectClip?.(clip.id)}
                onMouseEnter={(e) => {
                  const container = e.target.getStage()?.container();
                  if (container) container.style.cursor = 'pointer';
                }}
                onMouseLeave={(e) => {
                  const container = e.target.getStage()?.container();
                  if (container) container.style.cursor = 'default';
                }}
              >
                <Rect
                  width={clipWidth}
                  height={clipHeight}
                  y={10}
                  fill={colors.accentSoft}
                  stroke={colors.accent}
                  strokeWidth={1}
                  cornerRadius={4}
                />

                {clip.type === 'midi' && clip.notes && (
                  <Group y={10} clipFunc={(ctx) => ctx.rect(0, 0, clipWidth, clipHeight)}>
                    {clip.notes.map((note, i) => {
                      const minPitch = 36, maxPitch = 96;
                      const normalizedPitch = Math.max(0, Math.min(1, (note.pitch - minPitch) / (maxPitch - minPitch)));
                      const noteY = clipHeight - (normalizedPitch * clipHeight * 0.7) - (clipHeight * 0.15);
                      
                      return (
                        <Rect
                          key={`note-${i}`}
                          x={note.start * pixelsPerBeat}
                          y={noteY}
                          width={Math.max(3, note.len * pixelsPerBeat)}
                          height={2.5}
                          fill={colors.accent}
                          opacity={0.7}
                        />
                      );
                    })}
                  </Group>
                )}

                <Text x={8} y={18} text={clip.type === 'midi' ? "🎹 MIDI" : "🔊 Audio"} fontSize={11} fill={colors.accent} fontStyle="bold" />
                <Text 
                  x={8} y={32} 
                  text={timelineMode === 'beats' ? `B: ${clip.startBeat?.toFixed(1)}` : `${clip.length.toFixed(2)}s`} 
                  fontSize={10} fill={colors.textDim} 
                />
              </Group>
            );
          })}
        </Group>
      );
    });
  };

  return (
    <div ref={containerRef} className="arrangement-view" style={{ width: '100%', height: '100%', background: '#ffffff', position: 'relative' }}>
      {/* 模式切換按鈕 (漂浮在標尺右側) */}
      <div style={{ position: 'absolute', top: 5, right: 10, zIndex: 100, display: 'flex', gap: '5px' }}>
        <button 
          onClick={() => setTimelineMode('beats')}
          style={{ padding: '2px 8px', fontSize: '10px', borderRadius: '4px', border: '1px solid var(--border)', background: timelineMode === 'beats' ? 'var(--accent)' : 'white', color: timelineMode === 'beats' ? 'white' : 'black' }}
        >
          Beats
        </button>
        <button 
          onClick={() => setTimelineMode('seconds')}
          style={{ padding: '2px 8px', fontSize: '10px', borderRadius: '4px', border: '1px solid var(--border)', background: timelineMode === 'seconds' ? 'var(--accent)' : 'white', color: timelineMode === 'seconds' ? 'white' : 'black' }}
        >
          Secs
        </button>
      </div>

      <Stage width={size.width} height={size.height}>
        <Layer>
          {renderRulerAndGrid()}
          {renderClips()}
        </Layer>
      </Stage>
    </div>
  );
};

export default ArrangementView;
