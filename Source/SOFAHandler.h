#pragma once
#include "SOFA.h"
#include <JuceHeader.h>

/************************************************************************************/
/*!
 *  @brief          Helper function to  access element [i][j][k] of a "3D array" of dimensions [dim1][dim2][dim3]
 *                  stored in a 1D data array
 *
 */
 /************************************************************************************/
static inline const std::size_t array3DIndex(const unsigned long i,
    const unsigned long j,
    const unsigned long k,
    const unsigned long dim1,
    const unsigned long dim2,
    const unsigned long dim3)
{
    return dim2 * dim3 * i + dim3 * j + k;
}

/************************************************************************************/
/*!
 *  @brief          Helper function to  access element [i][j] of a "2D array" of dimensions [dim1][dim2]
 *                  stored in a 1D data array
 *
 */
 /************************************************************************************/
static inline const std::size_t array2DIndex(const unsigned long i,
    const unsigned long j,
    const unsigned long dim1,
    const unsigned long dim2)
{
    return dim2 * i + j;
}

class SOFAHandler /*: public juce::ResamplingAudioSource*/
{
public:
    SOFAHandler::SOFAHandler(const std::string& path);

    void init(double sampleRate);

    void getIR(juce::AudioBuffer<float>& buffer, double ver, double hor);
    std::size_t getIRSize();


private:
    std::size_t measurementsNr{ 0 };
    std::size_t receiversNr{ 0 };
    std::size_t samplesNr{ 0 };

    //double sampleRate;
    std::vector<float> irData;
    std::unique_ptr<sofa::SimpleFreeFieldHRIR> hrir;

    std::vector<std::size_t> dims;

    std::size_t anglesToPos(double ver, double hor);
};

