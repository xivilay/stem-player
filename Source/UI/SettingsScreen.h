#pragma once

#include <JuceHeader.h>
#include "../Core/StemDetector.h"

class StemPlayerAudioProcessor;
class StemPlayerAudioProcessorEditor;

class PatternListModel : public juce::TableListBoxModel
{
public:
    PatternListModel() = default;
    
    void setPatterns(juce::Array<StemPattern>& patterns);
    juce::Array<StemPattern>& getPatterns() { return *patternsRef; }
    
    int getNumRows() override;
    void paintRowBackground(juce::Graphics& g, int rowNumber, int width, int height,
                            bool rowIsSelected) override;
    void paintCell(juce::Graphics& g, int rowNumber, int columnId, int width, int height,
                   bool rowIsSelected) override;
    juce::Component* refreshComponentForCell(int rowNumber, int columnId, bool isRowSelected,
                                              juce::Component* existingComponentToUpdate) override;
    
    std::function<void()> onPatternsChanged;

private:
    juce::Array<StemPattern>* patternsRef { nullptr };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PatternListModel)
};

class SettingsScreen : public juce::Component
{
public:
    SettingsScreen(StemPlayerAudioProcessor& processor, 
                   StemPlayerAudioProcessorEditor& editor);
    ~SettingsScreen() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    void browseForDefaultFolder();
    void addPattern();
    void removeSelectedPattern();
    void resetPatternsToDefault();
    void saveSettings();
    
    StemPlayerAudioProcessor& audioProcessor;
    StemPlayerAudioProcessorEditor& editor;
    
    juce::Label titleLabel;
    juce::TextButton backButton;
    
    // Default folder section
    juce::Label folderSectionLabel;
    juce::Label defaultFolderLabel;
    juce::TextButton browseFolderButton;
    juce::TextButton clearFolderButton;
    
    // Patterns section
    juce::Label patternsSectionLabel;
    juce::TableListBox patternsTable;
    PatternListModel patternListModel;
    juce::TextButton addPatternButton;
    juce::TextButton removePatternButton;
    juce::TextButton resetPatternsButton;
    
    // Local copy of patterns for editing
    juce::Array<StemPattern> editingPatterns;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsScreen)
};

