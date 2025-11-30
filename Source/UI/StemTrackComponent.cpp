#include "StemTrackComponent.h"
#include "LookAndFeel.h"

StemTrackComponent::StemTrackComponent(int index)
    : trackIndex(index)
{
    // Stem name label - use fixed stem type name
    stemNameLabel.setText(StemDetector::getStemTypeName(index), juce::dontSendNotification);
    stemNameLabel.setFont(juce::Font(11.0f, juce::Font::bold));
    stemNameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    stemNameLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(stemNameLabel);
    
    // Volume slider (rotary knob with value displayed inside)
    volumeSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                                      juce::MathConstants<float>::pi * 2.75f, true);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.onValueChange = [this]() {
        if (currentTrack != nullptr && trackLoaded)
        {
            currentTrack->setVolume(static_cast<float>(volumeSlider.getValue()));
            
            if (onVolumeChanged)
                onVolumeChanged(trackIndex, static_cast<float>(volumeSlider.getValue()));
        }
    };
    addAndMakeVisible(volumeSlider);
    
    // Waveform display
    waveformDisplay.onPositionChanged = [this](double pos) {
        if (onPositionChanged)
            onPositionChanged(pos);
    };
    addAndMakeVisible(waveformDisplay);
}

StemTrackComponent::~StemTrackComponent()
{
}

void StemTrackComponent::setTrack(StemTrack* track)
{
    currentTrack = track;
    
    // Always use fixed stem type name based on index
    stemNameLabel.setText(StemDetector::getStemTypeName(trackIndex), juce::dontSendNotification);
    
    if (track != nullptr)
    {
        volumeSlider.setValue(track->getVolume(), juce::dontSendNotification);
        waveformDisplay.setTrack(track);
    }
    else
    {
        waveformDisplay.setTrack(nullptr);
    }
    
    repaint();
}

void StemTrackComponent::setTrackLoaded(bool loaded)
{
    trackLoaded = loaded;
    
    // Dim the controls if not loaded
    volumeSlider.setEnabled(loaded);
    
    float alpha = loaded ? 1.0f : 0.4f;
    stemNameLabel.setAlpha(alpha);
    
    repaint();
}

void StemTrackComponent::updatePlaybackPosition(double normalizedPosition)
{
    waveformDisplay.setPlaybackPosition(normalizedPosition);
}

void StemTrackComponent::setVolume(float volume)
{
    volumeSlider.setValue(volume, juce::sendNotificationSync);
}

void StemTrackComponent::setShowSeparateChannels(bool separate)
{
    waveformDisplay.setShowSeparateChannels(separate);
}

void StemTrackComponent::setDrawPlayhead(bool shouldDraw)
{
    waveformDisplay.setDrawPlayhead(shouldDraw);
}

juce::Rectangle<int> StemTrackComponent::getWaveformBounds() const
{
    return waveformDisplay.getBounds().translated(getX(), getY());
}

void StemTrackComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Flat background - slightly darker if not loaded
    if (trackLoaded)
        g.setColour(StemPlayerLookAndFeel::backgroundMedium);
    else
        g.setColour(StemPlayerLookAndFeel::backgroundDark.brighter(0.1f));
    
    g.fillRect(bounds);
    
    // Color accent bar on left based on stem type
    g.setColour(getStemColor(trackIndex).withAlpha(trackLoaded ? 1.0f : 0.3f));
    g.fillRect(bounds.getX(), bounds.getY(), 3.0f, bounds.getHeight());
    
    // Thin bottom border for separation
    g.setColour(StemPlayerLookAndFeel::backgroundDark);
    g.fillRect(bounds.getX(), bounds.getBottom() - 1.0f, bounds.getWidth(), 1.0f);
    
    // Show "Not found" message if track not loaded
    if (!trackLoaded)
    {
        g.setColour(StemPlayerLookAndFeel::textSecondary.withAlpha(0.5f));
        g.setFont(juce::Font(10.0f));
        auto waveformBounds = waveformDisplay.getBounds();
        g.drawText("Not found", waveformBounds, juce::Justification::centred, false);
    }
}

void StemTrackComponent::resized()
{
    auto bounds = getLocalBounds();
    
    // Left side: compact controls area
    auto leftArea = bounds.removeFromLeft(70);
    leftArea.reduce(4, 4);
    
    // Stem name at top
    stemNameLabel.setBounds(leftArea.removeFromTop(16));
    leftArea.removeFromTop(2);
    
    // Rotary knob takes remaining space (centered)
    int knobSize = juce::jmin(leftArea.getWidth() - 4, leftArea.getHeight() - 4);
    knobSize = juce::jmax(knobSize, 40);
    auto knobBounds = leftArea.withSizeKeepingCentre(knobSize, knobSize);
    volumeSlider.setBounds(knobBounds);
    
    // Waveform takes full remaining height
    waveformDisplay.setBounds(bounds);
}

juce::Colour StemTrackComponent::getStemColor(int stemIndex)
{
    switch (stemIndex)
    {
        case 0:  // Vocals
            return juce::Colour(0xfff472b6);  // Pink
        case 1:  // Drums
            return juce::Colour(0xfffbbf24);  // Amber
        case 2:  // Bass
            return juce::Colour(0xff818cf8);  // Indigo
        case 3:  // Other
        default:
            return juce::Colour(0xff94a3b8);  // Slate
    }
}
