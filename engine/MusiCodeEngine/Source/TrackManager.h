#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>

namespace MusiCode
{
    /**
     * TrackManager - 負責音軌的 ID 定位與元數據管理。
     * 解決 trackIndex 不穩定的問題，改用 te::EditItemID。
     */
    class TrackManager
    {
    public:
        /** 將字串形式的 ID 轉換為 te::EditItemID */
        static tracktion_engine::EditItemID stringToID(const juce::String& idString)
        {
            return tracktion_engine::EditItemID::fromString(idString);
        }

        /** 根據 ID 尋找音軌 */
        static tracktion_engine::AudioTrack* findAudioTrackByID(tracktion_engine::Edit& edit, 
                                                              tracktion_engine::EditItemID id)
        {
            for (auto track : tracktion_engine::getAudioTracks(edit))
            {
                if (track->itemID == id)
                    return track;
            }
            return nullptr;
        }

        /** 根據索引獲取 ID (向後相容用) */
        static tracktion_engine::EditItemID getIDFromIndex(tracktion_engine::Edit& edit, int index)
        {
            auto tracks = tracktion_engine::getAudioTracks(edit);
            if (index >= 0 && index < tracks.size())
                return tracks[index]->itemID;
            
            return {};
        }

        /** 獲取所有音軌資訊供前端使用 (包含 Clip 元數據) */
        static juce::var getTracksInfo(tracktion_engine::Edit& edit)
        {
            juce::Array<juce::var> trackList;
            for (auto track : tracktion_engine::getAudioTracks(edit))
            {
                juce::DynamicObject::Ptr obj = new juce::DynamicObject();
                obj->setProperty("id", track->itemID.toString());
                obj->setProperty("name", track->getName());
                obj->setProperty("index", track->getIndexInEditTrackList());
                
                // --- 提取 Clip 資訊 ---
                juce::Array<juce::var> clips;
                for (auto clip : track->getClips())
                {
                    juce::DynamicObject::Ptr clipObj = new juce::DynamicObject();
                    clipObj->setProperty("id", clip->itemID.toString());
                    clipObj->setProperty("start", clip->getPosition().getStart().inSeconds());
                    clipObj->setProperty("length", clip->getPosition().getLength().inSeconds());
                    
                    // --- [新增] 拍點元數據 ---
                    auto& tempoSeq = edit.tempoSequence;
                    double startBeat = tempoSeq.toBeats(clip->getPosition().getStart()).inBeats();
                    double endBeat = tempoSeq.toBeats(clip->getPosition().getEnd()).inBeats();
                    clipObj->setProperty("startBeat", startBeat);
                    clipObj->setProperty("lengthBeat", endBeat - startBeat);
                    
                    bool isMidi = (dynamic_cast<tracktion_engine::MidiClip*>(clip) != nullptr);
                    clipObj->setProperty("type", isMidi ? "midi" : "audio");

                    // --- [恢復] 提取音符預覽 (用於 ArrangementView 繪製) ---
                    if (isMidi)
                    {
                        if (auto midiClip = dynamic_cast<tracktion_engine::MidiClip*>(clip))
                        {
                            juce::Array<juce::var> previewNotes;
                            auto& sequence = midiClip->getSequence();
                            int count = 0;
                            for (auto* note : sequence.getNotes())
                            {
                                if (count++ > 50) break; // 限制預覽數量，防止 JSON 爆炸
                                
                                juce::DynamicObject::Ptr nObj = new juce::DynamicObject();
                                nObj->setProperty("pitch", note->getNoteNumber());
                                nObj->setProperty("start", note->getStartBeat().inBeats());
                                nObj->setProperty("length", note->getLengthBeats().inBeats());
                                previewNotes.add(nObj.get());
                            }
                            clipObj->setProperty("notes", previewNotes);
                        }
                    }
                    
                    clips.add(clipObj.get());
                }
                obj->setProperty("clips", clips);
                
                trackList.add(obj.get());
            }
            return trackList;
        }
    };
}
