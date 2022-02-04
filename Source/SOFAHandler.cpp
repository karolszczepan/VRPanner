#include "SOFAHandler.h"
#include "Resampler.h"

SOFAHandler::SOFAHandler(const std::string& path) :
	measurementsNr{ 0 },
	receiversNr{ 0 },
	samplesNr{ 0 }
{
	try
	{
		sofa::File file(path);
		const bool isSOFA = file.IsValid();
		if (isSOFA) {
			hrir = std::make_unique<sofa::SimpleFreeFieldHRIR>(path);
		}
		file.GetVariableDimensions(dims, "SourcePosition");
		SOFA_ASSERT(dims.size() == 2);
		std::vector< double > pos;
		pos.resize(dims[0] * dims[1]);
		file.GetSourcePosition(&pos[0], dims[0], dims[1]);
	}
	catch (std::exception& e)
	{
		std::cerr << "exception occured : " << e.what() << std::endl;
		exit(1);
	}
}

void SOFAHandler::init(double sampleRate)
{
	double sofaSamplerRate;
	if (!hrir->GetSamplingRate(sofaSamplerRate)) {
		return;
	}
	else {
		measurementsNr = static_cast<std::size_t>(hrir->GetNumMeasurements());
		receiversNr = static_cast<std::size_t>(hrir->GetNumReceivers());
		samplesNr = static_cast<std::size_t>(hrir->GetNumDataSamples());

		std::vector<double> data;
		hrir->GetDataIR(data);

		if (sofaSamplerRate != sampleRate) {
			Resampler resampler;
			std::vector<float> resampled;
			std::vector<float> floatData(data.begin(), data.end());
			float* dataPtr = floatData.data();
			
			for (std::size_t i = 0; i < data.size(); i += samplesNr) {
				auto chunk = resampler.resample(dataPtr, samplesNr, sofaSamplerRate, sampleRate);
				resampled.insert(resampled.end(), chunk.begin(), chunk.end());
			}
			samplesNr = resampler.requiredOutputSize(dataPtr, samplesNr, sofaSamplerRate, sampleRate);
			// irData = std::vector<double>(resampled.begin(), resampled.end());
			irData = resampled;
		}
		else {
			irData = std::vector<float>(data.begin(), data.end());;
		}
	}
}

void SOFAHandler::getIR(juce::AudioBuffer<float>& buffer, double ver, double hor)
{
	std::size_t start = anglesToPos(ver, hor);

	float* ptr = buffer.getWritePointer(0);
	for (std::size_t i = 0; i < samplesNr; ++i) {
		 *ptr++ = irData[start + i];
	}

	start += samplesNr;
	ptr = buffer.getWritePointer(1);
	for (std::size_t i = 0; i < samplesNr; ++i) {
		*ptr++ = irData[start + i];
	}
}

std::size_t SOFAHandler::getIRSize()
{
	return samplesNr;
}

std::size_t SOFAHandler::anglesToPos(double ver, double hor)
{
	std::size_t index0 = static_cast<unsigned>((ver + 45.0) / 15.0);
	jassert(index0 < 7);
	std::size_t index1 = static_cast<unsigned>(hor / 15.0);
	jassert(index0 < 24);

	return index0 * receiversNr * samplesNr * 24 + index1 * samplesNr * receiversNr;
}