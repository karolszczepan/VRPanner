/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
VrPanner2AudioProcessor::VrPanner2AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), params(*this, nullptr, "Parameters", createParams()),
                       verAngle{ 0.0f },
                       horAngle{ 0.0f }
                       
#endif
{
    sofa = std::make_unique<SOFAHandler>("D:\\Studia\\Magisterka\\DzwiekPrzestrzenny\\VrPanner2\\sofas\\listen_irc_1022.sofa");
}

VrPanner2AudioProcessor::~VrPanner2AudioProcessor()
{
}

//==============================================================================
const juce::String VrPanner2AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VrPanner2AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VrPanner2AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VrPanner2AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VrPanner2AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VrPanner2AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VrPanner2AudioProcessor::getCurrentProgram()
{
    return 0;
}

void VrPanner2AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VrPanner2AudioProcessor::getProgramName (int index)
{
    return {};
}

void VrPanner2AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VrPanner2AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    this->sampleRate = sampleRate;
    sofa->init(sampleRate);
    auto size = std::max(sofa->getIRSize(), static_cast<std::size_t>(samplesPerBlock));
    ir.setSize(getTotalNumOutputChannels(), size);
    sofa->getIR(ir, 0, 0);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = size;
    spec.numChannels = 2;

    auto tempIR = ir;
    
    conv.prepare(spec);
    conv.loadImpulseResponse(std::move(tempIR), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
}

void VrPanner2AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VrPanner2AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VrPanner2AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }

    auto newVer = params.getRawParameterValue("vertical")->load();
    auto newHor = params.getRawParameterValue("horizontal")->load();

    if (newVer != verAngle || newHor != horAngle) {
        verAngle = newVer;
        horAngle = newHor;
        sofa->getIR(ir, newVer, newHor);
        auto tempIR = ir;
        conv.loadImpulseResponse(std::move(tempIR), sampleRate, juce::dsp::Convolution::Stereo::yes, juce::dsp::Convolution::Trim::no, juce::dsp::Convolution::Normalise::no);
    }

    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    conv.process(context);
    auto a = conv.getCurrentIRSize();

    /*for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        // ..do something to the data...
    }*/
}

//==============================================================================
bool VrPanner2AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VrPanner2AudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void VrPanner2AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VrPanner2AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VrPanner2AudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout VrPanner2AudioProcessor::createParams() {


    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("vertical", "Vertical", juce::NormalisableRange<float> { -45.0f, 45.0f, }, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("horizontal", "Horizontal", juce::NormalisableRange<float> { 0.0f, 359.9f, }, 0.0f));
    //params.push_back(std::make_unique<juce::AudioParameterFloat>("wet_dry", "Wet/Dry", juce::NormalisableRange<float> { 0.0f, 1.0f, }, 0.3f));
    //juce::NormalisableRange<float> range{ 20.0f, 20000.0f };
    //range.setSkewForCentre(20.0f + (20000.0f - 20.0f) * 0.1f);

    return { params.begin(), params.end() };
}
