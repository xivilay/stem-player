#pragma once

#include <JuceHeader.h>
#include "StemTrack.h"

class StemEngine
{
public:
    StemEngine();
    ~StemEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    void loadSong(const juce::String& name, const juce::Array<juce::File>& stemFiles);
    void unloadSong();
    
    void play();
    void pause();
    void stop();
    void togglePlayPause();
    
    bool isPlaying() const { return playing; }
    
    void setPosition(double positionInSeconds);
    void setPositionNormalized(double normalizedPosition);
    double getPositionInSeconds() const;
    double getPositionNormalized() const;
    double getTotalLengthInSeconds() const;
    
    const juce::String& getCurrentSongName() const { return currentSongName; }
    
    int getNumTracks() const { return static_cast<int>(tracks.size()); }
    StemTrack* getTrack(int index);
    
    void setTrackVolume(int trackIndex, float volume);
    float getTrackVolume(int trackIndex) const;
    
    void updateSoloState();

private:
    juce::AudioFormatManager formatManager;
    
    juce::String currentSongName;
    std::vector<std::unique_ptr<StemTrack>> tracks;
    
    std::atomic<bool> playing { false };
    std::atomic<int64_t> currentPosition { 0 };
    int64_t totalLengthInSamples { 0 };
    
    double currentSampleRate { 44100.0 };
    int currentBlockSize { 512 };
    
    juce::CriticalSection processLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemEngine)
};

