import React, { useRef, useEffect, useState } from 'react';
import { Stage, Layer, Rect, Line, Text, Group } from 'react-konva';
import { EngineService } from '../services/EngineService';

interface Note { pitch: number; start: number; length: number; }
interface Clip { id: string; start: number; length: number; startBeat: number; lengthBeat: number; type: string; notes?: Note[]; }
interface Track { id: string | number; name: string; clips?: Clip[]; }

interface ArrangementViewProps {
  tracks: Track[];
  bpm: number;
  timeSigNumerator?: number;
  timeSigDenominator?: number;
  onSelectClip?: (clipID: string) => void;
}

const ArrangementView: React.FC<ArrangementViewProps> = ({ 
  tracks, bpm, timeSigNumerator = 4, timeSigDenominator = 4, onSelectClip 
}) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const stageRef = useRef<any>(null);
  const [size, setSize] = useState({ width: 800, height: 600 });
  const [timelineMode, setTimelineMode] = useState<'beats' | 'seconds'>('beats');
  
  const [playheadPos, setPlayheadPos] = useState({ seconds: 0, beats: 0 });
  const [loopRange, setLoopRange] = useState({ start: 0, end: 4 }); 
  const [scrollX, setScrollX] = useState(0);
  
  const headerHeight = 30; 
  const rowHeight = 120;
  const rowMargin = 2;
  const pixelsPerSecond = 40; 
  const pixelsPerBeat = 40; 

  const colors = {
    accent: '#007aff', accentSoft: '#e1f0ff', textDim: '#86868b', border: '#e1e1e3',
    gridMajor: 'rgba(0,0,0,0.1)', gridMinor: 'rgba(0,0,0,0.03)',
    rulerBg: '#f5f5f7', playhead: '#ff3b30', loopBg: 'rgba(0, 122, 255, 0.1)', ioMarker: '#5856d6'
  };

  // --- [核心修正] 移除視覺單位與引擎拍點的雙重縮放 ---
  const getPixelsPerUnit = () => timelineMode === 'beats' ? pixelsPerBeat : pixelsPerSecond;
  
  // 1 UI 單位對應的秒數。UserBPM 是以「拍」為單位的速度 (不論分母是 4 或 8)
  const getSecondsPerUnit = () => (timelineMode === 'seconds') ? 1 : (60 / bpm);

  // 經診斷，Tracktion Engine 的 inBeats() 已經根據拍號分母進行了縮放。
  // 因此 UI 不再需要額外的 getVisualScale。1 引擎拍 = 1 UI 網格單位。

  useEffect(() => {
    const handlePlayheadSync = (e: any) => {
      setPlayheadPos({ seconds: e.detail.seconds || 0, beats: e.detail.beats || 0 });
    };
    window.addEventListener('MusiCode_PlayheadSync' as any, handlePlayheadSync);
    return () => window.removeEventListener('MusiCode_PlayheadSync' as any, handlePlayheadSync);
  }, []);

  useEffect(() => {
    const updateSize = () => {
      if (containerRef.current) {
        setSize({ width: containerRef.current.offsetWidth, height: containerRef.current.offsetHeight });
      }
    };
    updateSize();
    window.addEventListener('resize', updateSize);
    return () => window.removeEventListener('resize', updateSize);
  }, []);

  // 初始同步
  useEffect(() => {
    syncLoopToBackend(loopRange);
  }, []);

  // 自動捲動
  useEffect(() => {
    const ppu = getPixelsPerUnit();
    // 直接使用 beats，不再乘以分母因子
    const currentX = (timelineMode === 'beats' ? playheadPos.beats : playheadPos.seconds) * ppu;
    
    if (currentX > scrollX + size.width * 0.8) {
      setScrollX(currentX - size.width * 0.2);
    } else if (currentX < scrollX) {
      setScrollX(Math.max(0, currentX - 10));
    }
  }, [playheadPos, size.width, timelineMode]);

  const handleSeek = (e: any) => {
    const x = e.evt.offsetX + scrollX;
    const val = x / getPixelsPerUnit(); 
    const engine = EngineService.getInstance();
    engine.sendCommand({ action: 'set_position', seconds: val * getSecondsPerUnit() } as any);
  };

  const syncLoopToBackend = (range: { start: number, end: number }) => {
    const engine = EngineService.getInstance();
    const spu = getSecondsPerUnit();
    engine.sendCommand({ action: 'set_loop_range', start: range.start * spu, end: range.end * spu } as any);
  };

  // 當 BPM 或拍號分母改變時，重新同步 Loop 範圍到後端 (因為拍子與秒數的換算率變了)
  useEffect(() => {
    syncLoopToBackend(loopRange);
  }, [bpm, timeSigDenominator]);

  const renderRulerAndGrid = () => {
    const elements = [];
    const ppu = getPixelsPerUnit();
    const startIdx = Math.floor(scrollX / ppu);
    const visibleCount = Math.ceil(size.width / ppu) + 4;

    for (let i = startIdx; i <= startIdx + visibleCount; i++) {
      const x = i * ppu;
      const isBar = i % timeSigNumerator === 0;
      elements.push(<Line key={`v-line-${i}`} points={[x, 0, x, size.height]} stroke={isBar ? colors.gridMajor : colors.gridMinor} strokeWidth={isBar ? 1.5 : 1} listening={false} />);
      if (timelineMode === 'beats' && isBar) {
        elements.push(<Text key={`bar-text-${i}`} x={x + 4} y={8} text={`${Math.floor(i / timeSigNumerator) + 1}.1`} fontSize={10} fill={colors.accent} fontStyle="bold" listening={false} />);
      } else if (timelineMode === 'seconds' && i % 5 === 0) {
        elements.push(<Text key={`time-text-${i}`} x={x + 4} y={8} text={`${i}s`} fontSize={10} fill={colors.textDim} listening={false} />);
      }
    }
    elements.push(<Rect key="loop-highlight" x={loopRange.start * ppu} y={headerHeight} width={(loopRange.end - loopRange.start) * ppu} height={size.height} fill={colors.loopBg} listening={false} />);
    return elements;
  };

  const renderIOMarkers = () => {
    const ppu = getPixelsPerUnit();
    return (
      <Group>
        <Group x={loopRange.start * ppu} draggable dragBoundFunc={(pos) => ({ x: Math.max(0, pos.x), y: 0 })}
          onDragMove={(e) => setLoopRange(prev => ({ ...prev, start: e.target.x() / ppu }))}
          onDragEnd={(e) => {
            const newRange = { ...loopRange, start: e.target.x() / ppu };
            if (newRange.start > newRange.end) [newRange.start, newRange.end] = [newRange.end, newRange.start];
            setLoopRange(newRange);
            syncLoopToBackend(newRange);
          }}
        >
          <Rect width={18} height={20} fill={colors.ioMarker} cornerRadius={2} />
          <Text text="I" x={6} y={4} fill="white" fontSize={12} fontStyle="bold" />
        </Group>

        <Group x={loopRange.end * ppu} draggable dragBoundFunc={(pos) => ({ x: Math.max(0, pos.x), y: 0 })}
          onDragMove={(e) => setLoopRange(prev => ({ ...prev, end: e.target.x() / ppu }))}
          onDragEnd={(e) => {
            const newRange = { ...loopRange, end: e.target.x() / ppu };
            if (newRange.end < newRange.start) [newRange.start, newRange.end] = [newRange.end, newRange.start];
            setLoopRange(newRange);
            syncLoopToBackend(newRange);
          }}
        >
          <Rect width={18} height={20} fill={colors.ioMarker} cornerRadius={2} />
          <Text text="O" x={5} y={4} fill="white" fontSize={12} fontStyle="bold" />
        </Group>
      </Group>
    );
  };

  return (
    <div ref={containerRef} className="arrangement-view" style={{ width: '100%', height: '100%', background: '#ffffff', position: 'relative', overflow: 'hidden' }}>
      <div style={{ position: 'absolute', top: 5, right: 10, zIndex: 100, display: 'flex', gap: '5px' }}>
        <button onClick={() => setTimelineMode('beats')} style={{ padding: '2px 8px', fontSize: '10px', borderRadius: '4px', background: timelineMode === 'beats' ? 'var(--accent)' : 'white', color: timelineMode === 'beats' ? 'white' : 'black', border: '1px solid var(--border)' }}>Beats</button>
        <button onClick={() => setTimelineMode('seconds')} style={{ padding: '2px 8px', fontSize: '10px', borderRadius: '4px', background: timelineMode === 'seconds' ? 'var(--accent)' : 'white', color: timelineMode === 'seconds' ? 'white' : 'black', border: '1px solid var(--border)' }}>Secs</button>
      </div>

      <Stage width={size.width} height={size.height}>
        <Layer>
           <Rect width={size.width} height={headerHeight} fill={colors.rulerBg} stroke={colors.border} strokeWidth={1} onClick={handleSeek} />
        </Layer>

        <Layer x={-scrollX}>
          {renderRulerAndGrid()}
          {renderIOMarkers()}
          {tracks.map((track, trackIndex) => {
            const rowY = trackIndex * (rowHeight + rowMargin) + headerHeight;
            return (
              <Group key={`track-group-${track.id}`} y={rowY}>
                {track.clips?.map((clip) => {
                  const ppu = getPixelsPerUnit();
                  const clipX = (timelineMode === 'beats' ? (clip.startBeat || 0) * ppu : clip.start * ppu);
                  const clipWidth = (timelineMode === 'beats' ? (clip.lengthBeat || 0) * ppu : clip.length * ppu);

                  const handleDeleteClip = (e: any) => {
                    e.cancelBubble = true; // 防止觸發 Clip 選擇事件
                    if (window.confirm("確定要刪除此片段嗎？")) {
                      EngineService.getInstance().sendCommand({ action: 'delete_clip', clipID: clip.id } as any);
                    }
                  };

                  return (
                    <Group key={clip.id} x={clipX} onClick={() => onSelectClip?.(clip.id)}>
                      <Rect width={clipWidth} height={rowHeight - 20} y={10} fill={colors.accentSoft} stroke={colors.accent} strokeWidth={1} cornerRadius={4} />
                      
                      {/* 刪除按鈕 (右上角) */}
                      <Group x={clipWidth - 22} y={14} onClick={handleDeleteClip} cursor="pointer">
                        <Rect width={18} height={18} fill="#ff3b30" cornerRadius={3} />
                        <Text text="×" x={4} y={0} fill="white" fontSize={16} fontStyle="bold" />
                      </Group>
                      {clip.type === 'midi' && clip.notes && (
                        <Group y={10} clipFunc={(ctx) => ctx.rect(0, 0, clipWidth, rowHeight - 20)}>
                          {clip.notes.map((note, i) => {
                            const noteY = (rowHeight - 20) - (Math.max(0, Math.min(1, (note.pitch - 36) / 60)) * (rowHeight - 20) * 0.7) - ((rowHeight - 20) * 0.15);
                            return <Rect key={`note-${i}`} x={note.start * ppu} y={noteY} width={Math.max(3, (note.length || 0.25) * ppu)} height={2.5} fill={colors.accent} opacity={0.7} />;
                          })}
                        </Group>
                      )}
                      <Text x={8} y={18} text={clip.type === 'midi' ? "🎹 MIDI" : "🔊 Audio"} fontSize={11} fill={colors.accent} fontStyle="bold" />
                    </Group>
                  );
                })}
              </Group>
            );
          })}
          
          <Group x={(timelineMode === 'beats' ? playheadPos.beats : playheadPos.seconds) * getPixelsPerUnit()} listening={false}>
            <Line points={[0, 0, 0, size.height]} stroke={colors.playhead} strokeWidth={1.5} />
            <Line points={[-5, 0, 5, 0, 0, 8]} fill={colors.playhead} closed={true} />
          </Group>
        </Layer>
      </Stage>
    </div>
  );
};

export default ArrangementView;
