#include "StemEngine.h"

StemEngine::StemEngine()
{
    formatManager.registerBasicFormats();
}

StemEngine::~StemEngine()
{
    unloadSong();
}

void StemEngine::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    currentBlockSize = samplesPerBlock;
    
    for (auto& track : tracks)
        track->prepareToPlay(sampleRate, samplesPerBlock);
}

void StemEngine::releaseResources()
{
    for (auto& track : tracks)
        track->releaseResources();
}

void StemEngine::processBlock(juce::AudioBuffer<float>& buffer)
{
    juce::ScopedLock sl(processLock);
    
    buffer.clear();
    
    if (tracks.empty() || !playing)
        return;
    
    int64_t pos = currentPosition.load();
    
    if (pos >= totalLengthInSamples)
    {
        playing = false;
        currentPosition = 0;
        return;
    }
    
    // Check if any track is soloed
    bool anySolo = false;
    for (auto& track : tracks)
    {
        if (track->isSolo())
        {
            anySolo = true;
            break;
        }
    }
    
    // Temporary buffer for each track
    juce::AudioBuffer<float> trackBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    
    for (auto& track : tracks)
    {
        if (!track->isLoaded())
            continue;
        
        // Skip if another track is soloed and this one isn't
        if (anySolo && !track->isSolo())
            continue;
        
        trackBuffer.clear();
        track->getNextAudioBlock(trackBuffer, pos);
        
        // Mix into main buffer
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            buffer.addFrom(ch, 0, trackBuffer, 
                          ch < trackBuffer.getNumChannels() ? ch : 0,
                          0, buffer.getNumSamples());
        }
    }
    
    // Advance position
    currentPosition = pos + buffer.getNumSamples();
}

void StemEngine::loadSong(const juce::String& name, const juce::Array<juce::File>& stemFiles)
{
    juce::ScopedLock sl(processLock);
    
    stop();
    unloadSong();
    
    currentSongName = name;
    
    for (const auto& file : stemFiles)
    {
        // Extract stem type from filename
        juce::String stemType = file.getFileNameWithoutExtension();
        
        // Remove song name prefix if present
        if (stemType.startsWithIgnoreCase(name))
        {
            stemType = stemType.substring(name.length());
            
            // Remove common separators
            while (stemType.isNotEmpty() && 
                   (stemType[0] == '_' || stemType[0] == '-' || stemType[0] == ' '))
            {
                stemType = stemType.substring(1);
            }
        }
        
        // Clean up parentheses
        if (stemType.startsWithChar('(') && stemType.endsWithChar(')'))
            stemType = stemType.substring(1, stemType.length() - 1);
        
        if (stemType.isEmpty())
            stemType = "Track " + juce::String(tracks.size() + 1);
        
        auto track = std::make_unique<StemTrack>(file, stemType);
        
        if (track->loadFile(formatManager))
        {
            track->prepareToPlay(currentSampleRate, currentBlockSize);
            
            // Update total length
            int64_t trackLength = track->getTotalLengthInSamples();
            if (trackLength > totalLengthInSamples)
                totalLengthInSamples = trackLength;
            
            tracks.push_back(std::move(track));
        }
    }
}

void StemEngine::unloadSong()
{
    juce::ScopedLock sl(processLock);
    
    playing = false;
    currentPosition = 0;
    totalLengthInSamples = 0;
    currentSongName.clear();
    tracks.clear();
}

void StemEngine::play()
{
    if (!tracks.empty())
        playing = true;
}

void StemEngine::pause()
{
    playing = false;
}

void StemEngine::stop()
{
    playing = false;
    currentPosition = 0;
}

void StemEngine::togglePlayPause()
{
    if (playing)
        pause();
    else
        play();
}

void StemEngine::setPosition(double positionInSeconds)
{
    if (currentSampleRate > 0)
    {
        int64_t newPos = static_cast<int64_t>(positionInSeconds * currentSampleRate);
        currentPosition = juce::jlimit<int64_t>(0, totalLengthInSamples, newPos);
    }
}

void StemEngine::setPositionNormalized(double normalizedPosition)
{
    double clampedPos = juce::jlimit(0.0, 1.0, normalizedPosition);
    int64_t newPos = static_cast<int64_t>(clampedPos * totalLengthInSamples);
    currentPosition = newPos;
}

double StemEngine::getPositionInSeconds() const
{
    if (currentSampleRate > 0)
        return static_cast<double>(currentPosition.load()) / currentSampleRate;
    return 0.0;
}

double StemEngine::getPositionNormalized() const
{
    if (totalLengthInSamples > 0)
        return static_cast<double>(currentPosition.load()) / totalLengthInSamples;
    return 0.0;
}

double StemEngine::getTotalLengthInSeconds() const
{
    if (currentSampleRate > 0)
        return static_cast<double>(totalLengthInSamples) / currentSampleRate;
    return 0.0;
}

StemTrack* StemEngine::getTrack(int index)
{
    if (index >= 0 && index < static_cast<int>(tracks.size()))
        return tracks[static_cast<size_t>(index)].get();
    return nullptr;
}

void StemEngine::setTrackVolume(int trackIndex, float volume)
{
    if (auto* track = getTrack(trackIndex))
        track->setVolume(volume);
}

float StemEngine::getTrackVolume(int trackIndex) const
{
    if (trackIndex >= 0 && trackIndex < static_cast<int>(tracks.size()))
        return tracks[static_cast<size_t>(trackIndex)]->getVolume();
    return 0.0f;
}

void StemEngine::updateSoloState()
{
    // This method can be called to refresh solo state if needed
    // The actual solo logic is in processBlock
}

