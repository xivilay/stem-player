#pragma once

#include <JuceHeader.h>
#include "../Core/StemTrack.h"
#include "../Core/StemDetector.h"
#include "WaveformDisplay.h"

class StemTrackComponent : public juce::Component
{
public:
    StemTrackComponent(int trackIndex);
    ~StemTrackComponent() override;

    void setTrack(StemTrack* track);
    void setTrackLoaded(bool loaded);
    void updatePlaybackPosition(double normalizedPosition);
    void setVolume(float volume);
    void setShowSeparateChannels(bool separate);
    void setDrawPlayhead(bool shouldDraw);
    
    // Get the waveform bounds relative to parent for overlay positioning
    juce::Rectangle<int> getWaveformBounds() const;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    std::function<void(int, float)> onVolumeChanged;
    std::function<void(double)> onPositionChanged;

private:
    int trackIndex;
    StemTrack* currentTrack { nullptr };
    bool trackLoaded { false };
    
    juce::Label stemNameLabel;
    juce::Slider volumeSlider;
    WaveformDisplay waveformDisplay;
    
    // Colors for different stem types
    juce::Colour getStemColor(int stemIndex);
    juce::Colour getStemBackgroundColor(int stemIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemTrackComponent)
};
