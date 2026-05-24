import React, { useRef, useEffect, useState } from 'react';

interface Note {
  pitch: number;
  start: number;
  len: number;
}

interface PianoRollViewProps {
  notes?: Note[];
}

const PianoRollView: React.FC<PianoRollViewProps> = ({ notes = [] }) => {
  const canvasRef = useRef<HTMLCanvasElement>(null);
  
  // 鋼琴捲軸配置
  const keyHeight = 20;
  const pixelsPerBeat = 80;
  const totalKeys = 128;
  const visibleBeats = 16;

  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // 清除畫布
    ctx.clearRect(0, 0, canvas.width, canvas.height);

    // 1. 繪製背景與琴鍵網格
    for (let i = 0; i < totalKeys; i++) {
      const y = i * keyHeight;
      const pitch = 127 - i;
      const isBlackKey = [1, 3, 6, 8, 10].includes(pitch % 12);

      ctx.fillStyle = isBlackKey ? '#f5f5f7' : '#ffffff';
      ctx.fillRect(0, y, canvas.width, keyHeight);
      
      ctx.strokeStyle = '#e1e1e3';
      ctx.lineWidth = 0.5;
      ctx.strokeRect(0, y, canvas.width, keyHeight);

      // 繪製音高標記 (每組 C)
      if (pitch % 12 === 0) {
        ctx.fillStyle = '#86868b';
        ctx.font = '10px Arial';
        ctx.fillText(`C${pitch / 12 - 1}`, 5, y + 14);
      }
    }

    // 2. 繪製垂直拍線
    for (let b = 0; b <= visibleBeats; b++) {
      const x = b * pixelsPerBeat;
      ctx.strokeStyle = b % 4 === 0 ? 'rgba(0,122,255,0.2)' : 'rgba(0,0,0,0.05)';
      ctx.lineWidth = b % 4 === 0 ? 1.5 : 1;
      ctx.beginPath();
      ctx.moveTo(x, 0);
      ctx.lineTo(x, canvas.height);
      ctx.stroke();
    }

    // 3. 繪製音符 (由 Arrangement 傳入的 selectedClipNotes)
    console.log('[PianoRollView] Drawing notes:', notes);
    notes.forEach(note => {
      const noteLength = (note as any).length || note.len || 0.25; // 修正命名不匹配
      const x = note.start * pixelsPerBeat;
      const y = (127 - note.pitch) * keyHeight;
      const width = noteLength * pixelsPerBeat;

      // 繪製音符主體
      ctx.fillStyle = '#007aff';
      ctx.globalAlpha = 0.8;
      ctx.fillRect(x + 2, y + 2, width - 4, keyHeight - 4);
      
      // 繪製音符高光
      ctx.fillStyle = 'rgba(255,255,255,0.3)';
      ctx.fillRect(x + 2, y + 2, width - 4, 2);
      
      ctx.globalAlpha = 1.0;
      ctx.strokeStyle = '#005bb5';
      ctx.lineWidth = 1;
      ctx.strokeRect(x + 2, y + 2, width - 4, keyHeight - 4);
    });

  }, [notes]);

  return (
    <div className="piano-roll-container" style={{ width: '100%', height: '100%', overflow: 'auto', background: '#fff' }}>
      <canvas 
        ref={canvasRef} 
        width={visibleBeats * pixelsPerBeat} 
        height={totalKeys * keyHeight}
        style={{ display: 'block' }}
      />
    </div>
  );
};

export default PianoRollView;
