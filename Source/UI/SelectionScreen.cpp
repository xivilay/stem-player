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
        g.fillRect(bounds.toFloat());
    }
    
    const auto& song = detectedSongs.getReference(rowNumber);
    
    // Song name
    g.setColour(StemPlayerLookAndFeel::textPrimary);
    g.setFont(juce::Font(15.0f, juce::Font::bold));
    g.drawText(song.songName, bounds.reduced(8, 0).removeFromTop(height / 2 + 4), 
               juce::Justification::centredLeft, true);
    
    // Show which stems are available
    juce::String stemInfo;
    int stemCount = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (song.stemFound[i])
        {
            if (stemCount > 0) stemInfo += ", ";
            stemInfo += StemDetector::getStemTypeName(i);
            stemCount++;
        }
    }
    stemInfo = juce::String(stemCount) + " stems: " + stemInfo;
    
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
    stemDetector.setPatterns(audioProcessor.getAppSettings().getStemRegexPatterns());
    
    // Folder path label
    folderLabel.setFont(juce::Font(13.0f));
    folderLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textPrimary);
    folderLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(folderLabel);
    
    // Status label (songs count)
    statusLabel.setFont(juce::Font(12.0f));
    statusLabel.setColour(juce::Label::textColourId, StemPlayerLookAndFeel::textSecondary);
    statusLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(statusLabel);
    
    // Load button
    loadButton.onClick = [this]() { loadSelectedSong(); };
    loadButton.setEnabled(false);
    addAndMakeVisible(loadButton);
    
    // Browse button
    browseButton.onClick = [this]() { browseForFolder(); };
    addAndMakeVisible(browseButton);
    
    // Settings button
    settingsButton.onClick = [this]() {
        editor.showScreen(StemPlayerAudioProcessor::Screen::Settings);
    };
    addAndMakeVisible(settingsButton);
    
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
    
    // Header area - slightly lighter than main background
    g.setColour(StemPlayerLookAndFeel::backgroundMedium.darker(0.3f));
    g.fillRect(getLocalBounds().removeFromTop(50));
}

void SelectionScreen::resized()
{
    auto bounds = getLocalBounds();
    
    // Single header row with all controls
    auto header = bounds.removeFromTop(50);
    header.reduce(15, 8);
    
    // Settings button on right (icon)
    settingsButton.setBounds(header.removeFromRight(40));
    header.removeFromRight(8);
    
    // Browse button
    browseButton.setBounds(header.removeFromRight(70));
    header.removeFromRight(8);
    
    // Load button
    loadButton.setBounds(header.removeFromRight(60));
    header.removeFromRight(15);
    
    // Status label (song count)
    statusLabel.setBounds(header.removeFromRight(100));
    header.removeFromRight(10);
    
    // Folder path takes remaining space
    folderLabel.setBounds(header);
    
    // Song list takes remaining space
    bounds.reduce(15, 10);
    songListBox.setBounds(bounds);
}

void SelectionScreen::refresh()
{
    // Reload patterns from settings
    stemDetector.setPatterns(audioProcessor.getAppSettings().getStemRegexPatterns());
    
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
#if JUCE_IOS
    // On iOS, default to Documents directory if no folder selected yet
    juce::File startFolder = currentFolder;
    if (!startFolder.isDirectory())
        startFolder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
#else
    juce::File startFolder = currentFolder;
#endif
    
    auto chooser = std::make_shared<juce::FileChooser>(
        "Select Stems Folder", startFolder, "", true);
    
    chooser->launchAsync(juce::FileBrowserComponent::openMode | 
                         juce::FileBrowserComponent::canSelectDirectories,
                         [this, chooser](const juce::FileChooser& fc) {
        auto result = fc.getResult();
        if (result.exists())
        {
            // On iOS, the result might be a file in the folder we want
            juce::File folder = result.isDirectory() ? result : result.getParentDirectory();
            
            if (folder.isDirectory())
            {
                currentFolder = folder;
                
                // Save the folder path for persistence
                audioProcessor.getAppSettings().setDefaultFolder(currentFolder.getFullPathName());
                
                scanCurrentFolder();
            }
        }
    });
}

void SelectionScreen::scanCurrentFolder()
{
    if (!currentFolder.isDirectory())
        return;
    
#if JUCE_IOS
    // On iOS, we may need to access security-scoped resources
    // The URL bookmark handling is done automatically by JUCE's FileChooser
    // but we should ensure we have access
    if (!currentFolder.hasReadAccess())
    {
        statusLabel.setText("Cannot access folder - permission denied", juce::dontSendNotification);
        return;
    }
#endif
    
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
    editor.onSongSelected(song.songName, song.stemFiles, song.stemFound);
}

