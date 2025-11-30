#include "IconButton.h"
#include "LookAndFeel.h"
#include "BinaryData.h"

IconButton::IconButton(IconType type)
    : juce::Button(""), iconType(type)
{
    loadIcon();
}

void IconButton::setIconType(IconType type)
{
    iconType = type;
    loadIcon();
    repaint();
}

const char* IconButton::getIconData(IconType type, int& size)
{
    switch (type)
    {
        case IconType::Back:
            size = BinaryData::back_svgSize;
            return BinaryData::back_svg;
        case IconType::Play:
            size = BinaryData::play_svgSize;
            return BinaryData::play_svg;
        case IconType::Pause:
            size = BinaryData::pause_svgSize;
            return BinaryData::pause_svg;
        case IconType::Stop:
            size = BinaryData::stop_svgSize;
            return BinaryData::stop_svg;
        case IconType::Settings:
            size = BinaryData::settings_svgSize;
            return BinaryData::settings_svg;
        case IconType::Load:
            size = BinaryData::load_svgSize;
            return BinaryData::load_svg;
        case IconType::Browse:
            size = BinaryData::browse_svgSize;
            return BinaryData::browse_svg;
        default:
            size = 0;
            return nullptr;
    }
}

void IconButton::loadIcon()
{
    int size = 0;
    const char* data = getIconData(iconType, size);
    
    if (data != nullptr && size > 0)
    {
        iconDrawable = juce::Drawable::createFromImageData(data, static_cast<size_t>(size));
    }
}

void IconButton::paintButton(juce::Graphics& g, bool shouldDrawButtonAsHighlighted, 
                              bool shouldDrawButtonAsDown)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    juce::Colour bgColour = StemPlayerLookAndFeel::backgroundLight;
    
    if (shouldDrawButtonAsDown)
        bgColour = StemPlayerLookAndFeel::accentPrimary;
    else if (shouldDrawButtonAsHighlighted)
        bgColour = bgColour.brighter(0.15f);
    
    g.setColour(bgColour);
    g.fillRect(bounds);
    
    // Draw icon
    if (iconDrawable != nullptr)
    {
        // Create a copy of the drawable to modify colors
        auto iconCopy = iconDrawable->createCopy();
        
        // Icon color
        juce::Colour iconColour = shouldDrawButtonAsDown 
            ? StemPlayerLookAndFeel::backgroundDark 
            : StemPlayerLookAndFeel::textPrimary;
        
        // Replace white with the desired color
        iconCopy->replaceColour(juce::Colours::white, iconColour);
        
        // Calculate icon bounds with padding - less padding for Browse and Load icons
        float padding = (iconType == IconType::Browse || iconType == IconType::Load) ? 0.15f : 0.22f;
        auto iconBounds = bounds.reduced(bounds.getWidth() * padding);
        
        iconCopy->drawWithin(g, iconBounds, 
                             juce::RectanglePlacement::centred, 1.0f);
    }
}
