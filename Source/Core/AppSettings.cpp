#include "AppSettings.h"

AppSettings::AppSettings()
{
    stemRegexPatterns = StemDetector::getDefaultPatterns();
}

juce::File AppSettings::getSettingsFile()
{
    auto settingsDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                           .getChildFile("StemPlayer");
    
    if (!settingsDir.exists())
        settingsDir.createDirectory();
    
    return settingsDir.getChildFile("settings.xml");
}

void AppSettings::loadSettings()
{
    auto file = getSettingsFile();
    
    if (!file.existsAsFile())
        return;
    
    auto xml = juce::XmlDocument::parse(file);
    
    if (xml == nullptr)
        return;
    
    if (xml->hasTagName("StemPlayerSettings"))
    {
        defaultFolder = xml->getStringAttribute("defaultFolder", "");
        showSeparateChannels = xml->getBoolAttribute("showSeparateChannels", false);
        
        // Load window bounds
        int wx = xml->getIntAttribute("windowX", 0);
        int wy = xml->getIntAttribute("windowY", 0);
        int ww = xml->getIntAttribute("windowWidth", 0);
        int wh = xml->getIntAttribute("windowHeight", 0);
        if (ww > 0 && wh > 0)
            windowBounds = juce::Rectangle<int>(wx, wy, ww, wh);
        
        // Load stem regex patterns
        auto patternsElement = xml->getChildByName("StemPatterns");
        if (patternsElement != nullptr)
        {
            auto defaults = StemDetector::getDefaultPatterns();
            stemRegexPatterns[0] = patternsElement->getStringAttribute("vocals", defaults[0]);
            stemRegexPatterns[1] = patternsElement->getStringAttribute("drums", defaults[1]);
            stemRegexPatterns[2] = patternsElement->getStringAttribute("bass", defaults[2]);
            stemRegexPatterns[3] = patternsElement->getStringAttribute("guitar", defaults[3]);
            stemRegexPatterns[4] = patternsElement->getStringAttribute("piano", defaults[4]);
            stemRegexPatterns[5] = patternsElement->getStringAttribute("other", defaults[5]);
        }
    }
}

void AppSettings::saveSettings()
{
    auto xml = std::make_unique<juce::XmlElement>("StemPlayerSettings");
    
    xml->setAttribute("defaultFolder", defaultFolder);
    xml->setAttribute("showSeparateChannels", showSeparateChannels);
    
    // Save window bounds
    if (windowBounds.getWidth() > 0 && windowBounds.getHeight() > 0)
    {
        xml->setAttribute("windowX", windowBounds.getX());
        xml->setAttribute("windowY", windowBounds.getY());
        xml->setAttribute("windowWidth", windowBounds.getWidth());
        xml->setAttribute("windowHeight", windowBounds.getHeight());
    }
    
    // Save stem regex patterns
    auto* patternsElement = xml->createNewChildElement("StemPatterns");
    patternsElement->setAttribute("vocals", stemRegexPatterns[0]);
    patternsElement->setAttribute("drums", stemRegexPatterns[1]);
    patternsElement->setAttribute("bass", stemRegexPatterns[2]);
    patternsElement->setAttribute("guitar", stemRegexPatterns[3]);
    patternsElement->setAttribute("piano", stemRegexPatterns[4]);
    patternsElement->setAttribute("other", stemRegexPatterns[5]);
    
    auto file = getSettingsFile();
    xml->writeTo(file);
}

void AppSettings::setDefaultFolder(const juce::String& folder)
{
    defaultFolder = folder;
    saveSettings();
}

void AppSettings::setStemRegexPatterns(const std::array<juce::String, NUM_STEM_TYPES>& patterns)
{
    stemRegexPatterns = patterns;
    saveSettings();
}

void AppSettings::resetStemRegexToDefaults()
{
    stemRegexPatterns = StemDetector::getDefaultPatterns();
    saveSettings();
}

void AppSettings::setShowSeparateChannels(bool separate)
{
    showSeparateChannels = separate;
    saveSettings();
}

void AppSettings::setWindowBounds(juce::Rectangle<int> bounds)
{
    windowBounds = bounds;
    saveSettings();
}
