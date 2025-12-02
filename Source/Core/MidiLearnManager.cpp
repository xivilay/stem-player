#include "MidiLearnManager.h"
#include "StemEngine.h"

MidiLearnManager::MidiLearnManager()
{
    // Initialize all mappings
    for (int i = 0; i < static_cast<int>(MidiControlType::NumControls); ++i)
    {
        mappings[i].controlType = static_cast<MidiControlType>(i);
        mappings[i].ccNumber = -1;
        mappings[i].channel = -1;
    }
}

juce::String MidiLearnManager::getControlName(MidiControlType type)
{
    switch (type)
    {
        case MidiControlType::Stem1Volume: return "Vocals Volume";
        case MidiControlType::Stem2Volume: return "Drums Volume";
        case MidiControlType::Stem3Volume: return "Bass Volume";
        case MidiControlType::Stem4Volume: return "Guitar Volume";
        case MidiControlType::Stem5Volume: return "Piano Volume";
        case MidiControlType::Stem6Volume: return "Other Volume";
        case MidiControlType::PlayPause:   return "Play/Pause";
        case MidiControlType::Stop:        return "Stop";
        case MidiControlType::Rewind:      return "Rewind";
        case MidiControlType::FastForward: return "Fast Forward";
        default: return "Unknown";
    }
}

void MidiLearnManager::startLearning(MidiControlType controlType)
{
    juce::ScopedLock sl(lock);
    learning = true;
    learningControlType = controlType;
}

void MidiLearnManager::stopLearning()
{
    juce::ScopedLock sl(lock);
    learning = false;
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
            if (learning)
            {
                setMapping(learningControlType, cc, channel);
                learning = false;
                
                if (onMappingChanged)
                {
                    MidiControlType capturedType = learningControlType;
                    int capturedCC = cc;
                    juce::MessageManager::callAsync([this, capturedType, capturedCC]() {
                        if (onMappingChanged)
                            onMappingChanged(capturedType, capturedCC);
                    });
                }
                continue;
            }
            
            // Apply mapped CC values
            for (const auto& mapping : mappings)
            {
                if (mapping.ccNumber == cc && 
                    (mapping.channel == -1 || mapping.channel == channel))
                {
                    switch (mapping.controlType)
                    {
                        case MidiControlType::Stem1Volume:
                            engine.setTrackVolume(0, value);
                            break;
                        case MidiControlType::Stem2Volume:
                            engine.setTrackVolume(1, value);
                            break;
                        case MidiControlType::Stem3Volume:
                            engine.setTrackVolume(2, value);
                            break;
                        case MidiControlType::Stem4Volume:
                            engine.setTrackVolume(3, value);
                            break;
                        case MidiControlType::Stem5Volume:
                            engine.setTrackVolume(4, value);
                            break;
                        case MidiControlType::Stem6Volume:
                            engine.setTrackVolume(5, value);
                            break;
                        case MidiControlType::PlayPause:
                            if (value > 0.5f)
                                engine.togglePlayPause();
                            break;
                        case MidiControlType::Stop:
                            if (value > 0.5f)
                                engine.stop();
                            break;
                        case MidiControlType::Rewind:
                            if (value > 0.5f)
                                engine.rewind();
                            break;
                        case MidiControlType::FastForward:
                            if (value > 0.5f)
                                engine.fastForward();
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}

void MidiLearnManager::setMapping(MidiControlType controlType, int ccNumber, int channel)
{
    int index = static_cast<int>(controlType);
    if (index >= 0 && index < static_cast<int>(MidiControlType::NumControls))
    {
        // Remove any other mapping using this CC (one CC per control)
        for (auto& mapping : mappings)
        {
            if (mapping.ccNumber == ccNumber && mapping.controlType != controlType)
            {
                mapping.ccNumber = -1;
                mapping.channel = -1;
            }
        }
        
        mappings[index].ccNumber = ccNumber;
        mappings[index].channel = channel;
    }
}

void MidiLearnManager::removeMapping(MidiControlType controlType)
{
    int index = static_cast<int>(controlType);
    if (index >= 0 && index < static_cast<int>(MidiControlType::NumControls))
    {
        mappings[index].ccNumber = -1;
        mappings[index].channel = -1;
    }
}

int MidiLearnManager::getMappedCC(MidiControlType controlType) const
{
    int index = static_cast<int>(controlType);
    if (index >= 0 && index < static_cast<int>(MidiControlType::NumControls))
        return mappings[index].ccNumber;
    return -1;
}

juce::ValueTree MidiLearnManager::getStateAsValueTree() const
{
    juce::ValueTree state("MidiMappings");
    
    for (const auto& mapping : mappings)
    {
        if (mapping.ccNumber >= 0)
        {
            juce::ValueTree mappingTree("Mapping");
            mappingTree.setProperty("controlType", static_cast<int>(mapping.controlType), nullptr);
            mappingTree.setProperty("ccNumber", mapping.ccNumber, nullptr);
            mappingTree.setProperty("channel", mapping.channel, nullptr);
            state.addChild(mappingTree, -1, nullptr);
        }
    }
    
    return state;
}

void MidiLearnManager::loadStateFromValueTree(const juce::ValueTree& state)
{
    juce::ScopedLock sl(lock);
    
    // Reset all mappings
    for (auto& mapping : mappings)
    {
        mapping.ccNumber = -1;
        mapping.channel = -1;
    }
    
    for (int i = 0; i < state.getNumChildren(); ++i)
    {
        auto mappingTree = state.getChild(i);
        
        if (mappingTree.hasType("Mapping"))
        {
            int controlTypeInt = mappingTree.getProperty("controlType", -1);
            int ccNumber = mappingTree.getProperty("ccNumber", -1);
            int channel = mappingTree.getProperty("channel", -1);
            
            if (controlTypeInt >= 0 && controlTypeInt < static_cast<int>(MidiControlType::NumControls) && ccNumber >= 0)
            {
                mappings[controlTypeInt].ccNumber = ccNumber;
                mappings[controlTypeInt].channel = channel;
            }
        }
    }
}
