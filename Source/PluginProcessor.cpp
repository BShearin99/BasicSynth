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
        FILTER_MODE,
        "Filter Mode",
        "",
        // NOTE: JUCE 5.3.2 still has the ComboBoxAttachment bug which requires
        // us to use a starting value of 0 here, rather than map it 1-4
        NormalisableRange<float>(0.0f, 3.0f, 1.0f),
        0.0f,
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

    parameters.state = ValueTree("Basic Synth");
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
    dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (uint32)samplesPerBlock;
    spec.numChannels      = 2;

    ladderFilter.reset();

    // TODO: Filter mode

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
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
}

void BasicSynth::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
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

    synthAudioSource.getNextAudioBlock(AudioSourceChannelInfo(buffer));

    dsp::AudioBlock<float> block(buffer);
    dsp::ProcessContextReplacing<float> context = dsp::ProcessContextReplacing<float>(block);

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
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void BasicSynth::setStateInformation(const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasicSynth();
}
