#pragma once

#include <JuceHeader.h>
#include <iostream>
//==============================================================================
/*
   
Utility to extract chapter markers from a DaVinci Resolve CSV file

Open Source GNU License


*/




class MyToggleLookAndFeel : public LookAndFeel_V4
{
    void drawToggleButton(Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown);
    
};




class MainComponent  : public juce::Component,
                       public juce::FilenameComponentListener
                                    
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================

    void paint (juce::Graphics&) override;
    void resized() override;
    void filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged) override ;

    void readFile(const File& fileToRead);
    static String readUpToNextComma(FileInputStream& inputStream);
    static String getFieldByNumber(int fNum, const String line);
    void saveButtonClicked();
    void writeNewfile(const File& filetowrite);
    void canSave(const bool cSave);
    void clearEditClicked();
    void shouldQuitToggled();
    void copyButtonClicked();
    bool justClear = false;
    Label AppTitle;
private:
    TextButton saveButton;
    TextButton clearEditButton;
    TextButton copyButton;
    ToggleButton shouldQuit;
    File dataFilePath;
    MyToggleLookAndFeel TLook;

    
    int buttonWidth = 100;
    int buttonHeight = 40;
    bool closeApp = false;
   

    std::unique_ptr<FilenameComponent> fileComp;
    std::unique_ptr<TextEditor>        textContent;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
       
        
};
