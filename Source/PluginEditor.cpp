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
    
    // Load saved window size or use default
    auto& settings = audioProcessor.getAppSettings();
    if (settings.hasWindowBounds())
    {
        auto bounds = settings.getWindowBounds();
        setSize(bounds.getWidth(), bounds.getHeight());
    }
    else
    {
        setSize(900, 700);
    }
    
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
    
    saveWindowBounds();
}

void StemPlayerAudioProcessorEditor::moved()
{
    saveWindowBounds();
}

void StemPlayerAudioProcessorEditor::saveWindowBounds()
{
    // Only save if we have a valid size
    if (getWidth() > 0 && getHeight() > 0)
    {
        auto screenPos = getScreenPosition();
        auto bounds = juce::Rectangle<int>(screenPos.x, screenPos.y, getWidth(), getHeight());
        audioProcessor.getAppSettings().setWindowBounds(bounds);
    }
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
                                                     const std::array<juce::File, NUM_STEM_TYPES>& stemFiles,
                                                     const std::array<bool, NUM_STEM_TYPES>& stemFound)
{
    audioProcessor.getStemEngine().loadSong(songName, stemFiles, stemFound);
    mainScreen->songLoaded(songName);
    showScreen(StemPlayerAudioProcessor::Screen::Main);
}

