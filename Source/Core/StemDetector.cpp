#include "StemDetector.h"

StemDetector::StemDetector()
{
    stemPatterns = getDefaultPatterns();
    
    audioExtensions.add(".mp3");
    audioExtensions.add(".wav");
    audioExtensions.add(".aiff");
    audioExtensions.add(".flac");
    audioExtensions.add(".ogg");
    audioExtensions.add(".m4a");
}

void StemDetector::setPatterns(const juce::Array<StemPattern>& patterns)
{
    stemPatterns = patterns;
}

juce::Array<StemPattern> StemDetector::getDefaultPatterns()
{
    juce::Array<StemPattern> defaults;
    
    // Underscore patterns (most common)
    defaults.add({ "Vocals", "_vocals" });
    defaults.add({ "Vocals", "_vocal" });
    defaults.add({ "Drums", "_drums" });
    defaults.add({ "Drums", "_drum" });
    defaults.add({ "Bass", "_bass" });
    defaults.add({ "Other", "_other" });
    defaults.add({ "Piano", "_piano" });
    defaults.add({ "Guitar", "_guitar" });
    defaults.add({ "Synth", "_synth" });
    defaults.add({ "Strings", "_strings" });
    
    // Parentheses patterns
    defaults.add({ "Vocals", " (Vocals)" });
    defaults.add({ "Vocals", " (Vocal)" });
    defaults.add({ "Drums", " (Drums)" });
    defaults.add({ "Drums", " (Drum)" });
    defaults.add({ "Bass", " (Bass)" });
    defaults.add({ "Other", " (Other)" });
    defaults.add({ "Piano", " (Piano)" });
    defaults.add({ "Guitar", " (Guitar)" });
    
    // Hyphen patterns
    defaults.add({ "Vocals", "-vocals" });
    defaults.add({ "Drums", "-drums" });
    defaults.add({ "Bass", "-bass" });
    defaults.add({ "Other", "-other" });
    
    return defaults;
}

bool StemDetector::isAudioFile(const juce::File& file) const
{
    juce::String ext = file.getFileExtension().toLowerCase();
    return audioExtensions.contains(ext);
}

juce::String StemDetector::detectStemType(const juce::String& filename) const
{
    juce::String lowerFilename = filename.toLowerCase();
    
    for (const auto& pattern : stemPatterns)
    {
        if (lowerFilename.contains(pattern.pattern.toLowerCase()))
            return pattern.stemType;
    }
    
    return "Unknown";
}

juce::String StemDetector::extractSongName(const juce::String& filename) const
{
    juce::String name = filename;
    
    // Remove file extension
    int lastDot = name.lastIndexOf(".");
    if (lastDot > 0)
        name = name.substring(0, lastDot);
    
    // Try to remove stem patterns to get the base song name
    juce::String lowerName = name.toLowerCase();
    
    for (const auto& pattern : stemPatterns)
    {
        juce::String lowerPattern = pattern.pattern.toLowerCase();
        int patternPos = lowerName.indexOf(lowerPattern);
        
        if (patternPos >= 0)
        {
            // Remove the pattern from the original (non-lowercased) string
            name = name.substring(0, patternPos) + name.substring(patternPos + pattern.pattern.length());
            break;
        }
    }
    
    return name.trim();
}

juce::Array<DetectedSong> StemDetector::scanDirectory(const juce::File& directory) const
{
    juce::Array<DetectedSong> songs;
    std::map<juce::String, DetectedSong> songMap;
    
    if (!directory.isDirectory())
        return songs;
    
    // Scan all audio files in the directory
    for (const auto& entry : juce::RangedDirectoryIterator(directory, false, "*", juce::File::findFiles))
    {
        juce::File file = entry.getFile();
        
        if (!isAudioFile(file))
            continue;
        
        juce::String filename = file.getFileName();
        juce::String songName = extractSongName(filename);
        juce::String stemType = detectStemType(filename);
        
        // Add to map
        auto& song = songMap[songName];
        song.songName = songName;
        song.stemFiles.add(file);
        song.stemTypes.add(stemType);
    }
    
    // Convert map to array
    for (auto& pair : songMap)
    {
        // Only include if there's more than one stem (otherwise it's not a multi-stem song)
        // Or include all detected audio files
        songs.add(std::move(pair.second));
    }
    
    // Sort by song name
    std::sort(songs.begin(), songs.end(), 
              [](const DetectedSong& a, const DetectedSong& b) {
                  return a.songName.compareIgnoreCase(b.songName) < 0;
              });
    
    return songs;
}

