#pragma once

#include <JuceHeader.h>

class StemEngine;

enum class MidiControlType
{
    Stem1Volume = 0,
    Stem2Volume,
    Stem3Volume,
    Stem4Volume,
    Stem5Volume,
    PlayPause,
    Stop,
    Rewind,
    FastForward,
    NumControls
};

struct MidiMapping
{
    MidiControlType controlType { MidiControlType::Stem1Volume };
    int ccNumber { -1 };
    int channel { -1 };  // -1 means any channel
};

class MidiLearnManager
{
public:
    MidiLearnManager();
    ~MidiLearnManager() = default;

    void startLearning(MidiControlType controlType);
    void stopLearning();
    bool isLearning() const { return learning; }
    MidiControlType getLearningControlType() const { return learningControlType; }
    
    void processMidiMessages(const juce::MidiBuffer& midiMessages, StemEngine& engine);
    
    void setMapping(MidiControlType controlType, int ccNumber, int channel = -1);
    void removeMapping(MidiControlType controlType);
    int getMappedCC(MidiControlType controlType) const;
    
    juce::ValueTree getStateAsValueTree() const;
    void loadStateFromValueTree(const juce::ValueTree& state);
    
    std::function<void(MidiControlType, int)> onMappingChanged;

    static juce::String getControlName(MidiControlType type);
    static constexpr int getNumControls() { return static_cast<int>(MidiControlType::NumControls); }

private:
    std::array<MidiMapping, static_cast<size_t>(MidiControlType::NumControls)> mappings;
    
    bool learning { false };
    MidiControlType learningControlType { MidiControlType::Stem1Volume };
    
    juce::CriticalSection lock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiLearnManager)
};
