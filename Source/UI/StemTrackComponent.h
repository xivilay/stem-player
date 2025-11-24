#pragma once

#include <JuceHeader.h>
#include "../Core/StemTrack.h"
#include "../Core/MidiLearnManager.h"
#include "WaveformDisplay.h"

class StemTrackComponent : public juce::Component
{
public:
    StemTrackComponent(int trackIndex, MidiLearnManager& midiManager);
    ~StemTrackComponent() override;

    void setTrack(StemTrack* track);
    void updatePlaybackPosition(double normalizedPosition);
    void updateMidiMappingDisplay();
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    
    std::function<void(int, float)> onVolumeChanged;
    std::function<void(double)> onPositionChanged;

private:
    void showVolumeContextMenu();
    
    int trackIndex;
    StemTrack* currentTrack { nullptr };
    MidiLearnManager& midiLearnManager;
    
    juce::Label stemNameLabel;
    juce::Slider volumeSlider;
    juce::Label volumeLabel;
    juce::Label midiMappingLabel;
    WaveformDisplay waveformDisplay;
    
    // Colors for different stem types
    juce::Colour getStemColor(const juce::String& stemType);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemTrackComponent)
};

