#include "Resampler.h"
#include <samplerate.h>

std::vector<float> Resampler::resample(
    const float* input,
    std::size_t inputSize,
    float inputSampleRate,
    float outputSampleRate)
{
    std::size_t targetSize = outputSampleRate * inputSize / inputSampleRate + 1;

    std::vector<float> output(targetSize);

    SRC_DATA sr_data; 
    sr_data.data_in = input;
    sr_data.data_out = output.data();
    sr_data.input_frames = static_cast<long>(inputSize);
    sr_data.output_frames = static_cast<long>(targetSize);
    sr_data.src_ratio = outputSampleRate / inputSampleRate;

    if (src_simple(&sr_data, SRC_SINC_MEDIUM_QUALITY, 1)) {
        return {};
    }

    return output;
}

std::size_t Resampler::requiredOutputSize(
    const float* input,
    std::size_t inputSize,
    float inputSampleRate,
    float outputSampleRate)
{
    return outputSampleRate * inputSize / inputSampleRate + 1;
}

/*std::size_t Resampler::resample(
    const float* input,
    std::size_t inputSize,
    float* output,
    std::size_t outputSize,
    float inputSampleRate,
    float outputSampleRate)
{
    SRC_DATA sr_data{
        input,
        output,
        static_cast<long>(inputSize),
        static_cast<long>(outputSize),
        outputSampleRate / inputSampleRate
    };

    if (src_simple(&sr_data, SRC_SINC_MEDIUM_QUALITY, 1)) {
        return 0;
    }

    return static_cast<std::size_t>(sr_data.output_frames_gen);
}*/