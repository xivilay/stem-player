#pragma once

#include <JuceHeader.h>
#include "StemTrack.h"
#include "StemDetector.h"

class StemEngine
{
public:
    StemEngine();
    ~StemEngine();

    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock(juce::AudioBuffer<float>& buffer);
    
    void loadSong(const juce::String& name, const std::array<juce::File, NUM_STEM_TYPES>& stemFiles,
                  const std::array<bool, NUM_STEM_TYPES>& stemFound);
    void unloadSong();
    
    void play();
    void pause();
    void stop();
    void togglePlayPause();
    void rewind();
    void fastForward();
    
    bool isPlaying() const { return playing; }
    
    void setSeekAmount(double seconds) { seekAmountSeconds = seconds; }
    
    void setPosition(double positionInSeconds);
    void setPositionNormalized(double normalizedPosition);
    double getPositionInSeconds() const;
    double getPositionNormalized() const;
    double getTotalLengthInSeconds() const;
    
    const juce::String& getCurrentSongName() const { return currentSongName; }
    
    static constexpr int getNumTracks() { return NUM_STEM_TYPES; }
    StemTrack* getTrack(int index);
    bool isTrackLoaded(int index) const;
    
    void setTrackVolume(int trackIndex, float volume);
    float getTrackVolume(int trackIndex) const;
    
    void updateSoloState();

private:
    juce::AudioFormatManager formatManager;
    
    juce::String currentSongName;
    std::array<std::unique_ptr<StemTrack>, NUM_STEM_TYPES> tracks;
    std::array<bool, NUM_STEM_TYPES> trackLoaded { false, false, false, false, false, false };
    
    std::atomic<bool> playing { false };
    std::atomic<int64_t> currentPosition { 0 };
    int64_t totalLengthInSamples { 0 };
    
    double currentSampleRate { 44100.0 };
    int currentBlockSize { 512 };
    double seekAmountSeconds { 5.0 };
    
    juce::CriticalSection processLock;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemEngine)
};

