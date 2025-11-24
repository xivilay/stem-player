#include "SelectionScreen.h"
#include "../PluginProcessor.h"
#include "../PluginEditor.h"
#include "LookAndFeel.h"

// SongListModel implementation
void SongListModel::setSongs(const juce::Array<DetectedSong>& songs)
{
    detectedSongs = songs;
}

const DetectedSong* SongListModel::getSong(int index) const
{
    if (index >= 0 && index < detectedSongs.size())
        return &detectedSongs.getReference(index);
    return nullptr;
}

int SongListModel::getNumRows()
{
    return detectedSongs.size();
}

void SongListModel::paintListBoxItem(int rowNumber, juce::Graphics& g, 
                                      int width, int height, bool rowIsSelected)
{
    if (rowNumber < 0 || rowNumber >= detectedSongs.size())
        return;
    
    auto bounds = juce::Rectangle<int>(0, 0, width, height).reduced(4, 2);
    
    // Background
    if (rowIsSelected)
    {
        g.setColour(StemPlayerLookAndFeel::accentPrimary.withAlpha(0.3f));
        g.fillRoundedRectangle(bounds.toFloat(), 4.0f);
    }
    
    const auto& song = detectedSongs.getReference(rowNumber);
    
    // Song name
    g.setColour(StemPlayerLookAndFeel::textPrimary);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(song.songName, bounds.reduced(8, 0).removeFromTop(height / 2 + 4), 
               juce::Justification::centredLeft, true);
    
    // Stem count and types
    juce::String stemInfo = juce::String(song.stemFiles.size()) + " stems: ";
    for (int i = 0; i < song.stemTypes.size(); ++i)
    {
        if (i > 0) stemInfo += ", ";
        stemInfo += song.stemTypes[i];
    }
    
    g.setColour(StemPlayerLookAndFeel::textSecondary);
    g.setFont(juce::Font(12.0f));
    g.drawText(stemInfo, bounds.reduced(8, 0).removeFromBottom(height / 2), 
               juce::Justification::centredLeft, true);
}

void SongListModel::listBoxItemClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (onSongSelected)
        onSongSelected(row);
}

void SongListModel::listBoxItemDoubleClicked(int row, const juce::MouseEvent& /*e*/)
{
    if (onSongDoubleClicked)
        onSongDoubleClicked(row);
}

// SelectionScreen implementation
SelectionScreen::SelectionScreen(StemPlayerAudioProcessor& processor, 
                                  StemPlayerAudioProcessorEditor& ed)
    : audioProcessor(processor), editor(ed)
{
    // Update detector patterns from settings
    stemDetector.setPatterns(audioProcessor.getAppSettings().getStemPatterns());
    
    // Title
    titleLabel.setText("Stem Player", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(32.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Folder path label
    folderLabel.setFont(juce::Font(13.0f));
    folderLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    folderLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(folderLabel);
    
    // Browse button
    browseButton.setButtonText("Browse Folder");
    browseButton.onClick = [this]() { browseForFolder(); };
    addAndMakeVisible(browseButton);
    
    // Settings button
    settingsButton.setButtonText("Settings");
    settingsButton.onClick = [this]() {
        editor.showScreen(StemPlayerAudioProcessor::Screen::Settings);
    };
    addAndMakeVisible(settingsButton);
    
    // Load button
    loadButton.setButtonText("Load Song");
    loadButton.onClick = [this]() { loadSelectedSong(); };
    loadButton.setEnabled(false);
    addAndMakeVisible(loadButton);
    
    // Song list
    songListModel.onSongSelected = [this](int row) {
        selectedSongIndex = row;
        loadButton.setEnabled(row >= 0);
    };
    songListModel.onSongDoubleClicked = [this](int /*row*/) {
        loadSelectedSong();
    };
    
    songListBox.setModel(&songListModel);
    songListBox.setRowHeight(56);
    songListBox.setColour(juce::ListBox::backgroundColourId, 
                          StemPlayerLookAndFeel::backgroundLight);
    addAndMakeVisible(songListBox);
    
    // Status label
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    statusLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(statusLabel);
    
    // Load default folder if set
    auto defaultFolder = audioProcessor.getAppSettings().getDefaultFolder();
    if (defaultFolder.isNotEmpty())
    {
        currentFolder = juce::File(defaultFolder);
        if (currentFolder.isDirectory())
            scanCurrentFolder();
    }
}

SelectionScreen::~SelectionScreen()
{
}

void SelectionScreen::paint(juce::Graphics& g)
{
    g.fillAll(StemPlayerLookAndFeel::backgroundDark);
    
    // Decorative gradient overlay
    juce::ColourGradient gradient(
        StemPlayerLookAndFeel::accentPrimary.withAlpha(0.05f), 0, 0,
        juce::Colours::transparentBlack, 0, (float)getHeight() * 0.4f, false);
    g.setGradientFill(gradient);
    g.fillRect(getLocalBounds());
}

void SelectionScreen::resized()
{
    auto bounds = getLocalBounds().reduced(30);
    
    // Title area
    titleLabel.setBounds(bounds.removeFromTop(60));
    bounds.removeFromTop(20);
    
    // Folder selection row
    auto folderRow = bounds.removeFromTop(40);
    browseButton.setBounds(folderRow.removeFromRight(120));
    folderRow.removeFromRight(10);
    settingsButton.setBounds(folderRow.removeFromRight(100));
    folderRow.removeFromRight(10);
    folderLabel.setBounds(folderRow);
    
    bounds.removeFromTop(20);
    
    // Load button at bottom
    auto bottomRow = bounds.removeFromBottom(45);
    loadButton.setBounds(bottomRow.withSizeKeepingCentre(200, 40));
    
    bounds.removeFromBottom(10);
    
    // Status at bottom
    statusLabel.setBounds(bounds.removeFromBottom(24));
    
    bounds.removeFromBottom(10);
    
    // Song list takes remaining space
    songListBox.setBounds(bounds);
}

void SelectionScreen::refresh()
{
    // Reload patterns from settings
    stemDetector.setPatterns(audioProcessor.getAppSettings().getStemPatterns());
    
    // Reload default folder if changed
    auto defaultFolder = audioProcessor.getAppSettings().getDefaultFolder();
    if (defaultFolder.isNotEmpty() && currentFolder.getFullPathName() != defaultFolder)
    {
        currentFolder = juce::File(defaultFolder);
        if (currentFolder.isDirectory())
            scanCurrentFolder();
    }
    else if (currentFolder.isDirectory())
    {
        scanCurrentFolder();
    }
}

void SelectionScreen::browseForFolder()
{
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Stems Folder", currentFolder, "", true);
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectDirectories,
                         [this, chooser](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.isDirectory())
        {
            currentFolder = result;
            scanCurrentFolder();
        }
    });
}

void SelectionScreen::scanCurrentFolder()
{
    if (!currentFolder.isDirectory())
        return;
    
    folderLabel.setText(currentFolder.getFullPathName(), juce::dontSendNotification);
    
    detectedSongs = stemDetector.scanDirectory(currentFolder);
    songListModel.setSongs(detectedSongs);
    songListBox.updateContent();
    
    selectedSongIndex = -1;
    loadButton.setEnabled(false);
    
    if (detectedSongs.isEmpty())
        statusLabel.setText("No songs found. Check stem patterns in settings.", 
                           juce::dontSendNotification);
    else
        statusLabel.setText(juce::String(detectedSongs.size()) + " songs found", 
                           juce::dontSendNotification);
}

void SelectionScreen::loadSelectedSong()
{
    if (selectedSongIndex < 0 || selectedSongIndex >= detectedSongs.size())
        return;
    
    const auto& song = detectedSongs.getReference(selectedSongIndex);
    editor.onSongSelected(song.songName, song.stemFiles);
}

