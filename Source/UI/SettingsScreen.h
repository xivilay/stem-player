#pragma once

#include <JuceHeader.h>
#include "../Core/StemDetector.h"
#include "../Core/MidiLearnManager.h"

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

// Individual MIDI assignment row
class MidiAssignmentRow : public juce::Component,
                           public juce::TextEditor::Listener
{
public:
    MidiAssignmentRow(MidiControlType controlType, MidiLearnManager& manager);
    ~MidiAssignmentRow() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateFromManager();
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;

private:
    void applyTextValue();
    
    MidiControlType controlType;
    MidiLearnManager& midiManager;
    
    juce::Label nameLabel;
    juce::TextEditor ccEditor;
    juce::TextButton learnButton;
    juce::TextButton clearButton;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MidiAssignmentRow)
};

class SettingsScreen : public juce::Component
{
public:
    SettingsScreen(StemPlayerAudioProcessor& processor, 
                   StemPlayerAudioProcessorEditor& editor);
    ~SettingsScreen() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;

private:
    void browseForDefaultFolder();
    void addPattern();
    void removeSelectedPattern();
    void resetPatternsToDefault();
    void saveSettings();
    void updateMidiRows();
    
    StemPlayerAudioProcessor& audioProcessor;
    StemPlayerAudioProcessorEditor& editor;
    
    juce::Label titleLabel;
    juce::TextButton backButton;
    
    // Default folder section
    juce::Label folderSectionLabel;
    juce::Label defaultFolderLabel;
    juce::TextButton browseFolderButton;
    juce::TextButton clearFolderButton;
    
    // Display section
    juce::Label displaySectionLabel;
    juce::ToggleButton separateChannelsToggle;
    
    // MIDI assignment section
    juce::Label midiSectionLabel;
    juce::Viewport midiViewport;
    juce::Component midiContainer;
    std::vector<std::unique_ptr<MidiAssignmentRow>> midiRows;
    
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
