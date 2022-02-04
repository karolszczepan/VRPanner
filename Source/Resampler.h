#pragma once

#include <vector>

class Resampler
{
public:

    std::vector<float> resample(
        const float* input, 
        std::size_t inputSize, 
        float inputSampleRate, 
        float outputSampleRate);

    /// Returns the required size of an output buffer, given an input
    std::size_t requiredOutputSize(
        const float* input, 
        std::size_t inputSize,
        float inputSampleRate,
        float outputSampleRate);

    /// Returns the number of output samples written
    /*static std::size_t resample(
        const float* input,
        std::size_t inputSize,
        float* output,
        std::size_t outputSize,
        float inputSampleRate,
        float outputSampleRate);*/
};

