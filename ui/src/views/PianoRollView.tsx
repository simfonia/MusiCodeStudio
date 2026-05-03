import React, { useRef, useEffect, useState } from 'react';
import { Stage, Layer, Rect, Line, Text } from 'react-konva';

const PianoRollView: React.FC = () => {
  const containerRef = useRef<HTMLDivElement>(null);
  const [size, setSize] = useState({ width: 800, height: 600 });

  // 音符高度與寬度單位
  const noteHeight = 24;
  const beatWidth = 160; // 每一拍的寬度
  const stepWidth = beatWidth / 4; // 十六分音符寬度

  useEffect(() => {
    if (containerRef.current) {
      setSize({
        width: containerRef.current.offsetWidth,
        height: containerRef.current.offsetHeight
      });
    }
  }, []);

  // 繪製背景網格
  const renderGrid = () => {
    const lines = [];
    const numSteps = Math.ceil(size.width / stepWidth);
    const numNotes = Math.ceil(size.height / noteHeight);

    // 垂直線 (時間軸)
    for (let i = 0; i <= numSteps; i++) {
      const isBeat = i % 4 === 0;
      const isMeasure = i % 16 === 0;
      lines.push(
        <Line
          key={`v-${i}`}
          points={[i * stepWidth, 0, i * stepWidth, size.height]}
          stroke={isMeasure ? '#c1c1c3' : isBeat ? '#e1e1e3' : '#f0f0f0'}
          strokeWidth={isMeasure ? 2 : 1}
        />
      );
    }

    // 水平線 (音高軸)
    for (let j = 0; j <= numNotes; j++) {
      lines.push(
        <Line
          key={`h-${j}`}
          points={[0, j * noteHeight, size.width, j * noteHeight]}
          stroke="#f0f0f0"
          strokeWidth={1}
        />
      );
    }

    return lines;
  };

  return (
    <div ref={containerRef} style={{ width: '100%', height: '100%', background: 'white' }}>
      <Stage width={size.width} height={size.height}>
        <Layer>
          {/* 背景網格 */}
          {renderGrid()}
          
          {/* 範例音符 (C4 顏色) */}
          <Rect
            x={beatWidth * 1}
            y={noteHeight * 10}
            width={beatWidth * 0.5}
            height={noteHeight - 2}
            fill="#ff2d55" // C4 Color
            cornerRadius={4}
            shadowBlur={5}
            shadowOpacity={0.1}
          />
          <Text
            x={beatWidth * 1 + 5}
            y={noteHeight * 10 + 6}
            text="C4"
            fontSize={10}
            fill="white"
          />

          <Rect
            x={beatWidth * 1.5}
            y={noteHeight * 8}
            width={beatWidth * 1}
            height={noteHeight - 2}
            fill="#ff9500" // D4 Color
            cornerRadius={4}
          />
        </Layer>
      </Stage>
    </div>
  );
};

export default PianoRollView;
