import React, { useEffect, useRef, useState } from 'react';
import { OpenSheetMusicDisplay } from 'opensheetmusicdisplay';

interface StaffViewProps {
  scoreXml?: string; // 傳入 MusicXML 字串
}

const StaffView: React.FC<StaffViewProps> = ({ scoreXml }) => {
  const containerRef = useRef<HTMLDivElement>(null);
  const osmdRef = useRef<OpenSheetMusicDisplay | null>(null);
  const [error, setError] = useState<string | null>(null);

  useEffect(() => {
    if (containerRef.current && !osmdRef.current) {
      // 初始化 OSMD
      osmdRef.current = new OpenSheetMusicDisplay(containerRef.current, {
        autoResize: true,
        drawTitle: false,
        drawSubtitle: false,
        drawComposer: false,
        drawMetronomeMarks: true,
        coloringEnabled: true,
        defaultColorNotehead: '#1d1d1f', // 使用 MusiCodeStudio 主色
      });
    }

    const renderScore = async () => {
      if (!osmdRef.current) return;

      try {
        // 如果沒有傳入 scoreXml，我們使用一個極簡的範例 (C4 Quarter Note)
        const xmlToLoad = scoreXml || `<?xml version="1.0" encoding="UTF-8" standalone="no"?>
          <!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.1 Partwise//EN" "http://www.musicxml.org/dtds/partwise.dtd">
          <score-partwise version="3.1">
            <part-list>
              <score-part id="P1"><part-name>Piano</part-name></score-part>
            </part-list>
            <part id="P1">
              <measure number="1">
                <attributes>
                  <divisions>1</divisions>
                  <key><fifths>0</fifths></key>
                  <clef><sign>G</sign><line>2</line></clef>
                </attributes>
                <note>
                  <pitch><step>C</step><octave>4</octave></pitch>
                  <duration>1</duration>
                  <type>quarter</type>
                </note>
                <note>
                  <pitch><step>D</step><octave>4</octave></pitch>
                  <duration>1</duration>
                  <type>quarter</type>
                </note>
                <note>
                  <pitch><step>E</step><octave>4</octave></pitch>
                  <duration>1</duration>
                  <type>quarter</type>
                </note>
                <note>
                  <pitch><step>G</step><octave>4</octave></pitch>
                  <duration>1</duration>
                  <type>quarter</type>
                </note>
              </measure>
            </part>
          </score-partwise>`;

        await osmdRef.current.load(xmlToLoad);
        osmdRef.current.render();
      } catch (e) {
        console.error('OSMD Error:', e);
        setError('樂譜渲染失敗');
      }
    };

    renderScore();
  }, [scoreXml]);

  return (
    <div style={{ width: '100%', height: '100%', overflow: 'auto', background: 'var(--bg-paper)' }}>
      {error && <div style={{ color: 'var(--danger)', padding: '20px' }}>{error}</div>}
      <div ref={containerRef} style={{ width: '100%' }} />
    </div>
  );
};

export default StaffView;
