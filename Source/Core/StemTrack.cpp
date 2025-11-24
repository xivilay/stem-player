#include "StemTrack.h"

StemTrack::StemTrack(const juce::File& f, const juce::String& type)
    : file(f), stemType(type)
{
}

StemTrack::~StemTrack()
{
    releaseResources();
}

bool StemTrack::loadFile(juce::AudioFormatManager& formatManager)
{
    auto* reader = formatManager.createReaderFor(file);
    
    if (reader == nullptr)
        return false;
    
    fileSampleRate = reader->sampleRate;
    
    readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    resamplingSource = std::make_unique<juce::ResamplingAudioSource>(readerSource.get(), false, 2);
    
    // Create thumbnail
    thumbnail = std::make_unique<juce::AudioThumbnail>(512, formatManager, thumbnailCache);
    thumbnail->setSource(new juce::FileInputSource(file));
    
    loaded = true;
    return true;
}

void StemTrack::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    
    if (resamplingSource != nullptr)
    {
        resamplingSource->setResamplingRatio(fileSampleRate / sampleRate);
        resamplingSource->prepareToPlay(samplesPerBlock, sampleRate);
    }
}

void StemTrack::releaseResources()
{
    if (resamplingSource != nullptr)
        resamplingSource->releaseResources();
}

void StemTrack::getNextAudioBlock(juce::AudioBuffer<float>& buffer, int64_t startSample)
{
    if (!loaded || readerSource == nullptr || muted)
    {
        buffer.clear();
        return;
    }
    
    // Set position
    readerSource->setNextReadPosition(startSample);
    
    // Create audio source channel info
    juce::AudioSourceChannelInfo info(&buffer, 0, buffer.getNumSamples());
    
    // Get audio from resampling source
    resamplingSource->getNextAudioBlock(info);
    
    // Apply volume
    buffer.applyGain(volume);
}

int64_t StemTrack::getTotalLengthInSamples() const
{
    if (readerSource != nullptr)
        return readerSource->getTotalLength();
    return 0;
}

double StemTrack::getLengthInSeconds() const
{
    if (fileSampleRate > 0)
        return static_cast<double>(getTotalLengthInSamples()) / fileSampleRate;
    return 0.0;
}

