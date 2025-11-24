#pragma once

#include <JuceHeader.h>

class StemTrack
{
public:
    StemTrack(const juce::File& file, const juce::String& stemType);
    ~StemTrack();

    bool loadFile(juce::AudioFormatManager& formatManager);
    void prepareToPlay(double sampleRate, int samplesPerBlock);
    void releaseResources();
    
    void getNextAudioBlock(juce::AudioBuffer<float>& buffer, int64_t startSample);
    
    const juce::String& getStemType() const { return stemType; }
    const juce::File& getFile() const { return file; }
    
    float getVolume() const { return volume; }
    void setVolume(float newVolume) { volume = juce::jlimit(0.0f, 1.0f, newVolume); }
    
    bool isMuted() const { return muted; }
    void setMuted(bool shouldMute) { muted = shouldMute; }
    
    bool isSolo() const { return solo; }
    void setSolo(bool shouldSolo) { solo = shouldSolo; }
    
    int64_t getTotalLengthInSamples() const;
    double getLengthInSeconds() const;
    double getSampleRate() const { return currentSampleRate; }
    
    juce::AudioThumbnail* getThumbnail() { return thumbnail.get(); }
    
    bool isLoaded() const { return loaded; }

private:
    juce::File file;
    juce::String stemType;
    
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::ResamplingAudioSource> resamplingSource;
    
    juce::AudioThumbnailCache thumbnailCache { 1 };
    std::unique_ptr<juce::AudioThumbnail> thumbnail;
    
    float volume { 1.0f };
    bool muted { false };
    bool solo { false };
    bool loaded { false };
    
    double currentSampleRate { 44100.0 };
    double fileSampleRate { 44100.0 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemTrack)
};

