import React, { useEffect, useRef } from 'react';
import * as Blockly from 'blockly';

const BlocklyView: React.FC = () => {
  const blocklyDiv = useRef<HTMLDivElement>(null);
  const workspace = useRef<Blockly.WorkspaceSvg | null>(null);

  useEffect(() => {
    if (blocklyDiv.current && !workspace.current) {
      workspace.current = Blockly.inject(blocklyDiv.current, {
        toolbox: {
          kind: 'categoryToolbox',
          contents: [
            {
              kind: 'category',
              name: '音樂控制',
              colour: '#007aff',
              contents: [
                { kind: 'block', type: 'music_play_note' }
              ]
            },
            {
              kind: 'category',
              name: '邏輯',
              colour: '#5856d6',
              contents: [
                { kind: 'block', type: 'controls_if' },
                { kind: 'block', type: 'logic_compare' }
              ]
            }
          ]
        },
        grid: { spacing: 20, length: 3, colour: '#f0f0f0', snap: true },
        zoom: { controls: true, wheel: true, startScale: 1.0, maxScale: 3, minScale: 0.3, scaleSpeed: 1.2 },
        trashcan: true
      });

      // 定義一個簡單的音樂積木作為原型
      Blockly.Blocks['music_play_note'] = {
        init: function() {
          this.appendValueInput("NOTE")
              .setCheck("String")
              .appendField("播放音符");
          this.appendValueInput("DURATION")
              .setCheck("Number")
              .appendField("時值");
          this.setPreviousStatement(true, null);
          this.setNextStatement(true, null);
          this.setColour('#007aff');
          this.setTooltip("播放一個特定音高與長度的音符");
        }
      };
    }

    return () => {
      if (workspace.current) {
        workspace.current.dispose();
        workspace.current = null;
      }
    };
  }, []);

  return (
    <div 
      ref={blocklyDiv} 
      style={{ width: '100%', height: '100%', textAlign: 'left' }} 
    />
  );
};

export default BlocklyView;
