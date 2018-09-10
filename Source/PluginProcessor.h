#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct BasicSynth  : public AudioProcessor
{
    static const StringRef FILTER_MODE;
    static const StringRef FILTER_CUTOFF;
    static const StringRef FILTER_RESONANCE;
    static const StringRef FILTER_DRIVE;

    static const StringRef REVERB_ROOM_SIZE;
    static const StringRef REVERB_DAMPING;
    static const StringRef REVERB_WIDTH;
    static const StringRef REVERB_FREEZE;
    static const StringRef REVERB_DRY;
    static const StringRef REVERB_WET;

    static const StringRef OUTPUT;

    // =============================================================================================

    BasicSynth();
    ~BasicSynth();

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    AudioProcessorValueTreeState parameters;
};
