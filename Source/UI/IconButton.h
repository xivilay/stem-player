#pragma once

#include <JuceHeader.h>

enum class IconType
{
    Back,
    Play,
    Pause,
    Stop,
    Settings,
    Load,
    Browse
};

class IconButton : public juce::Button
{
public:
    IconButton(IconType type);
    
    void setIconType(IconType type);
    IconType getIconType() const { return iconType; }
    
    void paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, 
                     bool shouldDrawButtonAsDown) override;

private:
    void loadIcon();
    static const char* getIconData(IconType type, int& size);
    
    IconType iconType;
    std::unique_ptr<juce::Drawable> iconDrawable;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(IconButton)
};
