#include "LookAndFeel.h"

// Deep space / synthwave inspired color palette
const juce::Colour StemPlayerLookAndFeel::backgroundDark   = juce::Colour(0xff0d0d1a);
const juce::Colour StemPlayerLookAndFeel::backgroundMedium = juce::Colour(0xff1a1a2e);
const juce::Colour StemPlayerLookAndFeel::backgroundLight  = juce::Colour(0xff252542);
const juce::Colour StemPlayerLookAndFeel::accentPrimary    = juce::Colour(0xffe94560);
const juce::Colour StemPlayerLookAndFeel::accentSecondary  = juce::Colour(0xff0f9b8e);
const juce::Colour StemPlayerLookAndFeel::textPrimary      = juce::Colour(0xffeaeaea);
const juce::Colour StemPlayerLookAndFeel::textSecondary    = juce::Colour(0xff8888aa);
const juce::Colour StemPlayerLookAndFeel::waveformColor    = juce::Colour(0xff6ee7b7);
const juce::Colour StemPlayerLookAndFeel::playheadColor    = juce::Colour(0xfffbbf24);

StemPlayerLookAndFeel::StemPlayerLookAndFeel()
{
    // Set default colors
    setColour(juce::ResizableWindow::backgroundColourId, backgroundDark);
    setColour(juce::TextButton::buttonColourId, backgroundLight);
    setColour(juce::TextButton::textColourOffId, textPrimary);
    setColour(juce::TextButton::textColourOnId, textPrimary);
    setColour(juce::ComboBox::backgroundColourId, backgroundLight);
    setColour(juce::ComboBox::textColourId, textPrimary);
    setColour(juce::ComboBox::outlineColourId, backgroundLight);
    setColour(juce::PopupMenu::backgroundColourId, backgroundMedium);
    setColour(juce::PopupMenu::textColourId, textPrimary);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, accentPrimary);
    setColour(juce::Label::textColourId, textPrimary);
    setColour(juce::TextEditor::backgroundColourId, backgroundLight);
    setColour(juce::TextEditor::textColourId, textPrimary);
    setColour(juce::TextEditor::outlineColourId, backgroundLight);
    setColour(juce::ListBox::backgroundColourId, backgroundMedium);
    setColour(juce::ListBox::textColourId, textPrimary);
    setColour(juce::ScrollBar::thumbColourId, backgroundLight);
}

void StemPlayerLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPosProportional, float rotaryStartAngle,
                                              float rotaryEndAngle, juce::Slider& slider)
{
    auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
    auto centreX = (float)x + (float)width * 0.5f;
    auto centreY = (float)y + (float)height * 0.5f;
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Outer ring / background
    g.setColour(backgroundLight);
    g.fillEllipse(rx, ry, rw, rw);
    
    // Inner circle (darker)
    float innerRadius = radius * 0.65f;
    g.setColour(backgroundDark);
    g.fillEllipse(centreX - innerRadius, centreY - innerRadius, innerRadius * 2.0f, innerRadius * 2.0f);

    // Value arc on the outer ring
    juce::Path valueArc;
    float arcRadius = radius * 0.85f;
    valueArc.addCentredArc(centreX, centreY, arcRadius, arcRadius, 0.0f, 
                           rotaryStartAngle, angle, true);
    g.setColour(accentPrimary);
    g.strokePath(valueArc, juce::PathStrokeType(5.0f, juce::PathStrokeType::curved, 
                                                 juce::PathStrokeType::rounded));

    // Small tick mark at current position
    float tickLength = radius * 0.2f;
    float tickInnerRadius = radius * 0.7f;
    juce::Path tick;
    tick.addRectangle(-1.5f, -tickInnerRadius - tickLength, 3.0f, tickLength);
    tick.applyTransform(juce::AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(textPrimary);
    g.fillPath(tick);

    // Draw value text in center
    int valuePercent = static_cast<int>(sliderPosProportional * 100.0f + 0.5f);
    juce::String valueText = juce::String(valuePercent) + "%";
    
    float fontSize = juce::jmin(innerRadius * 0.7f, 14.0f);
    g.setFont(juce::Font(fontSize, juce::Font::bold));
    g.setColour(textPrimary);
    
    juce::Rectangle<float> textBounds(centreX - innerRadius, centreY - innerRadius * 0.5f, 
                                       innerRadius * 2.0f, innerRadius);
    g.drawText(valueText, textBounds, juce::Justification::centred, false);
    
    juce::ignoreUnused(slider);
}

void StemPlayerLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                              float sliderPos, float /*minSliderPos*/, float /*maxSliderPos*/,
                                              const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    if (style == juce::Slider::LinearVertical)
    {
        auto trackWidth = 6.0f;
        auto trackX = (float)x + (float)width * 0.5f - trackWidth * 0.5f;
        
        // Track background
        g.setColour(backgroundLight);
        g.fillRoundedRectangle(trackX, (float)y, trackWidth, (float)height, 3.0f);
        
        // Value portion
        auto valueHeight = (float)height - (sliderPos - (float)y);
        g.setColour(accentSecondary);
        g.fillRoundedRectangle(trackX, sliderPos, trackWidth, valueHeight, 3.0f);
        
        // Thumb
        auto thumbSize = 16.0f;
        g.setColour(textPrimary);
        g.fillEllipse((float)x + (float)width * 0.5f - thumbSize * 0.5f, 
                      sliderPos - thumbSize * 0.5f, thumbSize, thumbSize);
    }
    else if (style == juce::Slider::LinearHorizontal)
    {
        auto trackHeight = 6.0f;
        auto trackY = (float)y + (float)height * 0.5f - trackHeight * 0.5f;
        
        // Track background
        g.setColour(backgroundLight);
        g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, 3.0f);
        
        // Value portion
        auto valueWidth = sliderPos - (float)x;
        g.setColour(accentSecondary);
        g.fillRoundedRectangle((float)x, trackY, valueWidth, trackHeight, 3.0f);
        
        // Thumb
        auto thumbSize = 14.0f;
        g.setColour(textPrimary);
        g.fillEllipse(sliderPos - thumbSize * 0.5f,
                      (float)y + (float)height * 0.5f - thumbSize * 0.5f, 
                      thumbSize, thumbSize);
    }
    else
    {
        juce::LookAndFeel_V4::drawLinearSlider(g, x, y, width, height, sliderPos, 
                                                0, 0, style, slider);
    }
}

void StemPlayerLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                                  const juce::Colour& /*backgroundColour*/,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    
    juce::Colour baseColour = backgroundLight;
    
    if (shouldDrawButtonAsDown)
        baseColour = accentPrimary;
    else if (shouldDrawButtonAsHighlighted)
        baseColour = backgroundLight.brighter(0.2f);
    
    if (button.getToggleState())
        baseColour = accentPrimary;
    
    g.setColour(baseColour);
    g.fillRoundedRectangle(bounds, 6.0f);
    
    // Subtle border
    g.setColour(baseColour.brighter(0.1f));
    g.drawRoundedRectangle(bounds, 6.0f, 1.0f);
}

void StemPlayerLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& button,
                                            bool /*shouldDrawButtonAsHighlighted*/,
                                            bool /*shouldDrawButtonAsDown*/)
{
    auto font = getTextButtonFont(button, button.getHeight());
    g.setFont(font);
    g.setColour(button.findColour(button.getToggleState() ? juce::TextButton::textColourOnId
                                                           : juce::TextButton::textColourOffId));
    
    auto bounds = button.getLocalBounds();
    g.drawText(button.getButtonText(), bounds, juce::Justification::centred, true);
}

juce::Font StemPlayerLookAndFeel::getTextButtonFont(juce::TextButton& /*button*/, int buttonHeight)
{
    return juce::Font(juce::jmin(15.0f, (float)buttonHeight * 0.55f));
}

void StemPlayerLookAndFeel::drawScrollbar(juce::Graphics& g, juce::ScrollBar& /*scrollbar*/, 
                                           int x, int y, int width, int height,
                                           bool isScrollbarVertical, int thumbStartPosition,
                                           int thumbSize, bool isMouseOver, bool /*isMouseDown*/)
{
    g.setColour(backgroundDark);
    g.fillRect(x, y, width, height);
    
    juce::Colour thumbColour = backgroundLight;
    if (isMouseOver)
        thumbColour = thumbColour.brighter(0.2f);
    
    g.setColour(thumbColour);
    
    if (isScrollbarVertical)
    {
        g.fillRoundedRectangle((float)x + 2.0f, (float)thumbStartPosition, 
                               (float)width - 4.0f, (float)thumbSize, 4.0f);
    }
    else
    {
        g.fillRoundedRectangle((float)thumbStartPosition, (float)y + 2.0f, 
                               (float)thumbSize, (float)height - 4.0f, 4.0f);
    }
}

