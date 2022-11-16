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
	addAndMakeVisible(saveButton);
	saveButton.setButtonText(TRANS("Save File"));
	saveButton.setColour(juce::TextButton::buttonColourId, Colours::green);
	saveButton.onClick = [this] { saveButtonClicked(); };
	canSave(false);

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
	juce::String newNextWord = "";
	int fieldTwentyTwoEnd = 0;
	int fieldSevenEnd = 0;
	int fieldTwentyTwoStart = 0;
	int fieldSevenStart = 0;
	int targetCount =0;
	int lineSize;
	juce::String fieldName2;
	juce::String fieldName1;
	String thisLine = "";
	String NewLine = "\n";

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

			// begins and end found, now extract the data
			fieldName1 = nextWord.substring(fieldSevenStart, fieldSevenEnd-4);  // first needed field less frames
			fieldName1 = fieldName1.unquoted();

			fieldName2 = nextWord.substring(fieldTwentyTwoStart, fieldTwentyTwoEnd); // second needed field
			fieldName2 = fieldName2.unquoted();
			newNextWord = fieldName1 + " " + fieldName2 + NewLine;
           textContent->insertTextAtCaret(newNextWord);		
	}
	canSave(true);
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

void MainComponent::canSave(const bool cSave)
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
		clearEditButton.setEnabled(false);
	}

}

void MainComponent::clearEditClicked()
{
	textContent->clear();
	justClear = true;
	fileComp->clearBox();
	canSave(false);
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

