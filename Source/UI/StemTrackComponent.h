#pragma once

#include <JuceHeader.h>
#include "../Core/StemTrack.h"
#include "../Core/StemDetector.h"
#include "WaveformDisplay.h"

// Custom slider that supports click-to-mute
class MuteableSlider : public juce::Slider
{
public:
    MuteableSlider();
    
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    bool isMuted() const { return muted; }
    void setMuted(bool shouldMute);
    
    std::function<void(bool)> onMuteChanged;

private:
    bool muted { false };
    double volumeBeforeMute { 1.0 };
    bool isDragging { false };
    juce::Point<int> mouseDownPosition;
    
    static constexpr int clickThreshold = 4;  // pixels
};

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
    
    // Get the stem type index (0=Vocals, 1=Drums, 2=Bass, 3=Other)
    int getTrackIndex() const { return trackIndex; }
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    std::function<void(int, float)> onVolumeChanged;
    std::function<void(double)> onPositionChanged;

private:
    int trackIndex;
    StemTrack* currentTrack { nullptr };
    bool trackLoaded { false };
    
    juce::Label stemNameLabel;
    MuteableSlider volumeSlider;
    WaveformDisplay waveformDisplay;
    
    // Colors for different stem types
    juce::Colour getStemColor(int stemIndex);
    juce::Colour getStemBackgroundColor(int stemIndex);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemTrackComponent)
};
