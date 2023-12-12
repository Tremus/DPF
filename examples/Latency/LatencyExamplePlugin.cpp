/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2022 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "DistrhoPlugin.hpp"
#include "src/DistrhoPluginInternal.hpp"

/**
  Plugin that demonstrates the latency API in DPF.
 */
struct LatencyExamplePlugin
{
    PluginPrivateData data;

    LatencyExamplePlugin()
        : data(),
          fLatency(1.0f),
          fLatencyInFrames(0),
          fBuffer(nullptr),
          fBufferPos(0),
          fBufferSize(0)
    {
        PluginPrivateData_init(&data, 1, 0), // 1 parameter
        // allocates buffer
        plugin_sampleRateChanged(this, data.sampleRate);
    }

    ~LatencyExamplePlugin()
    {
        delete[] fBuffer;
    }

    // Parameters
    float fLatency;
    uint32_t fLatencyInFrames;

    // Buffer for previous audio, size depends on sample rate
    float* fBuffer;
    uint32_t fBufferPos, fBufferSize;

    DISTRHO_DECLARE_NON_COPYABLE(LatencyExamplePlugin)
};

/* --------------------------------------------------------------------------------------------------------
* Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "Latency";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin that demonstrates the latency API in DPF.";
}

const char* plugin_getMaker(void* ptr)
{
    return "DISTRHO";
}

const char* plugin_getHomePage(void* ptr)
{
    return "https://github.com/DISTRHO/DPF";
}

const char* plugin_getLicense(void* ptr)
{
    return "ISC";
}

uint32_t plugin_getVersion(void* ptr)
{
    return d_version(1, 0, 0);
}

int64_t plugin_getUniqueId(void* ptr)
{
    return d_cconst('d', 'L', 'a', 't');
}

/* --------------------------------------------------------------------------------------------------------
* Init */

void plugin_initAudioPort(void* ptr, bool input, uint32_t index, AudioPort& port)
{
    // mark the (single) latency audio port as mono
    port.groupId = kPortGroupMono;

    // everything else is as default
    plugin_default_initAudioPort(input, index, port);
}

void plugin_initParameter(void*, uint32_t index, Parameter& parameter)
{
    if (index != 0)
        return;

    parameter.hints  = kParameterIsAutomatable;
    parameter.name   = "Latency";
    parameter.symbol = "latency";
    parameter.unit   = "s";
    parameter.ranges.def = 1.0f;
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 5.0f;
}

void plugin_initPortGroup(void*, const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}

/* --------------------------------------------------------------------------------------------------------
* Internal data */

float plugin_getParameterValue(void* ptr, uint32_t index)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    if (index != 0)
        return 0.0f;

    return plugin->fLatency;
}

void plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    if (index != 0)
        return;

    plugin->fLatency = value;
    plugin->fLatencyInFrames = value * plugin->data.sampleRate;
    plugin_setLatency(ptr, plugin->fLatencyInFrames);
}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void* ptr)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    plugin->fBufferPos = 0;
    std::memset(plugin->fBuffer, 0, sizeof(float) * plugin->fBufferSize);
}

void plugin_deactivate(void*) {}


void plugin_run(void* ptr, const float** inputs, float** outputs, uint32_t frames)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    const float* const in  = inputs[0];
    float* const       out = outputs[0];

    if (plugin->fLatencyInFrames == 0)
    {
        if (out != in)
            std::memcpy(out, in, sizeof(float)*frames);
        return;
    }

    // Put the new audio in the buffer.
    std::memcpy(plugin->fBuffer + plugin->fBufferPos, in, sizeof(float)*frames);
    plugin->fBufferPos += frames;

    // buffer is not filled enough yet
    if (plugin->fBufferPos < plugin->fLatencyInFrames+frames)
    {
        // silence output
        std::memset(out, 0, sizeof(float)*frames);
    }
    // buffer is ready to copy
    else
    {
        // copy latency buffer to output
        const uint32_t readPos = plugin->fBufferPos - plugin->fLatencyInFrames-frames;
        std::memcpy(out, plugin->fBuffer+readPos, sizeof(float) * frames);

        // move latency buffer back by some frames
        std::memmove(plugin->fBuffer, plugin->fBuffer+frames, sizeof(float) * plugin->fBufferPos);
        plugin->fBufferPos -= frames;
    }
}

/* --------------------------------------------------------------------------------------------------------
* Callbacks (optional) */

void plugin_bufferSizeChanged(void* ptr, uint32_t newBufferSize) {}

void plugin_sampleRateChanged(void* ptr, double newSampleRate)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    plugin->fBufferSize = newSampleRate * 6; // 6 seconds

    delete[] plugin->fBuffer;
    plugin->fBuffer = new float[plugin->fBufferSize];
    // buffer reset is done during activate()

    plugin->fLatencyInFrames = plugin->fLatency * newSampleRate;
    plugin_setLatency(ptr, plugin->fLatencyInFrames);
}

// -------------------------------------------------------------------------------------------------------

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new LatencyExamplePlugin();
}

void destroyPlugin(void* ptr)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    LatencyExamplePlugin* plugin = (LatencyExamplePlugin*)ptr;
    return &plugin->data;
}