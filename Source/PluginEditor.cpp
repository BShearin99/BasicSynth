#include "PluginProcessor.h"
#include "PluginEditor.h"

BasicSynthEditor::BasicSynthEditor(BasicSynth &p)
    : AudioProcessorEditor(&p),
      processor(p),
      keyboardComponent(processor.keyboardState, MidiKeyboardComponent::horizontalKeyboard)
{
    tooltipWindow->setOpaque(false);

    // Filter Controls
    // =============================================================================================

    filterMode.setName("Mode");
    filterMode.setTooltip("Mode");

    // We can add our combobox menu items by using getRootMenu() to retrieve our PopupMenu
    PopupMenu *menu = filterMode.getRootMenu();
    menu->addItem(1, "Lowpass 12dB");
    menu->addItem(2, "Highpass 12dB");
    menu->addItem(3, "Lowpass 24dB");
    menu->addItem(4, "Highpass 24dB");

    filterMode.setWantsKeyboardFocus(false);
    filterSection.addAndMakeVisible(filterMode);

    comboBoxAttachments.add(
        new ComboBoxAttachment(processor.parameters, BasicSynth::FILTER_MODE, filterMode)
    );

    // We set the name of our components in the constructor so we can use them for drawing text
    // later in our paint() method
    cutoff.setName("Cutoff");
    cutoff.setTooltip("Cutoff");

    // The RotaryHorizontalVerticalDrag style allows the knob to be dragged along either axis
    cutoff.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);

    // We hide all the text boxes in our UI using Slider::NoTextBox
    cutoff.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);

    // We also set all components to not want keyboard focus, so our piano component
    // can catch all key presses
    cutoff.setWantsKeyboardFocus(false);

    filterSection.addAndMakeVisible(cutoff);

    // The SliderAttachment class keeps our slider in sync with one of our parameters.
    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::FILTER_CUTOFF, cutoff)
    );

    resonance.setName("Resonance");
    resonance.setTooltip("Resonance");
    resonance.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    resonance.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    resonance.setWantsKeyboardFocus(false);
    filterSection.addAndMakeVisible(resonance);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::FILTER_RESONANCE, resonance)
    );

    drive.setName("Drive");
    drive.setTooltip("Drive");
    drive.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    drive.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    drive.setWantsKeyboardFocus(false);
    filterSection.addAndMakeVisible(drive);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::FILTER_DRIVE, drive)
    );

    filterSection.setText("Filter");
    addAndMakeVisible(filterSection);

    // Reverb Controls
    // =============================================================================================

    roomSize.setName("Room Size");
    roomSize.setTooltip("Room Size");
    roomSize.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    roomSize.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    roomSize.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(roomSize);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::REVERB_ROOM_SIZE, roomSize)
    );

    damping.setName("Damping");
    damping.setTooltip("Damping");
    damping.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    damping.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    damping.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(damping);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::REVERB_DAMPING, damping)
    );

    width.setName("Width");
    width.setTooltip("Width");
    width.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    width.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    width.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(width);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::REVERB_WIDTH, width)
    );

    freeze.setName("Freeze");
    freeze.setButtonText("Freeze");
    freeze.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(freeze);

    buttonAttachments.add(
        new ButtonAttachment(processor.parameters, BasicSynth::REVERB_FREEZE, freeze)
    );

    dryLevel.setName("Dry");
    dryLevel.setTooltip("Dry");
    dryLevel.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    dryLevel.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    dryLevel.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(dryLevel);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::REVERB_DRY, dryLevel)
    );

    wetLevel.setName("Wet");
    wetLevel.setTooltip("Wet");
    wetLevel.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    wetLevel.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    wetLevel.setWantsKeyboardFocus(false);
    reverbSection.addAndMakeVisible(wetLevel);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::REVERB_WET, wetLevel)
    );

    reverbSection.setText("Reverb");
    addAndMakeVisible(reverbSection);

    // Output Controls
    // =============================================================================================

    outputGain.setName("Output");
    outputGain.setTooltip("Output");
    outputGain.setSliderStyle(Slider::LinearVertical);
    outputGain.setPopupDisplayEnabled(true, true, nullptr);
    outputGain.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    outputGain.setWantsKeyboardFocus(false);
    outputSection.addAndMakeVisible(outputGain);

    sliderAttachments.add(
        new SliderAttachment(processor.parameters, BasicSynth::OUTPUT, outputGain)
    );

    addAndMakeVisible(outputSection);

    // Keyboard
    // =============================================================================================

    // The LookAndFeel associated with our components stores colors by unique identifiers. Each
    // Component has a enum of ColourIDs that it uses to associate part of its drawing with a color.
    // Here we match our keyboard's overlay to the color of our slider thumbs.
    const Colour colour = getLookAndFeel().findColour(Slider::thumbColourId);
    keyboardComponent.setColour(MidiKeyboardComponent::keyDownOverlayColourId,      colour);
    keyboardComponent.setColour(MidiKeyboardComponent::mouseOverKeyOverlayColourId, colour.withAlpha(0.5f));
    addAndMakeVisible(keyboardComponent);

    // =============================================================================================

    // Start a timer so our piano grabs keyboard focus every second
    startTimerHz(1);

    // We set our size at the end of our constructor so our resized() method is called to set up
    // our layout
    setSize(800, 300);
}

void BasicSynthEditor::timerCallback()
{
    // Have the piano component continuously grab keyboard focus so that our keypresses always
    // trigger notes.
    keyboardComponent.grabKeyboardFocus();
}

void BasicSynthEditor::paint (Graphics& g)
{
    // Our component is opaque so we must fill the entire context with a color
    g.fillAll(findColour(ResizableWindow::backgroundColourId));

    // Use the default font - in white - with a height of 10px
    g.setFont(10);
    g.setColour(Colours::white);


    // We iterate over the sliders, using the name we set for each to draw text inside of them
    Slider *sliders[8] = {
        &cutoff,   &resonance, &drive,
        &roomSize, &damping,   &width,
        &dryLevel, &wetLevel
    };

    for (auto slider : sliders)
    {
        g.drawText(
            slider->getName(),

            // Because our sliders are inside of group components we need to get the local area
            // relative to our main editor
            getLocalArea(slider, slider->getLocalBounds()),

            Justification::centred
        );
    }
}

void BasicSynthEditor::resized()
{
    /* This function is called every time the editor has its size set.
       We set up the layout of our child components here.

        Here we get the local bounds of our editor (0, 0, this->width, this->height) and use
        juce::Rectangle<int>::removeFrom...() to cut off sections from the rectangle and return them.
    */

    const int pad = 6;

    Rectangle<int> bounds = getLocalBounds().reduced(pad);

    Rectangle<int> section;

    // Keyboard & MIDI Input
    // =============================================================================================

    keyboardComponent.setBounds(
        bounds
            .removeFromBottom(70)
            .reduced(0, pad / 2)
    );

    bounds.removeFromBottom(30);

    // Output Controls
    // =============================================================================================

    outputSection.setBounds(
        bounds
            .removeFromRight(getWidth() / 10)
    );

    outputGain.setBounds(
        outputSection
            .getLocalBounds()
            .reduced(pad)
    );

    // Reverb Controls
    // =============================================================================================

    reverbSection.setBounds(
        bounds
            .removeFromRight(bounds.getWidth() / 2)
    );

    section = reverbSection
        .getLocalBounds()
        .reduced(pad)
        .withTrimmedTop(pad / 3);

    Rectangle<int> topHalf = section.removeFromTop(section.getHeight() / 2);

    roomSize.setBounds(
        topHalf
            .removeFromLeft(topHalf.getWidth() / 3)
    );

    damping.setBounds(
        topHalf
            .removeFromLeft(topHalf.getWidth() / 2)
    );

    width.setBounds(topHalf);

    freeze.setBounds(
        section.removeFromLeft(section.getWidth() / 3)
    );

    freeze.setBounds(
        freeze
            .getBounds()
            .withSizeKeepingCentre((int)(freeze.getWidth() / 1.5f), freeze.getHeight())
    );

    dryLevel.setBounds(
        section.removeFromLeft(section.getWidth() / 2)
    );

    wetLevel.setBounds(section);

    // Filter Controls
    // =============================================================================================

    filterSection.setBounds(bounds);

    section = filterSection
        .getLocalBounds()
        .reduced(pad)
        .withTrimmedTop(pad / 3);

    filterMode.setBounds(
        section
            .removeFromTop(section.getHeight() / 2)
            .reduced(0, pad)
    );

    filterMode.setBounds(
        filterMode.getBounds()
            .withSizeKeepingCentre(
                filterMode.getWidth() / 2,
                filterMode.getHeight() - (6 * pad)
            )
    );

    cutoff.setBounds(
        section
            .removeFromLeft(section.getWidth() / 3)
    );

    resonance.setBounds(
        section
            .removeFromLeft(section.getWidth() / 2)
    );

    drive.setBounds(section);
}
