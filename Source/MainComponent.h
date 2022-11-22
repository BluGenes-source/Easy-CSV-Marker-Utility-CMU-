#pragma once

#include <JuceHeader.h>
#include <iostream>
//==============================================================================
/*
   
Utility to extract chapter markers from a DaVinci Resolve CSV file

Open Source GNU License
Written By [11/14/2022 - Gene Brown]

*/



struct tTime
{
    int hours = 0;
    int mins = 0;
    int secs = 0;
};

struct Marker
{
    String desription;
    tTime timeStamp;
    int totalsec;
};


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
    void canSave(const bool cSave, bool cClear);
    void clearEditClicked();
    void shouldQuitToggled();
    void copyButtonClicked();

    
    tTime getTtime(const String atime);  // convert string to tTime struct
    int getSeconds(const tTime atime);
    void listTime();
    void ohNo(const String msg, const bool isError);
    bool justClear = false;
    Label AppTitle;
    tTime time1, time2;
    Array<Marker> markers;

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

    String TestTime1 = "00:00:45";
    String TestTime2 = "00:01:42";
    String TestTime3 = "00:02:35";
    String TestTime4 = "00:02:44";

    std::unique_ptr<FilenameComponent> fileComp;
    std::unique_ptr<TextEditor>        textContent;
   
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
       
        
};
