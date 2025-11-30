#pragma once

#include <JuceHeader.h>

// Fixed stem types in order
enum class StemType
{
    Vocals = 0,
    Drums,
    Bass,
    Other,
    NumTypes
};

struct StemRegexPattern
{
    StemType stemType;
    juce::String regex;  // Regex pattern to match this stem type
};

struct DetectedSong
{
    juce::String songName;
    std::array<juce::File, 4> stemFiles;  // Fixed order: Vocals, Drums, Bass, Other
    std::array<bool, 4> stemFound { false, false, false, false };
};

class StemDetector
{
public:
    StemDetector();
    ~StemDetector() = default;

    void setPatterns(const std::array<juce::String, 4>& patterns);
    const std::array<juce::String, 4>& getPatterns() const { return regexPatterns; }
    
    juce::Array<DetectedSong> scanDirectory(const juce::File& directory) const;
    
    static std::array<juce::String, 4> getDefaultPatterns();
    static juce::String getStemTypeName(StemType type);
    static juce::String getStemTypeName(int index);

private:
    StemType detectStemType(const juce::String& filename) const;
    juce::String extractSongName(const juce::String& filename, StemType detectedType) const;
    bool isAudioFile(const juce::File& file) const;
    bool matchesPattern(const juce::String& filename, const juce::String& pattern) const;
    
    std::array<juce::String, 4> regexPatterns;  // Vocals, Drums, Bass, Other
    juce::StringArray audioExtensions;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemDetector)
};
