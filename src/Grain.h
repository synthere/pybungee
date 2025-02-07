// Copyright (C) 2020-2024 Parabola Research Limited
// SPDX-License-Identifier: MPL-2.0

#pragma once

#include "Fourier.h"
#include "Output.h"
#include "Partials.h"
#include "Phase.h"
#include "Stretch.h"
#include "Window.h"

#include "bungee/../src/log2.h"
#include "bungee/Bungee.h"

#include <Eigen/Dense>

#include <array>
#include <complex>
#include <memory>
#include <numbers>
#include <random>
#include <sstream>

namespace Bungee {

struct Grain
{
	struct Analysis
	{
		double positionError;
		double hopIdeal;
		double speed;
		int hop; // rounded
	};

	int log2TransformLength;
	Request request;

	double requestHop{};
	bool continuous{};
	int passthrough{};
	int validBinCount{};

	Resample::Operations resampleOperations{};

	InputChunk inputChunk{};
	Analysis analysis{};

	Eigen::ArrayXXcf transformed;
	Eigen::ArrayX<Phase::Type> phase;
	Eigen::ArrayXf energy;
	Eigen::ArrayX<Phase::Type> rotation;
	Eigen::ArrayX<Phase::Type> delta;
	std::vector<Partials::Partial> partials;
	Resample::Padded inputResampled;

	Output::Segment segment;

	Grain(int log2SynthesisHop, int channelCount);

	InputChunk specify(const Request &request, Grain &previous, SampleRates sampleRates, int log2SynthesisHop);

	bool reverse() const
	{
		return analysis.hop < 0;
	}

	bool valid() const
	{
		return !std::isnan(request.position);
	}

	void applyEnvelope();

	auto inputChunkMap(const float *data, std::ptrdiff_t stride)
	{
		typedef Eigen::OuterStride<Eigen::Dynamic> Stride;
		typedef Eigen::Map<Eigen::ArrayXXf, 0, Stride> Map;
		return Map((float *)data, inputChunk.end - inputChunk.begin, transformed.cols(), Stride(stride));
	}

	Eigen::Ref<Eigen::ArrayXXf> resampleInput(Eigen::Ref<Eigen::ArrayXXf> ref, int log2WindowLength);
};

} // namespace Bungee
