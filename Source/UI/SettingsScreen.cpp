#include "SettingsScreen.h"
#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include "LookAndFeel.h"

#if JucePlugin_Build_Standalone
#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>
#endif

// AudioSettingsPanel implementation
AudioSettingsPanel::AudioSettingsPanel(juce::AudioDeviceManager& deviceManager,
                                       std::function<void()> onClose)
    : closeCallback(onClose)
{
    // Title
    titleLabel.setText("Audio Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(20.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // Close button
    closeButton.setButtonText("Done");
    closeButton.onClick = [this]() {
        if (closeCallback)
            closeCallback();
    };
    addAndMakeVisible(closeButton);
    
    // Audio device selector
    deviceSelector = std::make_unique<juce::AudioDeviceSelectorComponent>(
        deviceManager,
        0, 2,    // min/max input channels
        0, 2,    // min/max output channels
        false,   // show MIDI input options
        false,   // show MIDI output options
        false,   // show channels as stereo pairs
        false    // hide advanced options
    );
    addAndMakeVisible(deviceSelector.get());
}

void AudioSettingsPanel::paint(juce::Graphics& g)
{
    g.fillAll(StemPlayerLookAndFeel::backgroundDark);
    
    // Draw border at top
    g.setColour(StemPlayerLookAndFeel::backgroundLight);
    g.fillRect(0, 0, getWidth(), 1);
}

void AudioSettingsPanel::resized()
{
    auto bounds = getLocalBounds();
    
    // Header
    auto header = bounds.removeFromTop(50).reduced(15, 10);
    closeButton.setBounds(header.removeFromRight(70));
    header.removeFromRight(10);
    titleLabel.setBounds(header);
    
    // Device selector takes remaining space
    bounds.reduce(10, 5);
    deviceSelector->setBounds(bounds);
}

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
    
    // Back button (stays in main view, not scrollable)
    backButton.onClick = [this]() {
        editor.showScreen(StemPlayerAudioProcessor::Screen::Selection);
    };
    addAndMakeVisible(backButton);
    
    // Title (stays in main view, not scrollable)
    titleLabel.setText("Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);
    
    // Audio settings button (in header, right side)
    audioSettingsButton.setButtonText("Audio Settings...");
    audioSettingsButton.onClick = [this]() { showAudioSettings(); };
    addAndMakeVisible(audioSettingsButton);
    
    // Scrollable content viewport
    contentViewport.setViewedComponent(&contentContainer, false);
    contentViewport.setScrollBarsShown(true, false);
    addAndMakeVisible(contentViewport);
    
    // Default folder section
    folderSectionLabel.setText("Default Folder", juce::dontSendNotification);
    folderSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    folderSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    contentContainer.addAndMakeVisible(folderSectionLabel);
    
    defaultFolderLabel.setFont(juce::Font(12.0f));
    defaultFolderLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    defaultFolderLabel.setText(audioProcessor.getAppSettings().getDefaultFolder().isEmpty() 
                               ? "Not set" : audioProcessor.getAppSettings().getDefaultFolder(),
                               juce::dontSendNotification);
    contentContainer.addAndMakeVisible(defaultFolderLabel);
    
    browseFolderButton.setButtonText("Browse");
    browseFolderButton.onClick = [this]() { browseForDefaultFolder(); };
    contentContainer.addAndMakeVisible(browseFolderButton);
    
    clearFolderButton.setButtonText("Clear");
    clearFolderButton.onClick = [this]() {
        audioProcessor.getAppSettings().setDefaultFolder("");
        defaultFolderLabel.setText("Not set", juce::dontSendNotification);
    };
    contentContainer.addAndMakeVisible(clearFolderButton);
    
    // Display section
    displaySectionLabel.setText("Display", juce::dontSendNotification);
    displaySectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    displaySectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    contentContainer.addAndMakeVisible(displaySectionLabel);
    
    separateChannelsToggle.setButtonText("Show separate L/R channels");
    separateChannelsToggle.setColour(juce::ToggleButton::textColourId, StemPlayerLookAndFeel::textPrimary);
    separateChannelsToggle.setColour(juce::ToggleButton::tickColourId, StemPlayerLookAndFeel::accentPrimary);
    separateChannelsToggle.setToggleState(audioProcessor.getAppSettings().getShowSeparateChannels(), 
                                          juce::dontSendNotification);
    separateChannelsToggle.onClick = [this]() {
        audioProcessor.getAppSettings().setShowSeparateChannels(separateChannelsToggle.getToggleState());
    };
    contentContainer.addAndMakeVisible(separateChannelsToggle);
    
    // Stem patterns section
    patternsSectionLabel.setText("Stem Detection (Regex)", juce::dontSendNotification);
    patternsSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    patternsSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    contentContainer.addAndMakeVisible(patternsSectionLabel);
    
    for (int i = 0; i < 4; ++i)
    {
        patternRows[i] = std::make_unique<StemPatternRow>(i, editingPatterns, 
                                                          [this]() { savePatterns(); });
        contentContainer.addAndMakeVisible(patternRows[i].get());
    }
    
    resetPatternsButton.setButtonText("Reset to Defaults");
    resetPatternsButton.onClick = [this]() { resetPatternsToDefault(); };
    contentContainer.addAndMakeVisible(resetPatternsButton);
    
    // MIDI assignment section
    midiSectionLabel.setText("MIDI Control", juce::dontSendNotification);
    midiSectionLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    midiSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    contentContainer.addAndMakeVisible(midiSectionLabel);
    
    // Create MIDI assignment rows
    for (int i = 0; i < MidiLearnManager::getNumControls(); ++i)
    {
        auto controlType = static_cast<MidiControlType>(i);
        auto row = std::make_unique<MidiAssignmentRow>(controlType, audioProcessor.getMidiLearnManager());
        contentContainer.addAndMakeVisible(row.get());
        midiRows.push_back(std::move(row));
    }
    
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
    
    // Header row (fixed, not scrollable)
    auto header = bounds.removeFromTop(36);
    backButton.setBounds(header.removeFromLeft(40));
    header.removeFromLeft(15);
    
    // Audio settings button on the right
    audioSettingsButton.setBounds(header.removeFromRight(120));
    header.removeFromRight(10);
    
    titleLabel.setBounds(header);
    
    bounds.removeFromTop(15);
    
    // Content viewport takes remaining space
    contentViewport.setBounds(bounds);
    
    // Layout the content
    layoutContent();
    
    // Resize audio settings panel if visible
    if (audioSettingsPanel != nullptr)
        audioSettingsPanel->setBounds(getLocalBounds());
}

void SettingsScreen::layoutContent()
{
    int contentWidth = contentViewport.getWidth() - 16;  // Account for scrollbar
    int y = 0;
    
    // Default folder section
    folderSectionLabel.setBounds(0, y, contentWidth, 20);
    y += 24;
    
    int folderRowHeight = 28;
    clearFolderButton.setBounds(contentWidth - 50, y, 50, folderRowHeight);
    browseFolderButton.setBounds(contentWidth - 50 - 6 - 60, y, 60, folderRowHeight);
    defaultFolderLabel.setBounds(0, y, contentWidth - 50 - 6 - 60 - 8, folderRowHeight);
    y += folderRowHeight + 16;
    
    // Display section
    displaySectionLabel.setBounds(0, y, contentWidth, 20);
    y += 24;
    separateChannelsToggle.setBounds(0, y, contentWidth, 24);
    y += 24 + 16;
    
    // Stem patterns section
    patternsSectionLabel.setBounds(0, y, contentWidth, 20);
    y += 24;
    
    for (int i = 0; i < 4; ++i)
    {
        patternRows[i]->setBounds(0, y, contentWidth, 28);
        y += 30;
    }
    
    resetPatternsButton.setBounds(0, y, 120, 28);
    y += 28 + 16;
    
    // MIDI section
    midiSectionLabel.setBounds(0, y, contentWidth, 20);
    y += 24;
    
    int midiRowHeight = 30;
    for (auto& row : midiRows)
    {
        row->setBounds(0, y, contentWidth, midiRowHeight);
        y += midiRowHeight + 2;
    }
    
    y += 20;  // Bottom padding
    
    // Set content container size
    contentContainer.setSize(contentWidth, y);
}

void SettingsScreen::visibilityChanged()
{
    if (isVisible())
    {
        editingPatterns = audioProcessor.getAppSettings().getStemRegexPatterns();
        updatePatternRows();
        updateMidiRows();
        
        // Reset scroll position
        contentViewport.setViewPosition(0, 0);
    }
    else
    {
        // Close audio settings panel when navigating away
        if (audioSettingsPanel != nullptr)
        {
            removeChildComponent(audioSettingsPanel.get());
            audioSettingsPanel.reset();
        }
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

void SettingsScreen::showAudioSettings()
{
#if JucePlugin_Build_Standalone
    if (auto* holder = StandalonePluginHolder::getInstance())
    {
        // Create custom audio settings panel with close button
        audioSettingsPanel = std::make_unique<AudioSettingsPanel>(
            holder->deviceManager,
            [this]() {
                // Close the panel
                if (audioSettingsPanel != nullptr)
                {
                    removeChildComponent(audioSettingsPanel.get());
                    audioSettingsPanel.reset();
                }
            }
        );
        
        // Add as overlay covering the whole screen
        addAndMakeVisible(audioSettingsPanel.get());
        audioSettingsPanel->setBounds(getLocalBounds());
        audioSettingsPanel->toFront(true);
    }
#else
    juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon,
                                           "Audio Settings",
                                           "Audio device settings are only available in standalone mode.\n"
                                           "When running as a plugin, audio settings are managed by the host.");
#endif
}
