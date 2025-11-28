#include "PluginProcessor.h"
#include "PluginEditor.h"

StemPlayerAudioProcessorEditor::StemPlayerAudioProcessorEditor(StemPlayerAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    setLookAndFeel(&customLookAndFeel);
    
    selectionScreen = std::make_unique<SelectionScreen>(p, *this);
    mainScreen = std::make_unique<MainScreen>(p, *this);
    settingsScreen = std::make_unique<SettingsScreen>(p, *this);
    
    addChildComponent(selectionScreen.get());
    addChildComponent(mainScreen.get());
    addChildComponent(settingsScreen.get());
    
    showScreen(audioProcessor.getCurrentScreen());
    
    setSize(900, 700);
    setResizable(true, true);
    setResizeLimits(600, 400, 1920, 1080);
    
    startTimerHz(30);
}

StemPlayerAudioProcessorEditor::~StemPlayerAudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

void StemPlayerAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1a1a2e));
}

void StemPlayerAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    
    selectionScreen->setBounds(bounds);
    mainScreen->setBounds(bounds);
    settingsScreen->setBounds(bounds);
}

void StemPlayerAudioProcessorEditor::timerCallback()
{
    if (mainScreen->isVisible())
        mainScreen->updatePlaybackPosition();
}

void StemPlayerAudioProcessorEditor::showScreen(StemPlayerAudioProcessor::Screen screen)
{
    audioProcessor.setCurrentScreen(screen);
    
    selectionScreen->setVisible(screen == StemPlayerAudioProcessor::Screen::Selection);
    mainScreen->setVisible(screen == StemPlayerAudioProcessor::Screen::Main);
    settingsScreen->setVisible(screen == StemPlayerAudioProcessor::Screen::Settings);
    
    if (screen == StemPlayerAudioProcessor::Screen::Selection)
        selectionScreen->refresh();
    
    if (screen == StemPlayerAudioProcessor::Screen::Main)
        mainScreen->updateWaveformDisplayMode();
}

void StemPlayerAudioProcessorEditor::onSongSelected(const juce::String& songName, 
                                                     const juce::Array<juce::File>& stemFiles)
{
    audioProcessor.getStemEngine().loadSong(songName, stemFiles);
    mainScreen->songLoaded(songName);
    showScreen(StemPlayerAudioProcessor::Screen::Main);
}

