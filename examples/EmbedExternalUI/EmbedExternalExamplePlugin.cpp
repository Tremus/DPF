/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2021 Filipe Coelho <falktx@falktx.com>
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
  Plugin to show how to get some basic information sent to the UI.
 */
struct EmbedExternalExamplePlugin
{
    PluginPrivateData data;

    EmbedExternalExamplePlugin()
        : data(),
          fWidth(512.0f),
          fHeight(256.0f)
    {
        PluginPrivateData_init(&data, 0);
    }

    // Parameters
    float fWidth, fHeight;

    DISTRHO_DECLARE_NON_COPYABLE(EmbedExternalExamplePlugin)
};

/* --------------------------------------------------------------------------------------------------------
 * Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "EmbedExternalUI";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin to show how to use an embedable dpf-external UI.";
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
    return d_cconst('d', 'b', 'x', 't');
}

/* --------------------------------------------------------------------------------------------------------
* Init */

void plugin_initAudioPort(void* ptr, bool input, uint32_t index, AudioPort& port)
{
    // treat meter audio ports as stereo
    port.groupId = kPortGroupStereo;

    // everything else is as default
    plugin_default_initAudioPort(input, index, port);
}

void plugin_initParameter(void*, uint32_t index, Parameter& parameter)
{
    switch (index)
    {
    case kParameterWidth:
        parameter.hints      = kParameterIsAutomatable|kParameterIsInteger;
        parameter.ranges.def = 512.0f;
        parameter.ranges.min = 256.0f;
        parameter.ranges.max = 4096.0f;
        parameter.name   = "Width";
        parameter.symbol = "width";
        parameter.unit   = "px";
        break;
    case kParameterHeight:
        parameter.hints      = kParameterIsAutomatable|kParameterIsInteger;
        parameter.ranges.def = 256.0f;
        parameter.ranges.min = 256.0f;
        parameter.ranges.max = 4096.0f;
        parameter.name   = "Height";
        parameter.symbol = "height";
        parameter.unit   = "px";
        break;
    }
}

void plugin_initPortGroup(void*, const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}

/* --------------------------------------------------------------------------------------------------------
* Internal data */

float plugin_getParameterValue(void* ptr, uint32_t index)
{
    EmbedExternalExamplePlugin* plugin = (EmbedExternalExamplePlugin*)ptr;
    switch (index)
    {
    case kParameterWidth:
        return plugin->fWidth;
    case kParameterHeight:
        return plugin->fHeight;
    }

    return 0.0f;

}

void plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    EmbedExternalExamplePlugin* plugin = (EmbedExternalExamplePlugin*)ptr;
    switch (index)
    {
    case kParameterWidth:
        plugin->fWidth = value;
        break;
    case kParameterHeight:
        plugin->fHeight = value;
        break;
    }
}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void* ptr) {}
void plugin_deactivate(void*) {}

void plugin_run(void* ptr, const float** inputs, float** outputs, uint32_t frames)
{
    /**
        This plugin does nothing, it just demonstrates information usage.
        So here we directly copy inputs over outputs, leaving the audio untouched.
        We need to be careful in case the host re-uses the same buffer for both inputs and outputs.
    */
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);
    if (outputs[1] != inputs[1])
        std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);
}

void plugin_bufferSizeChanged(void* ptr, uint32_t newBufferSize) {}
void plugin_sampleRateChanged(void* ptr, double newSampleRate) {}

// -------------------------------------------------------------------------------------------------------

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new EmbedExternalExamplePlugin();
}

void destroyPlugin(void* ptr)
{
    EmbedExternalExamplePlugin* plugin = (EmbedExternalExamplePlugin*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    EmbedExternalExamplePlugin* plugin = (EmbedExternalExamplePlugin*)ptr;
    return &plugin->data;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
