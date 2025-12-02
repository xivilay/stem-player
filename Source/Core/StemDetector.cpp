#include "StemDetector.h"
#include <regex>

StemDetector::StemDetector()
{
    regexPatterns = getDefaultPatterns();
    
    audioExtensions.add(".mp3");
    audioExtensions.add(".wav");
    audioExtensions.add(".aiff");
    audioExtensions.add(".flac");
    audioExtensions.add(".ogg");
    audioExtensions.add(".m4a");
}

std::array<juce::String, NUM_STEM_TYPES> StemDetector::getDefaultPatterns()
{
    return {
        // Vocals: matches _vocal, _vocals, (Vocal), (Vocals), -vocal, -vocals (case insensitive)
        R"([\s_\-\(](vocals?)\)?)",
        // Drums: matches _drum, _drums, (Drum), (Drums), -drum, -drums
        R"([\s_\-\(](drums?)\)?)",
        // Bass: matches _bass, (Bass), -bass
        R"([\s_\-\(](bass)\)?)",
        // Guitar: matches _guitar, (Guitar), -guitar
        R"([\s_\-\(](guitar)\)?)",
        // Piano: matches _piano, (Piano), -piano, _keys, (Keys), -keys
        R"([\s_\-\(](piano|keys)\)?)",
        // Other: matches _other, (Other), -other, _inst, _instrumental
        R"([\s_\-\(](other|inst(rumental)?)\)?)"
    };
}

juce::String StemDetector::getStemTypeName(StemType type)
{
    switch (type)
    {
        case StemType::Vocals: return "Vocals";
        case StemType::Drums:  return "Drums";
        case StemType::Bass:   return "Bass";
        case StemType::Guitar: return "Guitar";
        case StemType::Piano:  return "Piano";
        case StemType::Other:  return "Other";
        default: return "Unknown";
    }
}

juce::String StemDetector::getStemTypeName(int index)
{
    if (index >= 0 && index < NUM_STEM_TYPES)
        return getStemTypeName(static_cast<StemType>(index));
    return "Unknown";
}

void StemDetector::setPatterns(const std::array<juce::String, NUM_STEM_TYPES>& patterns)
{
    regexPatterns = patterns;
}

bool StemDetector::isAudioFile(const juce::File& file) const
{
    juce::String ext = file.getFileExtension().toLowerCase();
    return audioExtensions.contains(ext);
}

bool StemDetector::matchesPattern(const juce::String& filename, const juce::String& pattern) const
{
    if (pattern.isEmpty())
        return false;
    
    try
    {
        std::regex regex(pattern.toStdString(), std::regex_constants::icase);
        return std::regex_search(filename.toStdString(), regex);
    }
    catch (const std::regex_error&)
    {
        // Invalid regex, try simple contains match as fallback
        return filename.toLowerCase().contains(pattern.toLowerCase());
    }
}

StemType StemDetector::detectStemType(const juce::String& filename) const
{
    // Check each stem type in order (excluding Other which is the fallback)
    for (int i = 0; i < NUM_STEM_TYPES - 1; ++i)
    {
        if (matchesPattern(filename, regexPatterns[i]))
            return static_cast<StemType>(i);
    }
    
    // Default to Other if no match
    return StemType::Other;
}

juce::String StemDetector::extractSongName(const juce::String& filename, StemType detectedType) const
{
    juce::String name = filename;
    
    // Remove file extension
    int lastDot = name.lastIndexOf(".");
    if (lastDot > 0)
        name = name.substring(0, lastDot);
    
    // Try to remove the stem type pattern from the filename
    const juce::String& pattern = regexPatterns[static_cast<int>(detectedType)];
    
    if (pattern.isNotEmpty())
    {
        try
        {
            std::regex regex(pattern.toStdString(), std::regex_constants::icase);
            std::string result = std::regex_replace(name.toStdString(), regex, "");
            name = juce::String(result);
        }
        catch (const std::regex_error&)
        {
            // Fallback: try to remove common patterns
        }
    }
    
    // Clean up trailing/leading separators
    name = name.trim();
    while (name.endsWithChar('_') || name.endsWithChar('-') || name.endsWithChar(' '))
        name = name.dropLastCharacters(1).trim();
    while (name.startsWithChar('_') || name.startsWithChar('-'))
        name = name.substring(1).trim();
    
    return name;
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
        StemType stemType = detectStemType(filename);
        juce::String songName = extractSongName(filename, stemType);
        
        if (songName.isEmpty())
            continue;
        
        // Add to map
        auto& song = songMap[songName];
        song.songName = songName;
        
        int stemIndex = static_cast<int>(stemType);
        song.stemFiles[stemIndex] = file;
        song.stemFound[stemIndex] = true;
    }
    
    // Convert map to array, only include songs with at least one stem
    for (auto& pair : songMap)
    {
        bool hasAnyStem = false;
        for (int i = 0; i < NUM_STEM_TYPES; ++i)
        {
            if (pair.second.stemFound[i])
            {
                hasAnyStem = true;
                break;
            }
        }
        
        if (hasAnyStem)
            songs.add(std::move(pair.second));
    }
    
    // Sort by song name
    std::sort(songs.begin(), songs.end(), 
              [](const DetectedSong& a, const DetectedSong& b) {
                  return a.songName.compareIgnoreCase(b.songName) < 0;
              });
    
    return songs;
}
