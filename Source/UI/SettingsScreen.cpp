#include "SettingsScreen.h"
#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include "LookAndFeel.h"

// Editable cell component for the table
class EditableTextCell : public juce::Label
{
public:
    EditableTextCell(PatternListModel& model, int row, int column)
        : listModel(model), rowIndex(row), columnIndex(column)
    {
        setEditable(false, true, false);
        setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
        setColour(juce::Label::textWhenEditingColourId, StemPlayerLookAndFeel::textPrimary);
        setColour(juce::TextEditor::backgroundColourId, StemPlayerLookAndFeel::backgroundLight);
    }
    
    void textWasEdited() override
    {
        auto& patterns = listModel.getPatterns();
        if (rowIndex >= 0 && rowIndex < patterns.size())
        {
            if (columnIndex == 1)
                patterns.getReference(rowIndex).stemType = getText();
            else if (columnIndex == 2)
                patterns.getReference(rowIndex).pattern = getText();
            
            if (listModel.onPatternsChanged)
                listModel.onPatternsChanged();
        }
    }
    
private:
    PatternListModel& listModel;
    int rowIndex;
    int columnIndex;
};

// PatternListModel implementation
void PatternListModel::setPatterns(juce::Array<StemPattern>& patterns)
{
    patternsRef = &patterns;
}

int PatternListModel::getNumRows()
{
    return patternsRef ? patternsRef->size() : 0;
}

void PatternListModel::paintRowBackground(juce::Graphics& g, int /*rowNumber*/, 
                                           int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll(StemPlayerLookAndFeel::accentPrimary.withAlpha(0.2f));
    else
        g.fillAll(StemPlayerLookAndFeel::backgroundMedium);
}

void PatternListModel::paintCell(juce::Graphics& /*g*/, int /*rowNumber*/, int /*columnId*/,
                                  int /*width*/, int /*height*/, bool /*rowIsSelected*/)
{
    // Cells are painted by the component
}

juce::Component* PatternListModel::refreshComponentForCell(int rowNumber, int columnId,
                                                            bool /*isRowSelected*/,
                                                            juce::Component* existingComponentToUpdate)
{
    if (patternsRef == nullptr || rowNumber >= patternsRef->size())
    {
        delete existingComponentToUpdate;
        return nullptr;
    }
    
    auto* cell = dynamic_cast<EditableTextCell*>(existingComponentToUpdate);
    
    if (cell == nullptr)
    {
        delete existingComponentToUpdate;
        cell = new EditableTextCell(*this, rowNumber, columnId);
    }
    
    const auto& pattern = patternsRef->getReference(rowNumber);
    
    if (columnId == 1)
        cell->setText(pattern.stemType, juce::dontSendNotification);
    else if (columnId == 2)
        cell->setText(pattern.pattern, juce::dontSendNotification);
    
    return cell;
}

// MidiAssignmentRow implementation
MidiAssignmentRow::MidiAssignmentRow(MidiControlType type, MidiLearnManager& manager)
    : controlType(type), midiManager(manager)
{
    nameLabel.setText(MidiLearnManager::getControlName(type), juce::dontSendNotification);
    nameLabel.setFont(juce::Font(14.0f));
    nameLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    ccEditor.setFont(juce::Font(14.0f));
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
    
    clearButton.setButtonText("Clear");
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
    g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
}

void MidiAssignmentRow::resized()
{
    auto bounds = getLocalBounds().reduced(8, 4);
    
    nameLabel.setBounds(bounds.removeFromLeft(140));
    bounds.removeFromLeft(10);
    
    clearButton.setBounds(bounds.removeFromRight(60));
    bounds.removeFromRight(6);
    learnButton.setBounds(bounds.removeFromRight(60));
    bounds.removeFromRight(10);
    
    ccEditor.setBounds(bounds.removeFromRight(60));
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

void MidiAssignmentRow::textEditorTextChanged(juce::TextEditor& /*editor*/)
{
    // Don't apply while typing
}

void MidiAssignmentRow::textEditorReturnKeyPressed(juce::TextEditor& /*editor*/)
{
    applyTextValue();
}

void MidiAssignmentRow::textEditorFocusLost(juce::TextEditor& /*editor*/)
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
        {
            midiManager.setMapping(controlType, cc);
        }
        else
        {
            // Invalid value - reset display
            updateFromManager();
        }
    }
}

// SettingsScreen implementation
SettingsScreen::SettingsScreen(StemPlayerAudioProcessor& processor, 
                                StemPlayerAudioProcessorEditor& ed)
    : audioProcessor(processor), editor(ed)
{
    // Load current patterns for editing
    editingPatterns = audioProcessor.getAppSettings().getStemPatterns();
    patternListModel.setPatterns(editingPatterns);
    patternListModel.onPatternsChanged = [this]() { saveSettings(); };
    
    // Title
    titleLabel.setText("Settings", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
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
    folderSectionLabel.setText("Default Stems Folder", juce::dontSendNotification);
    folderSectionLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    folderSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(folderSectionLabel);
    
    defaultFolderLabel.setFont(juce::Font(13.0f));
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
    
    // MIDI assignment section
    midiSectionLabel.setText("MIDI Control Assignments", juce::dontSendNotification);
    midiSectionLabel.setFont(juce::Font(16.0f, juce::Font::bold));
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
    audioProcessor.getMidiLearnManager().onMappingChanged = [this](MidiControlType type, int cc) {
        juce::ignoreUnused(type, cc);
        updateMidiRows();
    };
    
    // Patterns section
    patternsSectionLabel.setText("Stem Detection Patterns", juce::dontSendNotification);
    patternsSectionLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    patternsSectionLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    addAndMakeVisible(patternsSectionLabel);
    
    // Patterns table
    patternsTable.setModel(&patternListModel);
    patternsTable.setColour(juce::ListBox::backgroundColourId, 
                            StemPlayerLookAndFeel::backgroundMedium);
    patternsTable.setRowHeight(32);
    
    auto& header = patternsTable.getHeader();
    header.addColumn("Stem Type", 1, 150, 100, 200);
    header.addColumn("Pattern", 2, 200, 100, 300);
    header.setStretchToFitActive(true);
    
    addAndMakeVisible(patternsTable);
    
    // Pattern buttons
    addPatternButton.setButtonText("Add");
    addPatternButton.onClick = [this]() { addPattern(); };
    addAndMakeVisible(addPatternButton);
    
    removePatternButton.setButtonText("Remove");
    removePatternButton.onClick = [this]() { removeSelectedPattern(); };
    addAndMakeVisible(removePatternButton);
    
    resetPatternsButton.setButtonText("Reset to Default");
    resetPatternsButton.onClick = [this]() { resetPatternsToDefault(); };
    addAndMakeVisible(resetPatternsButton);
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
    auto bounds = getLocalBounds().reduced(30);
    
    // Header row
    auto header = bounds.removeFromTop(40);
    backButton.setBounds(header.removeFromLeft(80));
    header.removeFromLeft(20);
    titleLabel.setBounds(header);
    
    bounds.removeFromTop(20);
    
    // Default folder section
    folderSectionLabel.setBounds(bounds.removeFromTop(24));
    bounds.removeFromTop(8);
    
    auto folderRow = bounds.removeFromTop(35);
    clearFolderButton.setBounds(folderRow.removeFromRight(70));
    folderRow.removeFromRight(10);
    browseFolderButton.setBounds(folderRow.removeFromRight(90));
    folderRow.removeFromRight(10);
    defaultFolderLabel.setBounds(folderRow);
    
    bounds.removeFromTop(20);
    
    // MIDI section
    midiSectionLabel.setBounds(bounds.removeFromTop(24));
    bounds.removeFromTop(8);
    
    int midiRowHeight = 36;
    int midiTotalHeight = static_cast<int>(midiRows.size()) * (midiRowHeight + 4);
    int midiViewportHeight = juce::jmin(200, midiTotalHeight + 10);
    
    midiViewport.setBounds(bounds.removeFromTop(midiViewportHeight));
    midiContainer.setSize(midiViewport.getWidth() - 20, midiTotalHeight);
    
    int y = 0;
    for (auto& row : midiRows)
    {
        row->setBounds(0, y, midiContainer.getWidth(), midiRowHeight);
        y += midiRowHeight + 4;
    }
    
    bounds.removeFromTop(20);
    
    // Patterns section
    patternsSectionLabel.setBounds(bounds.removeFromTop(24));
    bounds.removeFromTop(8);
    
    // Pattern buttons at bottom
    auto buttonRow = bounds.removeFromBottom(40);
    addPatternButton.setBounds(buttonRow.removeFromLeft(70));
    buttonRow.removeFromLeft(10);
    removePatternButton.setBounds(buttonRow.removeFromLeft(80));
    buttonRow.removeFromLeft(10);
    resetPatternsButton.setBounds(buttonRow.removeFromLeft(130));
    
    bounds.removeFromBottom(10);
    
    // Table takes remaining space
    patternsTable.setBounds(bounds);
}

void SettingsScreen::visibilityChanged()
{
    if (isVisible())
        updateMidiRows();
}

void SettingsScreen::updateMidiRows()
{
    for (auto& row : midiRows)
        row->updateFromManager();
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

void SettingsScreen::addPattern()
{
    StemPattern newPattern;
    newPattern.stemType = "NewStem";
    newPattern.pattern = "_newstem";
    
    editingPatterns.add(newPattern);
    patternsTable.updateContent();
    saveSettings();
    
    // Select the new row
    patternsTable.selectRow(editingPatterns.size() - 1);
}

void SettingsScreen::removeSelectedPattern()
{
    int selectedRow = patternsTable.getSelectedRow();
    
    if (selectedRow >= 0 && selectedRow < editingPatterns.size())
    {
        editingPatterns.remove(selectedRow);
        patternsTable.updateContent();
        saveSettings();
    }
}

void SettingsScreen::resetPatternsToDefault()
{
    editingPatterns = StemDetector::getDefaultPatterns();
    patternsTable.updateContent();
    saveSettings();
}

void SettingsScreen::saveSettings()
{
    audioProcessor.getAppSettings().setStemPatterns(editingPatterns);
}
