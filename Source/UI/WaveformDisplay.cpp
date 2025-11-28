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
                    
                    // Use different color for each channel
                    juce::Colour channelColour = (ch == 0) ? waveformColour : waveformColourRight;
                    
                    // Draw channel label
                    g.setColour(channelColour.withAlpha(0.6f));
                    g.setFont(juce::Font(10.0f));
                    juce::String channelLabel = (ch == 0) ? "L" : "R";
                    g.drawText(channelLabel, channelBounds.removeFromLeft(14).toNearestInt(), 
                               juce::Justification::centred);
                    
                    // Gradient for this channel
                    juce::ColourGradient gradient(channelColour.withAlpha(0.9f), 
                                                  channelBounds.getX(), channelBounds.getCentreY(),
                                                  channelColour.withAlpha(0.4f), 
                                                  channelBounds.getX(), channelBounds.getBottom(),
                                                  false);
                    g.setGradientFill(gradient);
                    
                    // Draw single channel
                    thumbnail->drawChannel(g, channelBounds.toNearestInt(), 
                                           0.0, thumbnail->getTotalLength(), ch, 1.0f);
                    
                    // Subtle separator line between channels
                    if (ch < numChannels - 1)
                    {
                        g.setColour(StemPlayerLookAndFeel::backgroundDark.withAlpha(0.5f));
                        g.drawHorizontalLine(static_cast<int>(channelBounds.getBottom() + channelSpacing / 2), 
                                             waveformBounds.getX(), waveformBounds.getRight());
                    }
                }
            }
            else
            {
                // Mixed mode - overlay all channels on top of each other
                int numChannels = thumbnail->getNumChannels();
                
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    // Use slightly different alpha for each channel to create depth
                    float alpha = (ch == 0) ? 0.9f : 0.7f;
                    
                    juce::ColourGradient gradient(waveformColour.withAlpha(alpha), 
                                                  waveformBounds.getX(), waveformBounds.getCentreY(),
                                                  waveformColour.withAlpha(alpha * 0.4f), 
                                                  waveformBounds.getX(), waveformBounds.getBottom(),
                                                  false);
                    g.setGradientFill(gradient);
                    
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
