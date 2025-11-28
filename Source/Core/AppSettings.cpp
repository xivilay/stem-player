#include "AppSettings.h"

AppSettings::AppSettings()
{
    stemPatterns = StemDetector::getDefaultPatterns();
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
        
        auto patternsElement = xml->getChildByName("StemPatterns");
        if (patternsElement != nullptr)
        {
            stemPatterns.clear();
            
            for (auto* patternElement : patternsElement->getChildIterator())
            {
                if (patternElement->hasTagName("Pattern"))
                {
                    StemPattern pattern;
                    pattern.stemType = patternElement->getStringAttribute("type", "");
                    pattern.pattern = patternElement->getStringAttribute("pattern", "");
                    
                    if (pattern.stemType.isNotEmpty() && pattern.pattern.isNotEmpty())
                        stemPatterns.add(pattern);
                }
            }
        }
    }
}

void AppSettings::saveSettings()
{
    auto xml = std::make_unique<juce::XmlElement>("StemPlayerSettings");
    
    xml->setAttribute("defaultFolder", defaultFolder);
    xml->setAttribute("showSeparateChannels", showSeparateChannels);
    
    auto* patternsElement = xml->createNewChildElement("StemPatterns");
    
    for (const auto& pattern : stemPatterns)
    {
        auto* patternElement = patternsElement->createNewChildElement("Pattern");
        patternElement->setAttribute("type", pattern.stemType);
        patternElement->setAttribute("pattern", pattern.pattern);
    }
    
    auto file = getSettingsFile();
    xml->writeTo(file);
}

void AppSettings::setDefaultFolder(const juce::String& folder)
{
    defaultFolder = folder;
    saveSettings();
}

void AppSettings::setStemPatterns(const juce::Array<StemPattern>& patterns)
{
    stemPatterns = patterns;
    saveSettings();
}

void AppSettings::setShowSeparateChannels(bool separate)
{
    showSeparateChannels = separate;
    saveSettings();
}

