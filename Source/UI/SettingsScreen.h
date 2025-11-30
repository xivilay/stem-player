#pragma once

#include <JuceHeader.h>
#include "../Core/StemDetector.h"
#include "../Core/MidiLearnManager.h"

class StemPlayerAudioProcessor;
class StemPlayerAudioProcessorEditor;

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

// Stem regex pattern row
class StemPatternRow : public juce::Component,
                        public juce::TextEditor::Listener
{
public:
    StemPatternRow(int stemIndex, std::array<juce::String, 4>& patterns,
                   std::function<void()> onChanged);
    ~StemPatternRow() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void updateFromPatterns();
    void textEditorTextChanged(juce::TextEditor& editor) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;
    void textEditorFocusLost(juce::TextEditor& editor) override;

private:
    void applyTextValue();
    
    int stemIndex;
    std::array<juce::String, 4>& patterns;
    std::function<void()> onChange;
    
    juce::Label nameLabel;
    juce::TextEditor regexEditor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StemPatternRow)
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
    void resetPatternsToDefault();
    void savePatterns();
    void updateMidiRows();
    void updatePatternRows();
    
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
    
    // Stem patterns section
    juce::Label patternsSectionLabel;
    std::array<std::unique_ptr<StemPatternRow>, 4> patternRows;
    juce::TextButton resetPatternsButton;
    std::array<juce::String, 4> editingPatterns;
    
    // MIDI assignment section
    juce::Label midiSectionLabel;
    juce::Viewport midiViewport;
    juce::Component midiContainer;
    std::vector<std::unique_ptr<MidiAssignmentRow>> midiRows;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsScreen)
};
