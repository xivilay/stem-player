#include "MainScreen.h"
#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include "LookAndFeel.h"

// PlayheadOverlay implementation
PlayheadOverlay::PlayheadOverlay(MainScreen& owner)
    : mainScreen(owner)
{
    setInterceptsMouseClicks(true, false);
}

void PlayheadOverlay::paint(juce::Graphics& g)
{
    if (getWidth() <= 0)
        return;
    
    if (playbackPosition >= 0.0 && playbackPosition <= 1.0)
    {
        // The overlay is now sized to match the waveform area exactly
        float playheadX = 4.0f + (float)playbackPosition * ((float)getWidth() - 8.0f);
        
        // Glow effect
        g.setColour(StemPlayerLookAndFeel::playheadColor.withAlpha(0.3f));
        g.fillRoundedRectangle(playheadX - 5.0f, 0.0f, 
                               10.0f, (float)getHeight(), 2.0f);
        
        // Main playhead line
        g.setColour(StemPlayerLookAndFeel::playheadColor);
        g.fillRoundedRectangle(playheadX - 1.5f, 0.0f, 
                               3.0f, (float)getHeight(), 1.5f);
        
        // Top and bottom caps for visibility
        g.fillEllipse(playheadX - 5.0f, -2.0f, 10.0f, 10.0f);
        g.fillEllipse(playheadX - 5.0f, (float)getHeight() - 8.0f, 10.0f, 10.0f);
    }
}

void PlayheadOverlay::mouseDown(const juce::MouseEvent& event)
{
    if (waveformArea.isEmpty())
        return;
    
    // The overlay is now positioned directly over the waveform area
    // so event.position.x is already relative to the waveform
    float relativeX = event.position.x - 4.0f;
    float width = (float)getWidth() - 8.0f;
    
    double newPosition = juce::jlimit(0.0, 1.0, (double)(relativeX / width));
    playbackPosition = newPosition;
    repaint();
    
    if (onPositionChanged)
        onPositionChanged(newPosition);
}

void PlayheadOverlay::mouseDrag(const juce::MouseEvent& event)
{
    mouseDown(event);
}

void PlayheadOverlay::setPlaybackPosition(double normalizedPosition)
{
    if (std::abs(playbackPosition - normalizedPosition) > 0.001)
    {
        playbackPosition = normalizedPosition;
        repaint();
    }
}

void PlayheadOverlay::setWaveformBounds(juce::Rectangle<int> bounds)
{
    waveformArea = bounds;
}

// MainScreen implementation
MainScreen::MainScreen(StemPlayerAudioProcessor& processor, 
                        StemPlayerAudioProcessorEditor& ed)
    : audioProcessor(processor), editor(ed), playheadOverlay(*this)
{
    // Title
    titleLabel.setText("Now Playing", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(14.0f));
    titleLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Song name
    songNameLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    songNameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    songNameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(songNameLabel);
    
    // Back button
    backButton.setButtonText("< Back");
    backButton.onClick = [this]() {
        audioProcessor.getStemEngine().stop();
        editor.showScreen(StemPlayerAudioProcessor::Screen::Selection);
    };
    addAndMakeVisible(backButton);
    
    // Play/Pause button
    playPauseButton.setButtonText("Play");
    playPauseButton.onClick = [this]() {
        audioProcessor.getStemEngine().togglePlayPause();
        updateTransportButtons();
    };
    addAndMakeVisible(playPauseButton);
    
    // Stop button
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this]() {
        audioProcessor.getStemEngine().stop();
        updateTransportButtons();
    };
    addAndMakeVisible(stopButton);
    
    // Time display
    timeLabel.setFont(juce::Font(14.0f, juce::Font::FontStyleFlags::plain));
    timeLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    timeLabel.setJustificationType(juce::Justification::centred);
    timeLabel.setText("0:00 / 0:00", juce::dontSendNotification);
    addAndMakeVisible(timeLabel);
    
    // Tracks viewport
    tracksViewport.setViewedComponent(&tracksContainer, false);
    tracksViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(tracksViewport);
    
    // Playhead overlay (added after viewport so it's on top)
    playheadOverlay.onPositionChanged = [this](double pos) {
        audioProcessor.getStemEngine().setPositionNormalized(pos);
    };
    addAndMakeVisible(playheadOverlay);
    
    // Enable keyboard focus
    setWantsKeyboardFocus(true);
    addKeyListener(this);
}

MainScreen::~MainScreen()
{
    removeKeyListener(this);
}

void MainScreen::visibilityChanged()
{
    if (isVisible())
        grabKeyboardFocus();
}

bool MainScreen::keyPressed(const juce::KeyPress& key, juce::Component* /*originatingComponent*/)
{
    // Space bar - Play/Pause
    if (key == juce::KeyPress::spaceKey)
    {
        audioProcessor.getStemEngine().togglePlayPause();
        updateTransportButtons();
        return true;
    }
    
    // Left arrow - Rewind
    if (key == juce::KeyPress::leftKey)
    {
        audioProcessor.getStemEngine().rewind();
        return true;
    }
    
    // Right arrow - Fast Forward
    if (key == juce::KeyPress::rightKey)
    {
        audioProcessor.getStemEngine().fastForward();
        return true;
    }
    
    // Escape or S - Stop
    if (key == juce::KeyPress::escapeKey || key.getTextCharacter() == 's' || key.getTextCharacter() == 'S')
    {
        audioProcessor.getStemEngine().stop();
        updateTransportButtons();
        return true;
    }
    
    // Home - Go to start
    if (key == juce::KeyPress::homeKey)
    {
        audioProcessor.getStemEngine().setPosition(0);
        return true;
    }
    
    return false;
}

void MainScreen::paint(juce::Graphics& g)
{
    g.fillAll(StemPlayerLookAndFeel::backgroundDark);
    
    // Subtle gradient at top
    juce::ColourGradient gradient(
        StemPlayerLookAndFeel::accentSecondary.withAlpha(0.08f), 
        (float)getWidth() * 0.5f, 0,
        juce::Colours::transparentBlack, 
        (float)getWidth() * 0.5f, 150, false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds().removeFromTop(150));
}

void MainScreen::resized()
{
    auto bounds = getLocalBounds();
    
    // Header area
    auto header = bounds.removeFromTop(120);
    header.reduce(20, 15);
    
    // Back button in top left
    backButton.setBounds(header.removeFromLeft(80).removeFromTop(30));
    header.removeFromLeft(10);
    
    // Title and song name centered
    auto titleArea = header.reduced(80, 0);
    titleLabel.setBounds(titleArea.removeFromTop(20));
    songNameLabel.setBounds(titleArea.removeFromTop(36));
    
    // Transport controls
    auto transportArea = titleArea.removeFromTop(50);
    auto transportBounds = transportArea.withSizeKeepingCentre(300, 40);
    
    playPauseButton.setBounds(transportBounds.removeFromLeft(90));
    transportBounds.removeFromLeft(10);
    stopButton.setBounds(transportBounds.removeFromLeft(70));
    transportBounds.removeFromLeft(10);
    timeLabel.setBounds(transportBounds);
    
    // Tracks area
    bounds.removeFromTop(10);
    auto viewportBounds = bounds.reduced(15, 0);
    tracksViewport.setBounds(viewportBounds);
    
    // Update tracks container size
    int trackHeight = 140;
    int spacing = 10;
    int totalHeight = (int)trackComponents.size() * (trackHeight + spacing);
    tracksContainer.setSize(tracksViewport.getWidth() - 20, 
                           juce::jmax(totalHeight, tracksViewport.getHeight()));
    
    // Layout track components
    int y = 0;
    for (auto& trackComp : trackComponents)
    {
        trackComp->setBounds(0, y, tracksContainer.getWidth(), trackHeight);
        y += trackHeight + spacing;
    }
    
    // Position and size playhead overlay will be set in updatePlayheadOverlay
    updatePlayheadOverlay();
}

void MainScreen::songLoaded(const juce::String& songName)
{
    songNameLabel.setText(songName, juce::dontSendNotification);
    createTrackComponents();
    updateWaveformDisplayMode();
    updateTransportButtons();
    resized();
}

void MainScreen::createTrackComponents()
{
    trackComponents.clear();
    
    auto& engine = audioProcessor.getStemEngine();
    int numTracks = engine.getNumTracks();
    
    for (int i = 0; i < numTracks; ++i)
    {
        auto trackComp = std::make_unique<StemTrackComponent>(i);
        
        trackComp->setTrack(engine.getTrack(i));
        trackComp->setDrawPlayhead(false);  // Disable individual playheads
        
        trackComp->onVolumeChanged = [this](int trackIndex, float volume) {
            audioProcessor.getStemEngine().setTrackVolume(trackIndex, volume);
        };
        
        trackComp->onPositionChanged = [this](double pos) {
            audioProcessor.getStemEngine().setPositionNormalized(pos);
        };
        
        tracksContainer.addAndMakeVisible(trackComp.get());
        trackComponents.push_back(std::move(trackComp));
    }
}

void MainScreen::updatePlayheadOverlay()
{
    // Calculate the waveform area bounds and position the overlay only over waveforms
    if (!trackComponents.empty())
    {
        // Get bounds of first track's waveform to determine horizontal extent
        auto firstTrackBounds = trackComponents[0]->getWaveformBounds();
        
        // Get viewport bounds relative to this component
        auto viewportBounds = tracksViewport.getBounds();
        
        // Calculate the waveform X position relative to the main screen
        // firstTrackBounds is relative to tracksContainer
        int waveformLocalX = firstTrackBounds.getX();
        int waveformWidth = firstTrackBounds.getWidth();
        
        // Account for viewport position and scroll
        auto viewPos = tracksViewport.getViewPosition();
        int waveformScreenX = viewportBounds.getX() + waveformLocalX - viewPos.x;
        
        // Position the overlay only over the waveform column
        playheadOverlay.setBounds(waveformScreenX, viewportBounds.getY(), 
                                  waveformWidth, viewportBounds.getHeight());
        
        // The waveform bounds within the overlay are now at x=0
        playheadOverlay.setWaveformBounds(juce::Rectangle<int>(
            0, 0, waveformWidth, playheadOverlay.getHeight()));
    }
}

void MainScreen::updatePlaybackPosition()
{
    auto& engine = audioProcessor.getStemEngine();
    double pos = engine.getPositionNormalized();
    
    // Update playhead overlay
    playheadOverlay.setPlaybackPosition(pos);
    updatePlayheadOverlay();
    
    // Still update individual track positions for waveform rendering (without playhead)
    for (auto& trackComp : trackComponents)
        trackComp->updatePlaybackPosition(pos);
    
    // Update time display
    double currentTime = engine.getPositionInSeconds();
    double totalTime = engine.getTotalLengthInSeconds();
    timeLabel.setText(formatTime(currentTime) + " / " + formatTime(totalTime), 
                      juce::dontSendNotification);
    
    // Update stem volumes from MIDI (in case they changed via MIDI)
    for (size_t i = 0; i < trackComponents.size(); ++i)
    {
        if (auto* track = engine.getTrack(static_cast<int>(i)))
            trackComponents[i]->setVolume(track->getVolume());
    }
    
    updateTransportButtons();
}

void MainScreen::updateTransportButtons()
{
    bool isPlaying = audioProcessor.getStemEngine().isPlaying();
    playPauseButton.setButtonText(isPlaying ? "Pause" : "Play");
}

juce::String MainScreen::formatTime(double seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    return juce::String(mins) + ":" + juce::String(secs).paddedLeft('0', 2);
}

void MainScreen::updateWaveformDisplayMode()
{
    bool separateChannels = audioProcessor.getAppSettings().getShowSeparateChannels();
    
    for (auto& trackComp : trackComponents)
        trackComp->setShowSeparateChannels(separateChannels);
}
