#pragma once

#include <JuceHeader.h>
#include "../Core/StemDetector.h"
#include "IconButton.h"

class StemPlayerAudioProcessor;
class StemPlayerAudioProcessorEditor;

class SongListModel : public juce::ListBoxModel
{
public:
    SongListModel() = default;
    
    void setSongs(const juce::Array<DetectedSong>& songs);
    const DetectedSong* getSong(int index) const;
    
    int getNumRows() override;
    void paintListBoxItem(int rowNumber, juce::Graphics& g, int width, int height,
                          bool rowIsSelected) override;
    void listBoxItemClicked(int row, const juce::MouseEvent& e) override;
    void listBoxItemDoubleClicked(int row, const juce::MouseEvent& e) override;
    
    std::function<void(int)> onSongSelected;
    std::function<void(int)> onSongDoubleClicked;

private:
    juce::Array<DetectedSong> detectedSongs;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SongListModel)
};

class SelectionScreen : public juce::Component
{
public:
    SelectionScreen(StemPlayerAudioProcessor& processor, 
                    StemPlayerAudioProcessorEditor& editor);
    ~SelectionScreen() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void refresh();

private:
    void browseForFolder();
    void scanCurrentFolder();
    void loadSelectedSong();
    
    StemPlayerAudioProcessor& audioProcessor;
    StemPlayerAudioProcessorEditor& editor;
    
    StemDetector stemDetector;
    
    juce::Label folderLabel;
    IconButton browseButton { IconType::Browse };
    IconButton settingsButton { IconType::Settings };
    IconButton loadButton { IconType::Load };
    
    juce::ListBox songListBox;
    SongListModel songListModel;
    
    juce::Label statusLabel;
    
    juce::File currentFolder;
    juce::Array<DetectedSong> detectedSongs;
    int selectedSongIndex { -1 };
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SelectionScreen)
};

