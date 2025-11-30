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
    
    // Set colors - dark background, light waveform
    waveformDisplay.setBackgroundColour(getStemBackgroundColor(trackIndex));
    waveformDisplay.setWaveformColour(getStemColor(trackIndex));
    
    repaint();
}

void StemTrackComponent::setTrackLoaded(bool loaded)
{
    trackLoaded = loaded;
    
    // Dim the controls if not loaded
    volumeSlider.setEnabled(loaded);
    
    float alpha = loaded ? 1.0f : 0.4f;
    stemNameLabel.setAlpha(alpha);
    
    // Set colors - darker/dimmer if not loaded
    auto bgColor = getStemBackgroundColor(trackIndex);
    auto waveColor = getStemColor(trackIndex);
    
    if (loaded)
    {
        waveformDisplay.setBackgroundColour(bgColor);
        waveformDisplay.setWaveformColour(waveColor);
    }
    else
    {
        waveformDisplay.setBackgroundColour(bgColor.darker(0.5f));
        waveformDisplay.setWaveformColour(waveColor.withAlpha(0.3f));
    }
    
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
    
    // Flat background for controls area
    g.setColour(StemPlayerLookAndFeel::backgroundMedium);
    g.fillRect(bounds.removeFromLeft(70.0f));
    
    // Thin bottom border for separation
    g.setColour(StemPlayerLookAndFeel::backgroundDark);
    g.fillRect(getLocalBounds().toFloat().removeFromBottom(1.0f));
    
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
    // Modern vibrant palette - returns the "light" accent color
    switch (stemIndex)
    {
        case 0:  // Vocals - Coral/Salmon
            return juce::Colour(0xffff6b6b);
        case 1:  // Drums - Golden Yellow
            return juce::Colour(0xfffeca57);
        case 2:  // Bass - Electric Blue
            return juce::Colour(0xff54a0ff);
        case 3:  // Other - Mint Green
        default:
            return juce::Colour(0xff5cd85c);
    }
}

juce::Colour StemTrackComponent::getStemBackgroundColor(int stemIndex)
{
    // Dark versions of the palette colors for backgrounds
    switch (stemIndex)
    {
        case 0:  // Vocals - Dark Coral
            return juce::Colour(0xff2d1f1f);
        case 1:  // Drums - Dark Gold
            return juce::Colour(0xff2d2a1a);
        case 2:  // Bass - Dark Blue
            return juce::Colour(0xff1a2333);
        case 3:  // Other - Dark Mint
        default:
            return juce::Colour(0xff1a2d1a);
    }
}
