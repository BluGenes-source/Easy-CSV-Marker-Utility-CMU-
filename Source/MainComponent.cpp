#include "MainComponent.h"


/* 
 *  Written By [11/14/2022 - Gene Brown]*/


//==============================================================================
MainComponent::MainComponent()
{
	fileComp.reset(new FilenameComponent("fileComp",
		{},                       // current file
		false,                    // can edit file name,
		false,                    // is directory,
		false,                    // is for saving,
		{"*.csv"},                // browser wildcard suffix,
		{},                       // enforced suffix,
		"Select CSV file to open"));  // text when nothing selected
	addAndMakeVisible(fileComp.get());
	fileComp->addListener(this);
	fileComp->setDefaultBrowseTarget(File::getSpecialLocation(File::userDocumentsDirectory));

	textContent.reset(new TextEditor());
	addAndMakeVisible(textContent.get());
	textContent->setMultiLine(true);
	textContent->setReadOnly(false);
	textContent->setCaretVisible(true);
	textContent->setEnabled(true);
	textContent->setReturnKeyStartsNewLine(true);
	textContent->setColour(juce::TextEditor::textColourId, juce::Colours::white);

	addAndMakeVisible(saveButton);
	saveButton.setButtonText(TRANS("Save File"));
	saveButton.setColour(juce::TextButton::buttonColourId, Colours::green);
	saveButton.onClick = [this] { saveButtonClicked(); };
	canSave(false, false);

	addAndMakeVisible(clearEditButton);
	clearEditButton.setButtonText(TRANS("Clear Edit"));
	clearEditButton.onClick = [this] {clearEditClicked(); };
	addAndMakeVisible(copyButton);
	copyButton.setButtonText(TRANS("Copy to Clipboard"));
	copyButton.onClick = [this] {copyButtonClicked(); };

	addAndMakeVisible(shouldQuit);
	shouldQuit.setLookAndFeel(&TLook);
	
	
	shouldQuit.onClick = [this]{ shouldQuitToggled(); };
	shouldQuit.setButtonText(TRANS("Quit after Save"));
	shouldQuit.setToggleState(false, dontSendNotification);

	addAndMakeVisible(AppTitle);
	AppTitle.setFont(juce::Font("Times New Roman", 36.00f, juce::Font::plain).withTypefaceStyle("Bold"));
	//AppTitle.setColour(juce::Label::textColourId, juce::Colour(0xff1bcb39));
	AppTitle.setColour(juce::Label::textColourId, juce::Colours::goldenrod);
	AppTitle.setText("Easy Marker - Chapter Marker Utility",dontSendNotification);

	setSize(800, 600);
}

MainComponent::~MainComponent()
{
	fileComp->removeListener(this);
	shouldQuit.setLookAndFeel(nullptr);

}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
	auto area = getLocalBounds();
	auto strokeColour = juce::Colours::goldenrod;  // border
	g.fillAll(Colours::darkgrey);
	g.setColour(strokeColour);
	g.drawRect(area, 2);
}

void MainComponent::resized()
{
	AppTitle.setBounds((getWidth() / 2)- 180, 00, 400, 50);
	fileComp->setBounds(10, 60, getWidth() - 20, 50);
	textContent->setBounds(10, 120, getWidth() - 20, getHeight() - 250);
	saveButton.setBounds(getWidth()  - 150, getHeight() - 100, buttonWidth, buttonHeight);
	shouldQuit.setBounds(getWidth() - 550, getHeight() - 100, buttonWidth+50, buttonHeight);
	clearEditButton.setBounds(getWidth() - 275, getHeight() - 100, buttonWidth, buttonHeight);
	copyButton.setBounds(getWidth() - 385, getHeight() - 100, buttonWidth, buttonHeight);
}


void MainComponent::readFile(const File& fileToRead)
{
	bool triggerWarning = false;
	bool firstPass;
	int markerCount = 0;
	juce::String newNextWord = "";
	int fieldTwentyTwoEnd = 0;
	int fieldSevenEnd = 0;
	int fieldTwentyTwoStart = 0;
	int fieldSevenStart = 0;
	int targetCount =0;
	int lineSize;
	juce::String fieldName2;   // the description
	juce::String fieldName1;  // the time
	String thisLine = "";
	String NewLine = "\n";
	Marker tempMarker;
	Marker introMarker;
	
	bool secFlagged = false;

	markers.clear();
	if (!fileToRead.existsAsFile())
		return;  // file doesn't exist

	FileInputStream inputStream(fileToRead);

	if (!inputStream.openedOk())
		return;  // failed to open
	dataFilePath = fileToRead.getFullPathName();
	textContent->clear();

	textContent->insertTextAtCaret(NewLine);
	textContent->insertTextAtCaret(NewLine);
	thisLine = inputStream.readNextLine(); // skip 1st line
	firstPass = true;
	while (!inputStream.isExhausted())
	{
		auto nextWord = inputStream.readNextLine();

			lineSize = nextWord.length();

			for (int Index = 0; Index < lineSize ; Index++)  // begin 7th field
			{
				if (nextWord[Index] == ',')
					targetCount++;
				
				if (targetCount == 7)

				{
					fieldSevenStart = Index+1;
					targetCount = 0;
					break;
				}
			}

            /**
             * This calculates the fields from the CSV file.  If you are trying to use a CSV file from a different editor, of course, 
             * the fields would be wrong.  You would have to figure out the correct positions in the file.
             */
			for (int Index = 0; Index < lineSize; Index++)  // end 7th field
			{
				if (nextWord[Index] == ',')
					targetCount++;
				if (targetCount == 8)
				{
					fieldSevenEnd = Index;
					targetCount = 0;
					break;
				}
			}

			for (int Index = 0; Index < lineSize; Index++)  // begin 22nd field
			{
				if (nextWord[Index] == ',')
					targetCount++;

				if (targetCount == 20)

				{
					fieldTwentyTwoStart = Index + 1;
					targetCount = 0;
					break;
				}
			}

			for (int Index = 0; Index < lineSize; Index++)  // end 22nd field
			{
				if (nextWord[Index] == ',')
					targetCount++;
				if (targetCount == 21)
				{
					fieldTwentyTwoEnd = Index;
					targetCount = 0;
					break;
				}
			}

			// begins and ends found, now extract the data
			fieldName1 = nextWord.substring(fieldSevenStart, fieldSevenEnd-4);  // first needed field less frames
			fieldName1 = fieldName1.unquoted();

			fieldName2 = nextWord.substring(fieldTwentyTwoStart, fieldTwentyTwoEnd); // second needed field
			fieldName2 = fieldName2.unquoted();
			newNextWord = fieldName1 + " " + fieldName2 + NewLine;
			
			time1 = getTtime(fieldName1);
			tempMarker.timeStamp = time1;
			tempMarker.desription = fieldName2;
			if (firstPass)
			{
				auto tempSec = getSeconds(tempMarker.timeStamp);
				if (tempSec > 0)
				{ // first marker must be intro at 00:00 !!
					introMarker.desription = "Intro";
					introMarker.timeStamp.hours = 0;
					introMarker.timeStamp.mins = 0;
					introMarker.timeStamp.secs = 0;
					introMarker.totalsec = 0;
					markers.add(introMarker);
					markerCount++;
					String introString = "00:00:00 Intro" + NewLine;
					textContent->setColour(juce::TextEditor::textColourId, juce::Colours::limegreen);
					textContent->insertTextAtCaret(introString);
					firstPass = false;
					triggerWarning = true;
				}
			}
			
			markers.add(tempMarker);
			markerCount++;
			if (markers.size() > 1)
			{
				auto sec1 = getSeconds(markers[markerCount - 2].timeStamp);
				auto  sec2 = getSeconds(markers[markerCount - 1].timeStamp);
				auto secdiff = sec2 - sec1;

				if (secdiff < 10)
					{
						textContent->setColour(juce::TextEditor::textColourId, juce::Colours::yellow);
						textContent->insertTextAtCaret(newNextWord);
						secFlagged = true;
					}
				else
					{
						textContent->setColour(juce::TextEditor::textColourId, juce::Colours::white);
						textContent->insertTextAtCaret(newNextWord);
					}
			}
			else
				{
					textContent->setColour(juce::TextEditor::textColourId, juce::Colours::white);
					textContent->insertTextAtCaret(newNextWord);
				}
			
    	firstPass = false;
	}
	
	if (secFlagged)
	{
		ohNo(TRANS("Markers high lighted in Yellow were found to be shorter than 10 seconds! These may not work properly! You should edit this now.."), true);
		secFlagged = false;
	}
		
	if (triggerWarning)
	{
		ohNo(TRANS("Intro Marker was missing but was added!"), false);
		triggerWarning = false;
	}
		
	if (markers.size() < 3)
	{
		ohNo(TRANS("Three markers are required by YouTube for chapter markers to work! You need to add more markers."), true);
		canSave(false, true);
	}
	else
	canSave(true, true);
}	


void MainComponent::filenameComponentChanged(FilenameComponent* fileComponentThatHasChanged) 
{
	String isNewFile = "";
	if (fileComponentThatHasChanged == fileComp.get())
	{
		if (justClear)
		{
			justClear = false;
			
			fileComp->setCurrentFile(isNewFile, false, dontSendNotification);
			return;
		}
		readFile(fileComp->getCurrentFile());

	}
}


void MainComponent::writeNewfile(const File& filetowrite)
{
	FileOutputStream outputStream(filetowrite);

	if (outputStream.openedOk())
	{
		// create a new file with default entries
		outputStream.setPosition(0);
		outputStream.truncate();
		outputStream.writeText(textContent->getText(), false, false, "\r\n");
		outputStream.flush();// flush it
	}
	else
	{
		//   fileStatus =  outputStream.getStatus();
	}
}


/** cSave is used to enable or disable the buttons. cClear is used to enable or disable just the Clear Edit button. */
void MainComponent::canSave(const bool cSave, bool cClear)
{
	if (cSave)
	{
		saveButton.setColour(juce::TextButton::buttonColourId, Colours::green);
		saveButton.setEnabled(true);
		copyButton.setColour(juce::TextButton::buttonColourId, Colours::blue);
		copyButton.setEnabled(true);
		clearEditButton.setColour(juce::TextButton::buttonColourId, Colours::red);
		clearEditButton.setEnabled(true);
	}
	else
	{
		saveButton.setColour(juce::TextButton::buttonColourId, Colours::grey);
		saveButton.setEnabled(false);
		copyButton.setColour(juce::TextButton::buttonColourId, Colours::grey);
		copyButton.setEnabled(false);
		clearEditButton.setColour(juce::TextButton::buttonColourId, Colours::grey);
		if (cClear)
		{
			clearEditButton.setColour(juce::TextButton::buttonColourId, Colours::red);
			clearEditButton.setEnabled(true);
		}
		else
		{
			clearEditButton.setColour(juce::TextButton::buttonColourId, Colours::grey);
			clearEditButton.setEnabled(false);
		}
	}

}

void MainComponent::clearEditClicked()
{
	textContent->clear();
	justClear = true;
	fileComp->clearBox();
	canSave(false, false);
}

void MainComponent::shouldQuitToggled()
{
	if (shouldQuit.getToggleState())
	{
		closeApp = true;
	}
	else
	{
		closeApp = false;
	}
}

void MainComponent::copyButtonClicked()
{
	if (textContent->selectAll())
		textContent->copy();
}


/** Creates a time struct from a string */
tTime MainComponent::getTtime(const String atime)
{
	tTime tempTime;
	String h, m, s;
	h = atime.substring(0, 2);
	m = atime.substring(3, 5);
	s = atime.substring(6);

	tempTime.hours = h.getIntValue();
	tempTime.mins = m.getIntValue();
	tempTime.secs = s.getIntValue();

	return tempTime;
}

/** Returns the total number of seconds from the time struct */
int MainComponent::getSeconds(const tTime atime)
{
	int tempSecs = 0;
	tempSecs = atime.hours * 3600 + atime.mins * 60 + atime.secs;
	return tempSecs;
}


/** debug code - used to loop the array and display the time struct. */
void MainComponent::listTime()
{
	for (int Index1 = 0; Index1 < markers.size(); Index1++)
	{
		DBG(String (markers[Index1].timeStamp.hours));
		DBG(String(markers[Index1].timeStamp.mins));
		DBG(String(markers[Index1].timeStamp.secs));
		DBG("-------");
	}
}

void MainComponent::ohNo(const String msg, const bool isError)
{
	if (isError)
	{
		AlertWindow w5(TRANS("Warning!"), TRANS(msg), AlertWindow::WarningIcon);

		w5.addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
		if (w5.runModalLoop() != 0) // is they picked 'OK'
		{
			return;
		}
	}
	else
	{
		AlertWindow w5(TRANS("Info"), TRANS(msg), AlertWindow::InfoIcon);
		w5.addButton("OK", 1, KeyPress(KeyPress::returnKey, 0, 0));
		if (w5.runModalLoop() != 0) // is they picked 'OK'
		{
			return;
		}
	}
}

void MainComponent::saveButtonClicked()
{
	dataFilePath = dataFilePath.withFileExtension("txt");
	writeNewfile(dataFilePath);

	if (closeApp)
		JUCEApplication::getInstance()->systemRequestedQuit();
}

void MyToggleLookAndFeel::drawToggleButton(Graphics& g, ToggleButton& button, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	float mySize = 18.0f;
	
	auto fontSize = jmin(mySize, (float)button.getHeight() * 0.75f);
	auto tickWidth = fontSize * 1.1f;

	drawTickBox(g, button, 4.0f, ((float)button.getHeight() - tickWidth) * 0.5f,
		tickWidth, tickWidth,
		button.getToggleState(),
		button.isEnabled(),
		shouldDrawButtonAsHighlighted,
		shouldDrawButtonAsDown);

    g.setColour(button.findColour(ToggleButton::textColourId));
	g.setFont(fontSize);

	if (!button.isEnabled())
		g.setOpacity(0.5f);

	g.drawFittedText(button.getButtonText(),
		button.getLocalBounds().withTrimmedLeft(roundToInt(tickWidth) + 10)
		.withTrimmedRight(2),
		Justification::centredLeft, 10);
}

