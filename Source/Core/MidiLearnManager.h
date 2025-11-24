#pragma once

#include <JuceHeader.h>

class StemEngine;

struct MidiMapping
{
    int trackIndex { -1 };
    int ccNumber { -1 };
    int channel { -1 };  // -1 means any channel
};

class MidiLearnManager
{
public:
    MidiLearnManager();
    ~MidiLearnManager() = default;

    void startLearning(int trackIndex);
    void stopLearning();
    bool isLearning() const { return learning; }
    int getLearningTrackIndex() const { return learningTrackIndex; }
    
    void processMidiMessages(const juce::MidiBuffer& midiMessages, StemEngine& engine);
    
    void setMapping(int trackIndex, int ccNumber, int channel = -1);
    void removeMapping(int trackIndex);
    MidiMapping* getMapping(int trackIndex);
    
    juce::ValueTree getStateAsValueTree() const;
    void loadStateFromValueTree(const juce::ValueTree& state);
    
    std::function<void()> onMappingChanged;

private:
    std::vector<MidiMapping> mappings;
    
    bool learning { false };
    int learningTrackIndex { -1 };
    
    juce::CriticalSection lock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};

