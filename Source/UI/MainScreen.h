#pragma once

#include <JuceHeader.h>
#include "StemTrackComponent.h"

class StemPlayerAudioProcessor;
class StemPlayerAudioProcessorEditor;

class MainScreen : public juce::Component,
                   public juce::KeyListener
{
public:
    MainScreen(StemPlayerAudioProcessor& processor, 
               StemPlayerAudioProcessorEditor& editor);
    ~MainScreen() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    
    void songLoaded(const juce::String& songName);
    void updatePlaybackPosition();
    void updateWaveformDisplayMode();

private:
    void createTrackComponents();
    void updateTransportButtons();
    juce::String formatTime(double seconds);
    
    StemPlayerAudioProcessor& audioProcessor;
    StemPlayerAudioProcessorEditor& editor;
    
    juce::Label titleLabel;
    juce::Label songNameLabel;
    juce::TextButton backButton;
    juce::TextButton playPauseButton;
    juce::TextButton stopButton;
    juce::Label timeLabel;
    
    juce::Viewport tracksViewport;
    juce::Component tracksContainer;
    std::vector<std::unique_ptr<StemTrackComponent>> trackComponents;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainScreen)
};

