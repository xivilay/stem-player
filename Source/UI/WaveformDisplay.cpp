#include "WaveformDisplay.h"
#include "LookAndFeel.h"

WaveformDisplay::WaveformDisplay()
{
    waveformColour = StemPlayerLookAndFeel::waveformColor;
    backgroundColour = StemPlayerLookAndFeel::backgroundLight;
    playheadColour = StemPlayerLookAndFeel::playheadColor;
    
    startTimerHz(30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::setTrack(StemTrack* track)
{
    currentTrack = track;
    repaint();
}

void WaveformDisplay::setPlaybackPosition(double normalizedPosition)
{
    if (std::abs(playbackPosition - normalizedPosition) > 0.001)
    {
        playbackPosition = normalizedPosition;
        repaint();
    }
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background with rounded corners
    g.setColour(backgroundColour);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Draw waveform
    if (currentTrack != nullptr && currentTrack->getThumbnail() != nullptr)
    {
        auto* thumbnail = currentTrack->getThumbnail();
        
        if (thumbnail->getTotalLength() > 0)
        {
            auto waveformBounds = bounds.reduced(4.0f, 8.0f);
            
            // Gradient for waveform
            juce::ColourGradient gradient(waveformColour.withAlpha(0.9f), 
                                          waveformBounds.getX(), waveformBounds.getCentreY(),
                                          waveformColour.withAlpha(0.4f), 
                                          waveformBounds.getX(), waveformBounds.getBottom(),
                                          false);
            g.setGradientFill(gradient);
            
            thumbnail->drawChannels(g, waveformBounds.toNearestInt(), 
                                    0.0, thumbnail->getTotalLength(), 1.0f);
        }
    }
    else
    {
        // No track loaded placeholder
        g.setColour(StemPlayerLookAndFeel::textSecondary);
        g.drawText("No waveform", bounds, juce::Justification::centred);
    }
    
    // Draw playhead
    if (playbackPosition > 0.0 && playbackPosition <= 1.0)
    {
        float playheadX = bounds.getX() + 4.0f + 
                          (float)playbackPosition * (bounds.getWidth() - 8.0f);
        
        // Glow effect
        g.setColour(playheadColour.withAlpha(0.3f));
        g.fillRoundedRectangle(playheadX - 4.0f, bounds.getY() + 2.0f, 
                               8.0f, bounds.getHeight() - 4.0f, 2.0f);
        
        // Main playhead line
        g.setColour(playheadColour);
        g.fillRoundedRectangle(playheadX - 1.5f, bounds.getY() + 2.0f, 
                               3.0f, bounds.getHeight() - 4.0f, 1.5f);
    }
    
    // Border
    g.setColour(StemPlayerLookAndFeel::backgroundDark);
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);
}

void WaveformDisplay::resized()
{
    // Nothing specific needed here
}

void WaveformDisplay::mouseDown(const juce::MouseEvent& event)
{
    updatePositionFromMouse(event);
}

void WaveformDisplay::mouseDrag(const juce::MouseEvent& event)
{
    updatePositionFromMouse(event);
}

void WaveformDisplay::updatePositionFromMouse(const juce::MouseEvent& event)
{
    auto bounds = getLocalBounds().toFloat().reduced(4.0f, 0);
    
    double newPosition = (event.position.x - bounds.getX()) / bounds.getWidth();
    newPosition = juce::jlimit(0.0, 1.0, newPosition);
    
    playbackPosition = newPosition;
    repaint();
    
    if (onPositionChanged)
        onPositionChanged(newPosition);
}

void WaveformDisplay::timerCallback()
{
    // Check if thumbnail has been updated
    if (currentTrack != nullptr && currentTrack->getThumbnail() != nullptr)
    {
        if (currentTrack->getThumbnail()->isFullyLoaded())
            repaint();
    }
}

