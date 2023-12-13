/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2018 Filipe Coelho <falktx@falktx.com>
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
  Plugin to demonstrate parameter outputs using meters.
 */
struct ExamplePluginMeters
{
    PluginPrivateData data;

    ExamplePluginMeters()
        : data(),
          fColor(0.0f),
          fOutLeft(0.0f),
          fOutRight(0.0f),
          fNeedsReset(true)
    {
    }

   /**
      Parameters.
    */
    float fColor, fOutLeft, fOutRight;

   /**
      Boolean used to reset meter values.
      The UI will send a "reset" message which sets this as true.
    */
    volatile bool fNeedsReset;

    DISTRHO_DECLARE_NON_COPYABLE(ExamplePluginMeters)
};

/* --------------------------------------------------------------------------------------------------------
* Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "meters";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin to demonstrate parameter outputs using meters.";
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
    return d_cconst('d', 'M', 't', 'r');
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
    /**
        All parameters in this plugin have the same ranges.
    */
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.0f;

    /**
        Set parameter data.
    */
    switch (index)
    {
    case 0:
        parameter.hints  = kParameterIsAutomatable|kParameterIsInteger;
        parameter.name   = "color";
        parameter.symbol = "color";
        parameter.enumValues.count = 2;
        parameter.enumValues.restrictedMode = true;
        {
            ParameterEnumerationValue* const values = new ParameterEnumerationValue[2];
            parameter.enumValues.values = values;

            values[0].label = "Green";
            values[0].value = METER_COLOR_GREEN;
            values[1].label = "Blue";
            values[1].value = METER_COLOR_BLUE;
        }
        break;
    case 1:
        parameter.hints  = kParameterIsAutomatable|kParameterIsOutput;
        parameter.name   = "out-left";
        parameter.symbol = "out_left";
        break;
    case 2:
        parameter.hints  = kParameterIsAutomatable|kParameterIsOutput;
        parameter.name   = "out-right";
        parameter.symbol = "out_right";
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
    ExamplePluginMeters* plugin = (ExamplePluginMeters*)ptr;
    switch (index)
    {
    case 0: return plugin->fColor;
    case 1: return plugin->fOutLeft;
    case 2: return plugin->fOutRight;
    }

    return 0.0f;
}

void plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    ExamplePluginMeters* plugin = (ExamplePluginMeters*)ptr;
    // this is only called for input paramters, and we only have one of those.
    if (index != 0) return;

    plugin->fColor = value;
}

/* --------------------------------------------------------------------------------------------------------
* Process */

void plugin_activate(void*) {}
void plugin_deactivate(void*) {}

void plugin_run(void* ptr, const float** inputs, float** outputs, uint32_t frames)
{
    ExamplePluginMeters* plugin = (ExamplePluginMeters*)ptr;
    float tmp;
    float tmpLeft  = 0.0f;
    float tmpRight = 0.0f;

    for (uint32_t i=0; i<frames; ++i)
    {
        // left
        tmp = std::abs(inputs[0][i]);

        if (tmp > tmpLeft)
            tmpLeft = tmp;

        // right
        tmp = std::abs(inputs[1][i]);

        if (tmp > tmpRight)
            tmpRight = tmp;
    }

    if (tmpLeft > 1.0f)
        tmpLeft = 1.0f;
    if (tmpRight > 1.0f)
        tmpRight = 1.0f;

    if (plugin->fNeedsReset)
    {
        plugin->fOutLeft  = tmpLeft;
        plugin->fOutRight = tmpRight;
        plugin->fNeedsReset = false;
    }
    else
    {
        if (tmpLeft > plugin->fOutLeft)
            plugin->fOutLeft = tmpLeft;
        if (tmpRight > plugin->fOutRight)
            plugin->fOutRight = tmpRight;
    }

    // copy inputs over outputs if needed
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);

    if (outputs[1] != inputs[1])
        std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);
}

void plugin_bufferSizeChanged(void* ptr, uint32_t newBufferSize) {}
void plugin_sampleRateChanged(void* ptr, double newSampleRate) {}

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new ExamplePluginMeters();
}

void destroyPlugin(void* ptr)
{
    ExamplePluginMeters* plugin = (ExamplePluginMeters*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    ExamplePluginMeters* plugin = (ExamplePluginMeters*)ptr;
    return &plugin->data;
}