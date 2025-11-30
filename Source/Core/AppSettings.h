#pragma once

#include <JuceHeader.h>
#include "StemDetector.h"

class AppSettings
{
public:
    AppSettings();
    ~AppSettings() = default;

    void loadSettings();
    void saveSettings();
    
    juce::String getDefaultFolder() const { return defaultFolder; }
    void setDefaultFolder(const juce::String& folder);
    
    juce::Array<StemPattern>& getStemPatterns() { return stemPatterns; }
    const juce::Array<StemPattern>& getStemPatterns() const { return stemPatterns; }
    void setStemPatterns(const juce::Array<StemPattern>& patterns);
    
    bool getShowSeparateChannels() const { return showSeparateChannels; }
    void setShowSeparateChannels(bool separate);
    
    // Window state
    juce::Rectangle<int> getWindowBounds() const { return windowBounds; }
    void setWindowBounds(juce::Rectangle<int> bounds);
    bool hasWindowBounds() const { return windowBounds.getWidth() > 0 && windowBounds.getHeight() > 0; }
    
    static juce::File getSettingsFile();

private:
    juce::String defaultFolder;
    juce::Array<StemPattern> stemPatterns;
    bool showSeparateChannels { false };  // false = mixed, true = separate channels
    juce::Rectangle<int> windowBounds { 0, 0, 0, 0 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AppSettings)
};

