#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "UI/SelectionScreen.h"
#include "UI/MainScreen.h"
#include "UI/SettingsScreen.h"
#include "UI/LookAndFeel.h"

class StemPlayerAudioProcessorEditor : public juce::AudioProcessorEditor,
                                        public juce::Timer
{
public:
    explicit StemPlayerAudioProcessorEditor(StemPlayerAudioProcessor&);
    ~StemPlayerAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    void moved() override;
    void timerCallback() override;

    void showScreen(StemPlayerAudioProcessor::Screen screen);
    void onSongSelected(const juce::String& songName, const juce::Array<juce::File>& stemFiles);

private:
    void saveWindowBounds();
    
    StemPlayerAudioProcessor& audioProcessor;
    
    StemPlayerLookAndFeel customLookAndFeel;
    
    std::unique_ptr<SelectionScreen> selectionScreen;
    std::unique_ptr<MainScreen> mainScreen;
    std::unique_ptr<SettingsScreen> settingsScreen;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemPlayerAudioProcessorEditor)
};

