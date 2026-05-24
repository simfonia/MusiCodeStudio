import React, { useEffect, useRef } from 'react';
import * as Blockly from 'blockly';
import { EngineService } from '../services/EngineService';

const BlocklyView: React.FC = () => {
  const blocklyDiv = useRef<HTMLDivElement>(null);
  const workspace = useRef<Blockly.WorkspaceSvg | null>(null);

  useEffect(() => {
    if (blocklyDiv.current && !workspace.current) {
      // 1. 定義自定義積木
      Blockly.Blocks['plugin_set_param'] = {
        init: function() {
          this.appendDummyInput()
              .appendField("設定插件")
              .appendField(new Blockly.FieldTextInput("4OSC"), "PLUGIN")
              .appendField("參數")
              .appendField(new Blockly.FieldDropdown([
                ["濾波器截止頻率", "filterFreq"],
                ["濾波器共鳴", "filterResonance"],
                ["主音量", "masterVolume"]
              ]), "PARAM")
              .appendField("數值")
              .appendField(new Blockly.FieldNumber(0.5, 0, 1.0, 0.01), "VALUE");
          this.setPreviousStatement(true, null);
          this.setNextStatement(true, null);
          this.setColour('#ff9500');
          this.setTooltip("調整指定插件的參數值 (0.0 ~ 1.0)");
        }
      };

      // 2. 注入 Blockly
      workspace.current = Blockly.inject(blocklyDiv.current, {
        toolbox: {
          kind: 'categoryToolbox',
          contents: [
            {
              kind: 'category',
              name: '合成器控制',
              colour: '#ff9500',
              contents: [
                { kind: 'block', type: 'plugin_set_param' }
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

      // 3. 掛載即時連動監聽器
      workspace.current.addChangeListener((event: any) => {
        // 捕捉數值變更事件 (Blockly.Events.BLOCK_CHANGE)
        if (event.type === Blockly.Events.BLOCK_CHANGE) {
          const block = workspace.current?.getBlockById(event.blockId || '');
          if (block && block.type === 'plugin_set_param') {
            const pluginName = block.getFieldValue('PLUGIN');
            const paramID = block.getFieldValue('PARAM');
            const value = parseFloat(block.getFieldValue('VALUE'));
            
            // 透過 EngineService 發送即時更新
            EngineService.getInstance().setPluginParameter(pluginName, paramID, value);
          }
        }
      });
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
