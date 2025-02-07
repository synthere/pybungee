// Copyright (C) 2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#include "bungee/Bungee.h"
#include "bungee/Push.h"

#define CXXOPTS_NO_EXCEPTIONS
#include "cxxopts.hpp"
#undef CXXOPTS_NO_EXCEPTIONS

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace Bungee::FunctionData {

static void fail(const char *message)
{
	std::cerr << message << "\n";
	exit(1);
}
struct Parameter
{
	std::vector<float> *inputData;
    int inSampleRate;
    std::vector<float> *outputData;
    int outSampleRate;
    int channelCount;
};
struct Processor
{
	SampleRates sampleRates;
	int inputFrameCount;
	int inputFramesPad;
	int inputChannelStride;
	int channelCount;
	std::vector<float> *inputBuffer;// (-1,1)
    std::vector<float> *outputBuffer;
    int outputPos;

	Processor(Parameter &para, Request &request) 
	{
        sampleRates.input = para.inSampleRate;
        sampleRates.output = para.outSampleRate;

		inputFrameCount = para.inputData->size();
        channelCount = para.channelCount;

		inputFramesPad = 1 << 12;
		inputChannelStride = inputFrameCount; //inputFramesPad + inputFrameCount + inputFramesPad;

		inputBuffer = para.inputData;
        //memcpy(&inputBuffer[inputFramesPad], &(*para.inputData)[inputFramesPad], inputFrameCount);
  
        std::cout<<"output:"<<para.outputData->size()<<std::endl;
        //para.outputData->resize(nOutput);
        outputBuffer = para.outputData;
        outputPos = 0;
		restart(request);
	}

	void restart(Request &request)
	{
		if (request.speed < 0)
			request.position = inputFrameCount - 1;
		else
			request.position = 0.;
	}

	bool write(OutputChunk outputChunk)
	{
		double position[2];
		position[OutputChunk::begin] = outputChunk.request[OutputChunk::begin]->position;
		position[OutputChunk::end] = outputChunk.request[OutputChunk::end]->position;

		if (!std::isnan(position[OutputChunk::begin]))
		{
			double nPrerollInput = outputChunk.request[OutputChunk::begin]->speed < 0. ? position[OutputChunk::begin] - inputFrameCount + 1 : -position[OutputChunk::begin];
			nPrerollInput = std::max<int>(0., std::round(nPrerollInput));

			const int nPrerollOutput = std::round(nPrerollInput * (outputChunk.frameCount / std::abs(position[OutputChunk::end] - position[OutputChunk::begin])));

			if (outputChunk.frameCount > nPrerollOutput)
			{
				outputChunk.frameCount -= nPrerollOutput;
				outputChunk.data += nPrerollOutput;
				return writeChunk(outputChunk);
			}
		}

		return false;
	}

	const float *getInputAudio(InputChunk inputChunk) const
	{
		const float *audio = nullptr;
		if (inputChunk.begin != inputChunk.end)
		{
			inputChunk.begin += inputFramesPad;
			inputChunk.end += inputFramesPad;
			if (inputChunk.begin < 0)
			{
				inputChunk.begin -= inputChunk.begin;
				inputChunk.end -= inputChunk.begin;
			}
			else if (inputChunk.end > inputChannelStride)
			{
				inputChunk.begin -= inputChunk.end - inputChannelStride;
				inputChunk.end -= inputChunk.end - inputChannelStride;
			}
			audio = &(*inputBuffer)[inputChunk.begin];
		}
		return audio;
	}

	void getInputAudio(float *p, int stride, int position, int length) const
	{
		const float *source = getInputAudio(InputChunk{position, position + length});
		for (int c = 0; c < channelCount; ++c)
			for (int i = 0; i < length; ++i)
				p[c * stride + i] = source[c * inputChannelStride + i];
	}

	bool writeSamples(Bungee::OutputChunk chunk)
	{
		//const int count = chunk.frameCount * channelCount;
        const int count = std::min<int>(chunk.frameCount * channelCount, (*outputBuffer).size() - outputPos);

        //printf("outputsize:%d, counta;%d, chnstride:%d, op:%d\n", (*outputBuffer).size(), count, chunk.channelStride, outputPos);
		for (int f = 0; f < count / channelCount && outputPos < (*outputBuffer).size(); ++f)
			for (int c = 0; c < channelCount; ++c)
			{
                (*outputBuffer)[outputPos] = chunk.data[f + c * chunk.channelStride];
                outputPos +=1;
			}

        //printf("outupos:%d, %d\n", outputPos,(*outputBuffer).size());
		return outputPos >= (*outputBuffer).size();
	}

	bool writeChunk(Bungee::OutputChunk chunk)
	{
		return writeSamples(chunk);
	}

	void writeOutputFile()
	{

	}


};
	template <typename Type>
	static inline Type read(const char *data)
	{
		Type value = 0;
		for (unsigned i = 0; i < sizeof(Type); ++i)
			value |= (Type(data[i]) & 0xff) << 8 * i;
		return value;
	}

	template <typename Type>
	static inline void write(char *data, Type value)
	{
		for (unsigned i = 0; i < sizeof(Type); ++i)
			data[i] = value >> 8 * i;
	}

	template <typename Sample>
	static inline float toFloat(Sample x)
	{
		constexpr float k = -1.f / std::numeric_limits<Sample>::min();
		return k * x;
	}

	template <typename Sample>
	static inline Sample fromFloat(float x)
	{
		x = std::ldexp(x, 8 * sizeof(Sample) - 1);
		x = std::round(x);
		if (x < std::numeric_limits<Sample>::min())
			return std::numeric_limits<Sample>::min();
		if (x >= -(float)std::numeric_limits<Sample>::min())
			return std::numeric_limits<Sample>::max();
		return x;
	}
} // namespace Bungee::FunctionCall
