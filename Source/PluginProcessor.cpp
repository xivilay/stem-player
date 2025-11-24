#include "PluginProcessor.h"
#include "PluginEditor.h"

StemPlayerAudioProcessor::StemPlayerAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
    appSettings.loadSettings();
    
    if (appSettings.getDefaultFolder().isNotEmpty())
        currentScreen = Screen::Selection;
}

StemPlayerAudioProcessor::~StemPlayerAudioProcessor()
{
}

const juce::String StemPlayerAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool StemPlayerAudioProcessor::acceptsMidi() const
{
    return true;
}

bool StemPlayerAudioProcessor::producesMidi() const
{
    return false;
}

bool StemPlayerAudioProcessor::isMidiEffect() const
{
    return false;
}

double StemPlayerAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int StemPlayerAudioProcessor::getNumPrograms()
{
    return 1;
}

int StemPlayerAudioProcessor::getCurrentProgram()
{
    return 0;
}

void StemPlayerAudioProcessor::setCurrentProgram(int /*index*/)
{
}

const juce::String StemPlayerAudioProcessor::getProgramName(int /*index*/)
{
    return {};
}

void StemPlayerAudioProcessor::changeProgramName(int /*index*/, const juce::String& /*newName*/)
{
}

void StemPlayerAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    stemEngine.prepareToPlay(sampleRate, samplesPerBlock);
}

void StemPlayerAudioProcessor::releaseResources()
{
    stemEngine.releaseResources();
}

bool StemPlayerAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void StemPlayerAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // Process MIDI for learn
    midiLearnManager.processMidiMessages(midiMessages, stemEngine);

    // Clear output
    for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Process stems
    stemEngine.processBlock(buffer);
}

bool StemPlayerAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* StemPlayerAudioProcessor::createEditor()
{
    return new StemPlayerAudioProcessorEditor(*this);
}

void StemPlayerAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    juce::ValueTree state("StemPlayerState");
    
    // Save MIDI mappings
    auto midiMappings = midiLearnManager.getStateAsValueTree();
    state.addChild(midiMappings, -1, nullptr);
    
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void StemPlayerAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    
    if (xmlState != nullptr)
    {
        auto state = juce::ValueTree::fromXml(*xmlState);
        
        auto midiMappings = state.getChildWithName("MidiMappings");
        if (midiMappings.isValid())
            midiLearnManager.loadStateFromValueTree(midiMappings);
    }
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new StemPlayerAudioProcessor();
}

