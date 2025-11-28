#include "StemTrackComponent.h"
#include "LookAndFeel.h"

StemTrackComponent::StemTrackComponent(int index)
    : trackIndex(index)
{
    // Stem name label
    stemNameLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    stemNameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    stemNameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(stemNameLabel);
    
    // Volume slider
    volumeSlider.setSliderStyle(juce::Slider::LinearVertical);
    volumeSlider.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(1.0);
    volumeSlider.onValueChange = [this]() {
        if (currentTrack != nullptr)
        {
            currentTrack->setVolume(static_cast<float>(volumeSlider.getValue()));
            volumeLabel.setText(juce::String(static_cast<int>(volumeSlider.getValue() * 100)) + "%", 
                               juce::dontSendNotification);
            
            if (onVolumeChanged)
                onVolumeChanged(trackIndex, static_cast<float>(volumeSlider.getValue()));
        }
    };
    addAndMakeVisible(volumeSlider);
    
    // Volume percentage label
    volumeLabel.setFont(juce::Font(12.0f));
    volumeLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    volumeLabel.setJustificationType(juce::Justification::centred);
    volumeLabel.setText("100%", juce::dontSendNotification);
    addAndMakeVisible(volumeLabel);
    
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
    
    if (track != nullptr)
    {
        stemNameLabel.setText(track->getStemType(), juce::dontSendNotification);
        volumeSlider.setValue(track->getVolume(), juce::dontSendNotification);
        waveformDisplay.setTrack(track);
    }
    else
    {
        stemNameLabel.setText("", juce::dontSendNotification);
        waveformDisplay.setTrack(nullptr);
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
    
    // Background with stem type color accent
    g.setColour(StemPlayerLookAndFeel::backgroundMedium);
    g.fillRoundedRectangle(bounds, 8.0f);
    
    // Color accent bar on left
    if (currentTrack != nullptr)
    {
        g.setColour(getStemColor(currentTrack->getStemType()));
        g.fillRoundedRectangle(bounds.getX(), bounds.getY(), 4.0f, bounds.getHeight(), 2.0f);
    }
    
    // Subtle border
    g.setColour(StemPlayerLookAndFeel::backgroundLight);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 8.0f, 1.0f);
}

void StemTrackComponent::resized()
{
    auto bounds = getLocalBounds().reduced(12, 8);
    
    // Left side: stem name and volume controls
    auto leftArea = bounds.removeFromLeft(80);
    
    stemNameLabel.setBounds(leftArea.removeFromTop(24));
    
    auto sliderArea = leftArea.reduced(20, 4);
    volumeSlider.setBounds(sliderArea.removeFromTop(sliderArea.getHeight() - 18));
    volumeLabel.setBounds(sliderArea);
    
    // Rest: waveform display
    bounds.removeFromLeft(8);
    waveformDisplay.setBounds(bounds);
}

juce::Colour StemTrackComponent::getStemColor(const juce::String& stemType)
{
    juce::String lowerType = stemType.toLowerCase();
    
    if (lowerType.contains("vocal"))
        return juce::Colour(0xfff472b6);  // Pink
    if (lowerType.contains("drum"))
        return juce::Colour(0xfffbbf24);  // Amber
    if (lowerType.contains("bass"))
        return juce::Colour(0xff818cf8);  // Indigo
    if (lowerType.contains("piano") || lowerType.contains("keys"))
        return juce::Colour(0xff34d399);  // Emerald
    if (lowerType.contains("guitar"))
        return juce::Colour(0xfff97316);  // Orange
    if (lowerType.contains("synth"))
        return juce::Colour(0xffa78bfa);  // Purple
    if (lowerType.contains("string"))
        return juce::Colour(0xff22d3ee);  // Cyan
    if (lowerType.contains("other"))
        return juce::Colour(0xff94a3b8);  // Slate
    
    return StemPlayerLookAndFeel::accentSecondary;
}
