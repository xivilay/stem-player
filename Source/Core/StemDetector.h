#pragma once

#include <JuceHeader.h>

// Number of supported stem types
static constexpr int NUM_STEM_TYPES = 6;

// Fixed stem types in order (Other is always last)
enum class StemType
{
    Vocals = 0,
    Drums,
    Bass,
    Guitar,
    Piano,
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
    std::array<juce::File, NUM_STEM_TYPES> stemFiles;  // Fixed order: Vocals, Drums, Bass, Guitar, Piano, Other
    std::array<bool, NUM_STEM_TYPES> stemFound { false, false, false, false, false, false };
};

class StemDetector
{
public:
    StemDetector();
    ~StemDetector() = default;

    void setPatterns(const std::array<juce::String, NUM_STEM_TYPES>& patterns);
    const std::array<juce::String, NUM_STEM_TYPES>& getPatterns() const { return regexPatterns; }
    
    juce::Array<DetectedSong> scanDirectory(const juce::File& directory) const;
    
    static std::array<juce::String, NUM_STEM_TYPES> getDefaultPatterns();
    static juce::String getStemTypeName(StemType type);
    static juce::String getStemTypeName(int index);

private:
    StemType detectStemType(const juce::String& filename) const;
    juce::String extractSongName(const juce::String& filename, StemType detectedType) const;
    bool isAudioFile(const juce::File& file) const;
    bool matchesPattern(const juce::String& filename, const juce::String& pattern) const;
    
    std::array<juce::String, NUM_STEM_TYPES> regexPatterns;  // Vocals, Drums, Bass, Guitar, Piano, Other
    juce::StringArray audioExtensions;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemDetector)
};
