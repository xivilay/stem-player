#pragma once

#include <JuceHeader.h>
#include "Core/StemEngine.h"
#include "Core/MidiLearnManager.h"
#include "Core/AppSettings.h"

class StemPlayerAudioProcessor : public juce::AudioProcessor
{
public:
    StemPlayerAudioProcessor();
    ~StemPlayerAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    StemEngine& getStemEngine() { return stemEngine; }
    MidiLearnManager& getMidiLearnManager() { return midiLearnManager; }
    AppSettings& getAppSettings() { return appSettings; }

    enum class Screen { Selection, Main, Settings };
    Screen getCurrentScreen() const { return currentScreen; }
    void setCurrentScreen(Screen screen) { currentScreen = screen; }

private:
    StemEngine stemEngine;
    MidiLearnManager midiLearnManager;
    AppSettings appSettings;
    Screen currentScreen { Screen::Selection };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemPlayerAudioProcessor)
};

