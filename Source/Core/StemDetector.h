#pragma once

#include <JuceHeader.h>

struct StemPattern
{
    juce::String stemType;  // e.g., "Drums", "Vocals", "Bass"
    juce::String pattern;   // e.g., "_drums", " (Drums)", "-drums"
};

struct DetectedSong
{
    juce::String songName;
    juce::Array<juce::File> stemFiles;
    juce::StringArray stemTypes;
};

class StemDetector
{
public:
    StemDetector();
    ~StemDetector() = default;

    void setPatterns(const juce::Array<StemPattern>& patterns);
    juce::Array<StemPattern>& getPatterns() { return stemPatterns; }
    const juce::Array<StemPattern>& getPatterns() const { return stemPatterns; }
    
    juce::Array<DetectedSong> scanDirectory(const juce::File& directory) const;
    
    static juce::Array<StemPattern> getDefaultPatterns();

private:
    juce::String extractSongName(const juce::String& filename) const;
    juce::String detectStemType(const juce::String& filename) const;
    bool isAudioFile(const juce::File& file) const;
    
    juce::Array<StemPattern> stemPatterns;
    juce::StringArray audioExtensions;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemDetector)
};

