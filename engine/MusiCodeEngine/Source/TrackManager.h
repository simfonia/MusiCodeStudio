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

        /** 獲取所有音軌資訊供前端使用 */
        static juce::var getTracksInfo(tracktion_engine::Edit& edit)
        {
            juce::Array<juce::var> trackList;
            for (auto track : tracktion_engine::getAudioTracks(edit))
            {
                juce::DynamicObject::Ptr obj = new juce::DynamicObject();
                obj->setProperty("id", track->itemID.toString());
                obj->setProperty("name", track->getName());
                obj->setProperty("index", track->getIndexInEditTrackList());
                trackList.add(obj.get());
            }
            return trackList;
        }
    };
}
