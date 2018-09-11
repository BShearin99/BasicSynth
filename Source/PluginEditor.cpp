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

    cutoff.setName("Cutoff");
    cutoff.setTooltip("Cutoff");
    cutoff.setSliderStyle(Slider::RotaryHorizontalVerticalDrag);
    cutoff.setTextBoxStyle(Slider::NoTextBox, true, 0, 0);
    cutoff.setWantsKeyboardFocus(false);
    filterSection.addAndMakeVisible(cutoff);

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

    // Keyboard & MIDI Input
    // =============================================================================================

    const juce::Colour colour = getLookAndFeel().findColour(Slider::thumbColourId);
    keyboardComponent.setColour(MidiKeyboardComponent::keyDownOverlayColourId,      colour);
    keyboardComponent.setColour(MidiKeyboardComponent::mouseOverKeyOverlayColourId, colour.withAlpha(0.5f));
    addAndMakeVisible(keyboardComponent);

    addAndMakeVisible(midiInputList);

    // =============================================================================================

    setSize(800, 300);
}

void BasicSynthEditor::paint (Graphics& g)
{
    g.fillAll(findColour(ResizableWindow::backgroundColourId));

    Slider *sliders[8] = {
        &cutoff, &resonance, &drive,
        &roomSize, &damping, &width, &dryLevel, &wetLevel
    };

    g.setColour(Colours::white);
    g.setFont(10);

    for (auto slider : sliders)
    {
        g.drawText(
            slider->getName(),
            getLocalArea(slider, slider->getLocalBounds()),
            Justification::centred
        );
    }
}

void BasicSynthEditor::resized()
{
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

    midiInputList.setBounds(
        bounds
            .removeFromBottom(30)
            .reduced(0, pad / 2)
    );

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
