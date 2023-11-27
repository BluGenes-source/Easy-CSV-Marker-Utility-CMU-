# Chapter Marker Utility (CMU)

Updated to use VS 2022 and JUCE 7.04

A utility to create chapter markers from a comma seperated value file (CSV) exported from DaVinci Resolve, 
or any video editor that can export CSV files.

To use a different video editor, see the source code to change the field values.

This application was created using JUCE version 6.07, but, may work with later versions. (untested).

Get the API here: https://github.com/juce-framework

Just clone and then use "checkout branch 6.0.7"  
This is located in the "releases" area of GitHub. (https://github.com/juce-framework/JUCE/releases)

From there, follow the instructions on how to build the ProJucer App.

## Patching JUCE Core

It does require that you add a method to the JUCE core that allows you to clear the filename box in the
filenameComponent.


Open the file juce_FilenameComponent.h and at the bottom of the public declarartions add this method:
e.g. (\JUCE\modules\juce_gui_basics\filebrowser)
~~~cpp
void clearBox();
~~~

In the .cpp file add this code:

~~~cpp
void FilenameComponent::clearBox()
{
    filenameBox.clear();
    setCurrentFile({}, true, dontSendNotification);
}
~~~

This is called when you click the *Clear Edit* button.

This clears the filename box, but, does not remove the history.  The intention is to have a way to "start over".

### Enabling the component resizer with native OS windows title bar

Optionally, you can enable the bottom resizer component to show when using the native title bar.

in the file found in the JUCE gui basics module, windows, juce_ResisiableWindow.cpp
(\modules\juce_gui_basics\windows\juce_ResisiableWindow.cpp )

comment out the flag isUsingNativeTitleBar();  

Line 203

~~~cpp
const bool resizerHidden = isFullScreen() || isKioskMode() /* || isUsingNativeTitleBar()*/;
~~~

## How to use

Open the jucer file and choose your exporter, launch and build it..

Run the app and just drop a CSV file onto the file name component.  It will create the markers and you can save as a text file, or, copy and paste from the clip board. It saves the text file in the same directory as the CSV file.

The app was created to use with YouTube.  Just paste to the description of your video.

Although YouTube has "automatic" chapters, this will catch typo's if spell check is enabled in your browser.

Also usefull for videos that have already been posted, but, don't have markers set.

This is open source under GNU licsence.