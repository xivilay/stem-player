#pragma once

#include <JuceHeader.h>

class StemPlayerLookAndFeel : public juce::LookAndFeel_V4
{
public:
    StemPlayerLookAndFeel();
    ~StemPlayerLookAndFeel() override = default;

    // Colors
    static const juce::Colour backgroundDark;
    static const juce::Colour backgroundMedium;
    static const juce::Colour backgroundLight;
    static const juce::Colour accentPrimary;
    static const juce::Colour accentSecondary;
    static const juce::Colour textPrimary;
    static const juce::Colour textSecondary;
    static const juce::Colour waveformColor;
    static const juce::Colour playheadColor;
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider& slider) override;

    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider& slider) override;

    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                              const juce::Colour& backgroundColour,
                              bool shouldDrawButtonAsHighlighted,
                              bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g, juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    juce::Font getTextButtonFont(juce::TextButton& button, int buttonHeight) override;
    
    void drawScrollbar(juce::Graphics& g, juce::ScrollBar& scrollbar, int x, int y,
                       int width, int height, bool isScrollbarVertical, int thumbStartPosition,
                       int thumbSize, bool isMouseOver, bool isMouseDown) override;

private:
    juce::Font mainFont;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemPlayerLookAndFeel)
};

