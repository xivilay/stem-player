#include "MidiLearnManager.h"
#include "StemEngine.h"

MidiLearnManager::MidiLearnManager()
{
}

void MidiLearnManager::startLearning(int trackIndex)
{
    juce::ScopedLock sl(lock);
    learning = true;
    learningTrackIndex = trackIndex;
}

void MidiLearnManager::stopLearning()
{
    juce::ScopedLock sl(lock);
    learning = false;
    learningTrackIndex = -1;
}

void MidiLearnManager::processMidiMessages(const juce::MidiBuffer& midiMessages, StemEngine& engine)
{
    juce::ScopedLock sl(lock);
    
    for (const auto metadata : midiMessages)
    {
        auto message = metadata.getMessage();
        
        if (message.isController())
        {
            int cc = message.getControllerNumber();
            int channel = message.getChannel();
            float value = message.getControllerValue() / 127.0f;
            
            // If we're learning, capture this CC
            if (learning && learningTrackIndex >= 0)
            {
                setMapping(learningTrackIndex, cc, channel);
                learning = false;
                
                if (onMappingChanged)
                {
                    juce::MessageManager::callAsync([this]() {
                        if (onMappingChanged)
                            onMappingChanged();
                    });
                }
                
                learningTrackIndex = -1;
                continue;
            }
            
            // Apply mapped CC values
            for (const auto& mapping : mappings)
            {
                if (mapping.ccNumber == cc && 
                    (mapping.channel == -1 || mapping.channel == channel))
                {
                    engine.setTrackVolume(mapping.trackIndex, value);
                }
            }
        }
    }
}

void MidiLearnManager::setMapping(int trackIndex, int ccNumber, int channel)
{
    // Remove existing mapping for this track
    removeMapping(trackIndex);
    
    // Also remove any mapping using this CC (one CC per parameter)
    mappings.erase(
        std::remove_if(mappings.begin(), mappings.end(),
                       [ccNumber, channel](const MidiMapping& m) {
                           return m.ccNumber == ccNumber && 
                                  (m.channel == channel || m.channel == -1 || channel == -1);
                       }),
        mappings.end());
    
    MidiMapping newMapping;
    newMapping.trackIndex = trackIndex;
    newMapping.ccNumber = ccNumber;
    newMapping.channel = channel;
    
    mappings.push_back(newMapping);
}

void MidiLearnManager::removeMapping(int trackIndex)
{
    mappings.erase(
        std::remove_if(mappings.begin(), mappings.end(),
                       [trackIndex](const MidiMapping& m) {
                           return m.trackIndex == trackIndex;
                       }),
        mappings.end());
}

MidiMapping* MidiLearnManager::getMapping(int trackIndex)
{
    for (auto& mapping : mappings)
    {
        if (mapping.trackIndex == trackIndex)
            return &mapping;
    }
    return nullptr;
}

juce::ValueTree MidiLearnManager::getStateAsValueTree() const
{
    juce::ValueTree state("MidiMappings");
    
    for (const auto& mapping : mappings)
    {
        juce::ValueTree mappingTree("Mapping");
        mappingTree.setProperty("trackIndex", mapping.trackIndex, nullptr);
        mappingTree.setProperty("ccNumber", mapping.ccNumber, nullptr);
        mappingTree.setProperty("channel", mapping.channel, nullptr);
        state.addChild(mappingTree, -1, nullptr);
    }
    
    return state;
}

void MidiLearnManager::loadStateFromValueTree(const juce::ValueTree& state)
{
    juce::ScopedLock sl(lock);
    mappings.clear();
    
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto mappingTree = state.getChild(i);
        
        if (mappingTree.hasType("Mapping"))
        {
            MidiMapping mapping;
            mapping.trackIndex = mappingTree.getProperty("trackIndex", -1);
            mapping.ccNumber = mappingTree.getProperty("ccNumber", -1);
            mapping.channel = mappingTree.getProperty("channel", -1);
            
            if (mapping.trackIndex >= 0 && mapping.ccNumber >= 0)
                mappings.push_back(mapping);
        }
    }
}

