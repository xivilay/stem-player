#include "WaveformDisplay.h"
#include "LookAndFeel.h"

WaveformDisplay::WaveformDisplay()
{
    waveformColour = StemPlayerLookAndFeel::waveformColor;
    waveformColourRight = juce::Colour(0xff60a5fa);  // Blue for right channel
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

void WaveformDisplay::setShowSeparateChannels(bool separate)
{
    if (showSeparateChannels != separate)
    {
        showSeparateChannels = separate;
        repaint();
    }
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Flat background
    g.setColour(backgroundColour);
    g.fillRect(bounds);
    
    // Draw waveform - full height
    if (currentTrack != nullptr && currentTrack->getThumbnail() != nullptr)
    {
        auto* thumbnail = currentTrack->getThumbnail();
        
        if (thumbnail->getTotalLength() > 0)
        {
            auto waveformBounds = bounds.reduced(2.0f, 2.0f);
            int numChannels = thumbnail->getNumChannels();
            
            if (showSeparateChannels && numChannels > 1)
            {
                // Draw each channel separately
                float channelHeight = waveformBounds.getHeight() / numChannels;
                float channelSpacing = 2.0f;
                
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    auto channelBounds = waveformBounds.withHeight(channelHeight - channelSpacing)
                                                       .withY(waveformBounds.getY() + ch * channelHeight);
                    
                    // Use stem color with slight variation for L/R
                    float alpha = (ch == 0) ? 1.0f : 0.75f;
                    
                    // Draw channel label
                    g.setColour(waveformColour.withAlpha(0.5f));
                    g.setFont(juce::Font(10.0f));
                    juce::String channelLabel = (ch == 0) ? "L" : "R";
                    g.drawText(channelLabel, channelBounds.removeFromLeft(14).toNearestInt(), 
                               juce::Justification::centred);
                    
                    // Draw channel waveform
                    g.setColour(waveformColour.withAlpha(alpha));
                    thumbnail->drawChannel(g, channelBounds.toNearestInt(), 
                                           0.0, thumbnail->getTotalLength(), ch, 1.0f);
                    
                    // Subtle separator line between channels
                    if (ch < numChannels - 1)
                    {
                        g.setColour(backgroundColour.darker(0.3f));
                        g.drawHorizontalLine(static_cast<int>(channelBounds.getBottom() + channelSpacing / 2), 
                                             waveformBounds.getX(), waveformBounds.getRight());
                    }
                }
            }
            else
            {
                // Mixed mode - overlay all channels with the stem color
                int numChannelsToDraw = thumbnail->getNumChannels();
                
                for (int ch = 0; ch < numChannelsToDraw; ++ch)
                {
                    // Use slightly different alpha for each channel to create depth
                    float alpha = (ch == 0) ? 1.0f : 0.7f;
                    
                    g.setColour(waveformColour.withAlpha(alpha));
                    thumbnail->drawChannel(g, waveformBounds.toNearestInt(), 
                                           0.0, thumbnail->getTotalLength(), ch, 1.0f);
                }
            }
        }
    }
    else
    {
        // No track loaded placeholder
        g.setColour(StemPlayerLookAndFeel::textSecondary);
        g.drawText("No waveform", bounds, juce::Justification::centred);
    }
    
    // Draw playhead (only if enabled) - minimal style
    if (drawPlayhead && playbackPosition > 0.0 && playbackPosition <= 1.0)
    {
        float playheadX = bounds.getX() + 2.0f + 
                          (float)playbackPosition * (bounds.getWidth() - 4.0f);
        
        // Simple vertical line
        g.setColour(playheadColour);
        g.fillRect(playheadX - 1.0f, bounds.getY(), 2.0f, bounds.getHeight());
    }
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
