#pragma once

#include <JuceHeader.h>
#include "../Core/StemTrack.h"

class WaveformDisplay : public juce::Component,
                         public juce::Timer
{
public:
    WaveformDisplay();
    ~WaveformDisplay() override;

    void setTrack(StemTrack* track);
    void setPlaybackPosition(double normalizedPosition);
    void setShowSeparateChannels(bool separate);
    void setDrawPlayhead(bool shouldDraw) { drawPlayhead = shouldDraw; repaint(); }
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseDrag(const juce::MouseEvent& event) override;
    
    void timerCallback() override;
    
    std::function<void(double)> onPositionChanged;

private:
    void updatePositionFromMouse(const juce::MouseEvent& event);
    
    StemTrack* currentTrack { nullptr };
    double playbackPosition { 0.0 };
    bool showSeparateChannels { false };
    bool drawPlayhead { true };
    
    juce::Colour waveformColour { 0xff6ee7b7 };
    juce::Colour waveformColourRight { 0xff60a5fa };  // Blue for right channel
    juce::Colour backgroundColour { 0xff1a1a2e };
    juce::Colour playheadColour { 0xfffbbf24 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformDisplay)
};

