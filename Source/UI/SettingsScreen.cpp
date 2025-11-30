#include "SettingsScreen.h"
#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include "LookAndFeel.h"

// MidiAssignmentRow implementation
MidiAssignmentRow::MidiAssignmentRow(MidiControlType type, MidiLearnManager& manager)
    : controlType(type), midiManager(manager)
{
    nameLabel.setText(MidiLearnManager::getControlName(type), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(13.0f));
    nameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    ccEditor.setFont(juce::Font(13.0f));
    ccEditor.setJustification(juce::Justification::centred);
    ccEditor.setInputRestrictions(3, "0123456789");
    ccEditor.setColour(juce::TextEditor::backgroundColourId, StemPlayerLookAndFeel::backgroundLight);
    ccEditor.setColour(juce::TextEditor::textColourId, StemPlayerLookAndFeel::textPrimary);
    ccEditor.setColour(juce::TextEditor::outlineColourId, StemPlayerLookAndFeel::backgroundLight);
    ccEditor.addListener(this);
    addAndMakeVisible(ccEditor);
    
    learnButton.setButtonText("Learn");
    learnButton.onClick = [this]() {
        if (midiManager.isLearning() && midiManager.getLearningControlType() == controlType)
        {
            midiManager.stopLearning();
            learnButton.setButtonText("Learn");
        }
        else
        {
            midiManager.startLearning(controlType);
            learnButton.setButtonText("...");
        }
    };
    addAndMakeVisible(learnButton);
    
    clearButton.setButtonText("X");
    clearButton.onClick = [this]() {
        midiManager.removeMapping(controlType);
        ccEditor.setText("", juce::dontSendNotification);
    };
    addAndMakeVisible(clearButton);
    
    updateFromManager();
}

MidiAssignmentRow::~MidiAssignmentRow()
{
    ccEditor.removeListener(this);
}

void MidiAssignmentRow::paint(juce::Graphics& g)
{
    g.setColour(StemPlayerLookAndFeel::backgroundMedium);
    g.fillRect(getLocalBounds().toFloat());
}

void MidiAssignmentRow::resized()
{
    auto bounds = getLocalBounds().reduced(6, 2);
    
    nameLabel.setBounds(bounds.removeFromLeft(120));
    bounds.removeFromLeft(8);
    
    clearButton.setBounds(bounds.removeFromRight(30));
    bounds.removeFromRight(4);
    learnButton.setBounds(bounds.removeFromRight(50));
    bounds.removeFromRight(8);
    
    ccEditor.setBounds(bounds.removeFromRight(50));
}

void MidiAssignmentRow::updateFromManager()
{
    int cc = midiManager.getMappedCC(controlType);
    
    if (cc >= 0)
        ccEditor.setText(juce::String(cc), juce::dontSendNotification);
    else
        ccEditor.setText("", juce::dontSendNotification);
    
    if (midiManager.isLearning() && midiManager.getLearningControlType() == controlType)
        learnButton.setButtonText("...");
    else
        learnButton.setButtonText("Learn");
}

void MidiAssignmentRow::textEditorTextChanged(juce::TextEditor&) {}

void MidiAssignmentRow::textEditorReturnKeyPressed(juce::TextEditor&)
{
    applyTextValue();
}

void MidiAssignmentRow::textEditorFocusLost(juce::TextEditor&)
{
    applyTextValue();
}

void MidiAssignmentRow::applyTextValue()
{
    juce::String text = ccEditor.getText().trim();
    
    if (text.isEmpty())
    {
        midiManager.removeMapping(controlType);
    }
    else
    {
        int cc = text.getIntValue();
        if (cc >= 0 && cc <= 127)
            midiManager.setMapping(controlType, cc);
        else
            updateFromManager();
    }
}

// StemPatternRow implementation
StemPatternRow::StemPatternRow(int index, std::array<juce::String, 4>& pats,
                                std::function<void()> onChanged)
    : stemIndex(index), patterns(pats), onChange(onChanged)
{
    nameLabel.setText(StemDetector::getStemTypeName(index), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(13.0f, juce::Font::bold));
    nameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    regexEditor.setFont(juce::Font(12.0f));
    regexEditor.setColour(juce::TextEditor::backgroundColourId, StemPlayerLookAndFeel::backgroundLight);
    regexEditor.setColour(juce::TextEditor::textColourId, StemPlayerLookAndFeel::textPrimary);
    regexEditor.setColour(juce::TextEditor::outlineColourId, StemPlayerLookAndFeel::backgroundLight);
    regexEditor.addListener(this);
    addAndMakeVisible(regexEditor);
    
    updateFromPatterns();
}

StemPatternRow::~StemPatternRow()
{
    regexEditor.removeListener(this);
}

void StemPatternRow::paint(juce::Graphics& g)
{
    g.setColour(StemPlayerLookAndFeel::backgroundMedium);
    g.fillRect(getLocalBounds().toFloat());
}

void StemPatternRow::resized()
{
    auto bounds = getLocalBounds().reduced(6, 2);
    
    nameLabel.setBounds(bounds.removeFromLeft(70));
    bounds.removeFromLeft(8);
    regexEditor.setBounds(bounds);
}

void StemPatternRow::updateFromPatterns()
{
    if (stemIndex >= 0 && stemIndex < 4)
        regexEditor.setText(patterns[stemIndex], juce::dontSendNotification);
}

void StemPatternRow::textEditorTextChanged(juce::TextEditor&) {}

void StemPatternRow::textEditorReturnKeyPressed(juce::TextEditor&)
{
    applyTextValue();
}

void StemPatternRow::textEditorFocusLost(juce::TextEditor&)
{
    applyTextValue();
}

void StemPatternRow::applyTextValue()
{
    if (stemIndex >= 0 && stemIndex < 4)
    {
        patterns[stemIndex] = regexEditor.getText();
        if (onChange)
            onChange();
    }
}

// SettingsScreen implementation
SettingsScreen::SettingsScreen(StemPlayerAudioProcessor& processor, 
                                StemPlayerAudioProcessorEditor& ed)
    : audioProcessor(processor), editor(ed)
{
    // Load current patterns for editing
    editingPatterns = audioProcessor.getAppSettings().getStemRegexPatterns();
    
    // Title
    titleLabel.setText("Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // Back button
    backButton.setButtonText("< Back");
    backButton.onClick = [this]() {
        editor.showScreen(StemPlayerAudioProcessor::Screen::Selection);
    };
    addAndMakeVisible(backButton);
    
    // Default folder section
    folderSectionLabel.setText("Default Folder", juce::dontSendNotification);
    folderSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    folderSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(folderSectionLabel);
    
    defaultFolderLabel.setFont(juce::Font(12.0f));
    defaultFolderLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    defaultFolderLabel.setText(audioProcessor.getAppSettings().getDefaultFolder().isEmpty() 
                               ? "Not set" : audioProcessor.getAppSettings().getDefaultFolder(),
                               juce::dontSendNotification);
    addAndMakeVisible(defaultFolderLabel);
    
    browseFolderButton.setButtonText("Browse");
    browseFolderButton.onClick = [this]() { browseForDefaultFolder(); };
    addAndMakeVisible(browseFolderButton);
    
    clearFolderButton.setButtonText("Clear");
    clearFolderButton.onClick = [this]() {
        audioProcessor.getAppSettings().setDefaultFolder("");
        defaultFolderLabel.setText("Not set", juce::dontSendNotification);
    };
    addAndMakeVisible(clearFolderButton);
    
    // Display section
    displaySectionLabel.setText("Display", juce::dontSendNotification);
    displaySectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    displaySectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(displaySectionLabel);
    
    separateChannelsToggle.setButtonText("Show separate L/R channels");
    separateChannelsToggle.setColour(juce::ToggleButton::textColourId, StemPlayerLookAndFeel::textPrimary);
    separateChannelsToggle.setColour(juce::ToggleButton::tickColourId, StemPlayerLookAndFeel::accentPrimary);
    separateChannelsToggle.setToggleState(audioProcessor.getAppSettings().getShowSeparateChannels(), 
                                          juce::dontSendNotification);
    separateChannelsToggle.onClick = [this]() {
        audioProcessor.getAppSettings().setShowSeparateChannels(separateChannelsToggle.getToggleState());
    };
    addAndMakeVisible(separateChannelsToggle);
    
    // Stem patterns section
    patternsSectionLabel.setText("Stem Detection (Regex)", juce::dontSendNotification);
    patternsSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    patternsSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(patternsSectionLabel);
    
    for (int i = 0; i < 4; ++i)
    {
        patternRows[i] = std::make_unique<StemPatternRow>(i, editingPatterns, 
                                                          [this]() { savePatterns(); });
        addAndMakeVisible(patternRows[i].get());
    }
    
    resetPatternsButton.setButtonText("Reset to Defaults");
    resetPatternsButton.onClick = [this]() { resetPatternsToDefault(); };
    addAndMakeVisible(resetPatternsButton);
    
    // MIDI assignment section
    midiSectionLabel.setText("MIDI Control", juce::dontSendNotification);
    midiSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    midiSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(midiSectionLabel);
    
    // Create MIDI assignment rows
    for (int i = 0; i < MidiLearnManager::getNumControls(); ++i)
    {
        auto controlType = static_cast<MidiControlType>(i);
        auto row = std::make_unique<MidiAssignmentRow>(controlType, audioProcessor.getMidiLearnManager());
        midiContainer.addAndMakeVisible(row.get());
        midiRows.push_back(std::move(row));
    }
    
    midiViewport.setViewedComponent(&midiContainer, false);
    midiViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(midiViewport);
    
    // Set up callback for MIDI learn updates
    audioProcessor.getMidiLearnManager().onMappingChanged = [this](MidiControlType, int) {
        updateMidiRows();
    };
}

SettingsScreen::~SettingsScreen()
{
    audioProcessor.getMidiLearnManager().onMappingChanged = nullptr;
}

void SettingsScreen::paint(juce::Graphics& g)
{
    g.fillAll(StemPlayerLookAndFeel::backgroundDark);
}

void SettingsScreen::resized()
{
    auto bounds = getLocalBounds().reduced(20);
    
    // Header row
    auto header = bounds.removeFromTop(36);
    backButton.setBounds(header.removeFromLeft(70));
    header.removeFromLeft(15);
    titleLabel.setBounds(header);
    
    bounds.removeFromTop(15);
    
    // Default folder section
    folderSectionLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);
    
    auto folderRow = bounds.removeFromTop(28);
    clearFolderButton.setBounds(folderRow.removeFromRight(50));
    folderRow.removeFromRight(6);
    browseFolderButton.setBounds(folderRow.removeFromRight(60));
    folderRow.removeFromRight(8);
    defaultFolderLabel.setBounds(folderRow);
    
    bounds.removeFromTop(12);
    
    // Display section
    displaySectionLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);
    separateChannelsToggle.setBounds(bounds.removeFromTop(24));
    
    bounds.removeFromTop(12);
    
    // Stem patterns section
    patternsSectionLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);
    
    for (int i = 0; i < 4; ++i)
    {
        patternRows[i]->setBounds(bounds.removeFromTop(28));
        bounds.removeFromTop(2);
    }
    
    resetPatternsButton.setBounds(bounds.removeFromTop(28).removeFromLeft(120));
    
    bounds.removeFromTop(12);
    
    // MIDI section
    midiSectionLabel.setBounds(bounds.removeFromTop(20));
    bounds.removeFromTop(4);
    
    midiViewport.setBounds(bounds);
    
    int midiRowHeight = 30;
    int midiTotalHeight = static_cast<int>(midiRows.size()) * (midiRowHeight + 2);
    midiContainer.setSize(midiViewport.getWidth() - 12, midiTotalHeight);
    
    int y = 0;
    for (auto& row : midiRows)
    {
        row->setBounds(0, y, midiContainer.getWidth(), midiRowHeight);
        y += midiRowHeight + 2;
    }
}

void SettingsScreen::visibilityChanged()
{
    if (isVisible())
    {
        editingPatterns = audioProcessor.getAppSettings().getStemRegexPatterns();
        updatePatternRows();
        updateMidiRows();
    }
}

void SettingsScreen::updateMidiRows()
{
    for (auto& row : midiRows)
        row->updateFromManager();
}

void SettingsScreen::updatePatternRows()
{
    for (int i = 0; i < 4; ++i)
        patternRows[i]->updateFromPatterns();
}

void SettingsScreen::browseForDefaultFolder()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Default Stems Folder", juce::File(), "", true);
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectDirectories,
                         [this, chooser](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.isDirectory())
        {
            audioProcessor.getAppSettings().setDefaultFolder(result.getFullPathName());
            defaultFolderLabel.setText(result.getFullPathName(), juce::dontSendNotification);
        }
    });
}

void SettingsScreen::resetPatternsToDefault()
{
    editingPatterns = StemDetector::getDefaultPatterns();
    updatePatternRows();
    savePatterns();
}

void SettingsScreen::savePatterns()
{
    audioProcessor.getAppSettings().setStemRegexPatterns(editingPatterns);
}
