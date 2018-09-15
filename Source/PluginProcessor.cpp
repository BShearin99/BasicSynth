#include "PluginProcessor.h"
#include "PluginEditor.h"

const StringRef BasicSynth::FILTER_MODE      = "filter_mode";
const StringRef BasicSynth::FILTER_CUTOFF    = "filter_cutoff";
const StringRef BasicSynth::FILTER_RESONANCE = "filter_resonance";
const StringRef BasicSynth::FILTER_DRIVE     = "filter_drive";

const StringRef BasicSynth::REVERB_ROOM_SIZE = "reverb_room_size";
const StringRef BasicSynth::REVERB_DAMPING   = "reverb_damping";
const StringRef BasicSynth::REVERB_WIDTH     = "reverb_width";
const StringRef BasicSynth::REVERB_FREEZE    = "reverb_freeze";
const StringRef BasicSynth::REVERB_DRY       = "reverb_dry";
const StringRef BasicSynth::REVERB_WET       = "reverb_wet";

const StringRef BasicSynth::OUTPUT           = "output";

BasicSynth::BasicSynth() :
    AudioProcessor(BusesProperties().withOutput("Output", AudioChannelSet::stereo(), true)),
    synthAudioSource(keyboardState),
    parameters(*this, nullptr)
{
    // Filter Parameters
    // =============================================================================================
    parameters.createAndAddParameter(
        // We pass the static ID string we created above to create our parameter
        FILTER_MODE,

        // Assign a formatted name string for the label
        "Filter Mode",

        // This argument is for the parameter's unit label. We leave it empty since the filter
        // mode does not use units
        "",

        // NOTE: JUCE 5.3.2 has a ComboBoxAttachment bug which requires
        // us to use a starting value of 0 here, rather than map it 1-4
        NormalisableRange<float>(0.0f, 3.0f, 1.0f),

        // This is the starting default value for our parameter
        0.0f,

        // createAndAddParameter() accepts lambdas/functors for converting between a numeric
        // value and a formatted string representation. Here we check which position our parameter
        // is at and return the correct filter mode name.
        [](float value)
        {
            switch ((int)value)
            {
                case 0: return "Lowpass 12dB";
                case 1: return "Highpass 12dB";
                case 2: return "Lowpass 24dB";
                case 3: return "Highpass 24dB";
                default: return "";
            }
        },

        // The inverse is that we get passed text and convert it back to a numeric value. If a user
        // were to type in "Highpass 12dB" into a text entry for the parameter it would map to 1.0f.
        [](const String &text)
        {
            if (text == "Lowpass 12dB")
                return 0.0f;
            else if (text == "Highpass 12dB")
                return 1.0f;
            else if (text == "Lowpass 24dB")
                return 2.0f;
            else if (text == "Highpass 24dB")
                return 3.0f;
            else
                return 0.0f;
        },

        // Here we mark specficic attributes about the parameter. The first two default to false
        // and true already, but we must mark this parameter as discrete so the host knows that
        // it should use stepped values

        false, // isMetaParameter
        true,  // isAutomatableParameter
        true   // isDiscrete
    );
    filterMode = parameters.getRawParameterValue(FILTER_MODE);

    parameters.createAndAddParameter(
        FILTER_CUTOFF,
        "Filter Cutoff",
        "Hz",
        NormalisableRange<float>(20.0f, 20000.0f),
        1000.0f,
        nullptr,
        nullptr
    );
    filterCutoff = parameters.getRawParameterValue(FILTER_CUTOFF);

    parameters.createAndAddParameter(
        FILTER_RESONANCE,
        "Filter Resonance",
        "%",
        NormalisableRange<float>(0.0f, 1.0f),
        0.0f,
        nullptr,
        nullptr
    );
    filterResonance = parameters.getRawParameterValue(FILTER_RESONANCE);

    parameters.createAndAddParameter(
        FILTER_DRIVE,
        "Filter Drive",
        "",
        NormalisableRange<float>(1.0f, 10.0f),
        1.0f,
        nullptr,
        nullptr
    );
    filterDrive = parameters.getRawParameterValue(FILTER_DRIVE);

    // Reverb Controls
    // =============================================================================================

    parameters.createAndAddParameter(
        REVERB_ROOM_SIZE,
        "Reverb Room Size",
        "",
        NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        nullptr,
        nullptr
    );
    reverbRoomSize = parameters.getRawParameterValue(REVERB_ROOM_SIZE);

    parameters.createAndAddParameter(
        REVERB_DAMPING,
        "Reverb Damping",
        "%",
        NormalisableRange<float>(0.0f, 1.0f),
        0.0f,
        nullptr,
        nullptr
    );
    reverbDamping = parameters.getRawParameterValue(REVERB_DAMPING);

    parameters.createAndAddParameter(
        REVERB_WIDTH,
        "Reverb Width",
        "%",
        NormalisableRange<float>(0.0f, 1.0f),
        0.0f,
        nullptr,
        nullptr
    );
    reverbWidth = parameters.getRawParameterValue(REVERB_WIDTH);

    parameters.createAndAddParameter(
        REVERB_FREEZE,
        "Reverb Freeze",
        "",
        NormalisableRange<float>(0.0f, 1.0f),
        0.0f,
        nullptr,
        nullptr
    );
    reverbFreeze = parameters.getRawParameterValue(REVERB_FREEZE);

    parameters.createAndAddParameter(
        REVERB_DRY,
        "Reverb Dry Level",
        "%",
        NormalisableRange<float>(0.0f, 1.0f),
        1.0f,
        nullptr,
        nullptr
    );
    reverbDry = parameters.getRawParameterValue(REVERB_DRY);

    parameters.createAndAddParameter(
        REVERB_WET,
        "Reverb Wet Level",
        "%",
        NormalisableRange<float>(0.0f, 1.0f),
        0.5f,
        nullptr,
        nullptr
    );
    reverbWet = parameters.getRawParameterValue(REVERB_WET);

    // Output Control
    // =============================================================================================

    parameters.createAndAddParameter(
        OUTPUT,
        "Output",
        "dB",
        NormalisableRange<float>(-60.0f, 0.0f),
        -6.0f,
        nullptr,
        nullptr
    );
    output = parameters.getRawParameterValue(OUTPUT);

    parameters.state = ValueTree("BasicSynth");
}

BasicSynth::~BasicSynth()
{
}

const String BasicSynth::getName() const
{
    return JucePlugin_Name;
}

bool BasicSynth::acceptsMidi() const
{
    return true;
}

bool BasicSynth::producesMidi() const
{
    return false;
}

bool BasicSynth::isMidiEffect() const
{
    return false;
}

double BasicSynth::getTailLengthSeconds() const
{
    return 0.0;
}

int BasicSynth::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int BasicSynth::getCurrentProgram()
{
    return 0;
}

void BasicSynth::setCurrentProgram(int index)
{
}

const String BasicSynth::getProgramName(int index)
{
    return {};
}

void BasicSynth::changeProgramName(int index, const String& newName)
{
}

void BasicSynth::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    // This function is called whenever the "transport" changes. In a Digital Audio Workstation
    // this happens when you move the playhead. Here in our standalone plugin it only happens
    // when the plugin starts up or if the sample rate or block size are changed.

    // ProcessSpec is a struct that holds information about our current audio processing.
    dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (uint32)samplesPerBlock;
    spec.numChannels      = 2;

    ladderFilter.reset();

    if (*filterMode < 1.0f)
        ladderFilter.setMode(dsp::LadderFilter<float>::Mode::LPF12);
    else if (*filterMode < 2.0f)
        ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF12);
    else if (*filterMode < 3.0f)
        ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF24);
    else
        ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF24);

    lastFilterMode = *filterMode;

    ladderFilter.setCutoffFrequencyHz(*filterCutoff);
    ladderFilter.setResonance(*filterResonance);
    ladderFilter.setDrive(*filterDrive);
    ladderFilter.prepare(spec);

    reverb.reset();
    Reverb::Parameters reverbParams;
    reverbParams.roomSize   = *reverbRoomSize;
    reverbParams.damping    = *reverbDamping;
    reverbParams.wetLevel   = *reverbWet;
    reverbParams.dryLevel   = *reverbDry;
    reverbParams.freezeMode = *reverbFreeze;
    reverb.setParameters(reverbParams);
    reverb.prepare(spec);

    synthAudioSource.prepareToPlay(samplesPerBlock, sampleRate);
}

void BasicSynth::releaseResources()
{
    ladderFilter.reset();
    reverb.reset();
    synthAudioSource.releaseResources();
}

bool BasicSynth::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // This is the place where you check if the layout is supported.
    // In this plugin we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
}

void BasicSynth::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Request the next audio block from our synthesizer audio source. This will parse the current
    // MIDI messages and fill the audio buffer with the synthesized audio signal
    synthAudioSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));

    // When using juce::dsp classes we have to pass our audio buffer as a ProcessContext
    dsp::AudioBlock<float> block(buffer);
    dsp::ProcessContextReplacing<float> context = dsp::ProcessContextReplacing<float>(block);

    if (lastFilterMode != *filterMode)
    {
        if (*filterMode < 1.0f)
            ladderFilter.setMode(dsp::LadderFilter<float>::Mode::LPF12);
        else if (*filterMode < 2.0f)
            ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF12);
        else if (*filterMode < 3.0f)
            ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::LPF24);
        else
            ladderFilter.setMode(juce::dsp::LadderFilter<float>::Mode::HPF24);

        lastFilterMode = *filterMode;
    }

    ladderFilter.setCutoffFrequencyHz(*filterCutoff);
    ladderFilter.setResonance(*filterResonance);
    ladderFilter.setDrive(*filterDrive);
    ladderFilter.process(context);

    Reverb::Parameters reverbParams;
    reverbParams.roomSize   = *reverbRoomSize;
    reverbParams.damping    = *reverbDamping;
    reverbParams.wetLevel   = *reverbWet;
    reverbParams.dryLevel   = *reverbDry;
    reverbParams.freezeMode = *reverbFreeze;
    reverb.setParameters(reverbParams);
    reverb.process(context);

    buffer.applyGain(Decibels::decibelsToGain(*output));
}

bool BasicSynth::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* BasicSynth::createEditor()
{
    return new BasicSynthEditor(*this);
}

void BasicSynth::getStateInformation(MemoryBlock& destData)
{
    // The AudioProcessorValueTreeState class has functions that let us easily store our
    // plugin parameter values as XML data
    auto state = parameters.copyState();
    ScopedPointer<XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void BasicSynth::setStateInformation(const void* data, int sizeInBytes)
{
    ScopedPointer<XmlElement> xml(getXmlFromBinary(data, sizeInBytes));

    if (xml.get() != nullptr)
        if (xml->hasTagName(parameters.state.getType().toString()))
            parameters.replaceState(ValueTree::fromXml(*xml));
}

//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicSynth();
}
