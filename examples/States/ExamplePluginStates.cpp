/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2015 Filipe Coelho <falktx@falktx.com>
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

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------------------------------------------

/**
  Simple plugin to demonstrate state usage (including UI).
  The plugin will be treated as an effect, but it will not change the host audio.
 */
struct ExamplePluginStates
{
    PluginPrivateData data;

    ExamplePluginStates()
    {
        // 0 parameters, 2 programs, 9 states
        PluginPrivateData_init(&data, 0, 2, 9);
       /**
          Initialize all our parameters to their defaults.
          In this example all default values are false, so we can simply zero them.
        */
        std::memset(fParamGrid, 0, sizeof(bool)*9);
    }

   /**
      Our parameters used to display the grid on/off states.
    */
    bool fParamGrid[9];

private:
   /**
      Set our plugin class as non-copyable and add a leak detector just in case.
    */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExamplePluginStates)
};

/* --------------------------------------------------------------------------------------------------------
* Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "states";
}

const char* plugin_getDescription(void* ptr)
{
    return "Simple plugin to demonstrate state usage (including UI).\n\
The plugin will be treated as an effect, but it will not change the host audio.";
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
    return d_cconst('d', 'S', 't', 's');
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

void plugin_initParameter(void*, uint32_t, Parameter&) {}

void plugin_initPortGroup(void*, const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}

void plugin_initProgramName(void* ptr, uint32_t index, String& programName)
{
    switch (index)
    {
    case 0:
        programName = "Default";
        break;
    case 1:
        programName = "Custom";
        break;
    }
}

/**
    Initialize the state @a index.@n
    This function will be called once, shortly after the plugin is created.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_STATE is enabled.
*/
void plugin_initState(void* ptr, uint32_t index, State& state)
{
    switch (index)
    {
    case 0:
        state.key = "top-left";
        state.label = "Top Left";
        break;
    case 1:
        state.key = "top-center";
        state.label = "Top Center";
        break;
    case 2:
        state.key = "top-right";
        state.label = "Top Right";
        break;
    case 3:
        state.key = "middle-left";
        state.label = "Middle Left";
        break;
    case 4:
        state.key = "middle-center";
        state.label = "Middle Center";
        break;
    case 5:
        state.key = "middle-right";
        state.label = "Middle Right";
        break;
    case 6:
        state.key = "bottom-left";
        state.label = "Bottom Left";
        break;
    case 7:
        state.key = "bottom-center";
        state.label = "Bottom Center";
        break;
    case 8:
        state.key = "bottom-right";
        state.label = "Bottom Right";
        break;
    }

    state.hints = kStateIsHostWritable;
    state.defaultValue = "false";
}

/* --------------------------------------------------------------------------------------------------------
* Internal data */

float plugin_getParameterValue(void* ptr, uint32_t index)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    return plugin->fParamGrid[index];
}

void plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    plugin->fParamGrid[index] = value;
}

void plugin_loadProgram(void* ptr, uint32_t index)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    switch (index)
    {
    case 0:
        plugin->fParamGrid[0] = false;
        plugin->fParamGrid[1] = false;
        plugin->fParamGrid[2] = false;
        plugin->fParamGrid[3] = false;
        plugin->fParamGrid[4] = false;
        plugin->fParamGrid[5] = false;
        plugin->fParamGrid[6] = false;
        plugin->fParamGrid[7] = false;
        plugin->fParamGrid[8] = false;
        break;
    case 1:
        plugin->fParamGrid[0] = true;
        plugin->fParamGrid[1] = true;
        plugin->fParamGrid[2] = false;
        plugin->fParamGrid[3] = false;
        plugin->fParamGrid[4] = true;
        plugin->fParamGrid[5] = true;
        plugin->fParamGrid[6] = true;
        plugin->fParamGrid[7] = false;
        plugin->fParamGrid[8] = true;
        break;
    }
}

/**
    Get the value of an internal state.
    The host may call this function from any non-realtime context.
*/
String plugin_getState(void* ptr, const char* key)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;

    static const String sTrue ("true");
    static const String sFalse("false");

    // check which block changed
    if (std::strcmp(key, "top-left") == 0)
        return plugin->fParamGrid[0] ? sTrue : sFalse;
    else if (std::strcmp(key, "top-center") == 0)
        return plugin->fParamGrid[1] ? sTrue : sFalse;
    else if (std::strcmp(key, "top-right") == 0)
        return plugin->fParamGrid[2] ? sTrue : sFalse;
    else if (std::strcmp(key, "middle-left") == 0)
        return plugin->fParamGrid[3] ? sTrue : sFalse;
    else if (std::strcmp(key, "middle-center") == 0)
        return plugin->fParamGrid[4] ? sTrue : sFalse;
    else if (std::strcmp(key, "middle-right") == 0)
        return plugin->fParamGrid[5] ? sTrue : sFalse;
    else if (std::strcmp(key, "bottom-left") == 0)
        return plugin->fParamGrid[6] ? sTrue : sFalse;
    else if (std::strcmp(key, "bottom-center") == 0)
        return plugin->fParamGrid[7] ? sTrue : sFalse;
    else if (std::strcmp(key, "bottom-right") == 0)
        return plugin->fParamGrid[8] ? sTrue : sFalse;

    return sFalse;
}

/**
    Change an internal state.
*/
void plugin_setState(void* ptr, const char* key, const char* value)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    const bool valueOnOff = (std::strcmp(value, "true") == 0);

    // check which block changed
    if (std::strcmp(key, "top-left") == 0)
        plugin->fParamGrid[0] = valueOnOff;
    else if (std::strcmp(key, "top-center") == 0)
        plugin->fParamGrid[1] = valueOnOff;
    else if (std::strcmp(key, "top-right") == 0)
        plugin->fParamGrid[2] = valueOnOff;
    else if (std::strcmp(key, "middle-left") == 0)
        plugin->fParamGrid[3] = valueOnOff;
    else if (std::strcmp(key, "middle-center") == 0)
        plugin->fParamGrid[4] = valueOnOff;
    else if (std::strcmp(key, "middle-right") == 0)
        plugin->fParamGrid[5] = valueOnOff;
    else if (std::strcmp(key, "bottom-left") == 0)
        plugin->fParamGrid[6] = valueOnOff;
    else if (std::strcmp(key, "bottom-center") == 0)
        plugin->fParamGrid[7] = valueOnOff;
    else if (std::strcmp(key, "bottom-right") == 0)
        plugin->fParamGrid[8] = valueOnOff;
}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void*) {}
void plugin_deactivate(void*) {}

/* --------------------------------------------------------------------------------------------------------
* Process */

/**
    Run/process function for plugins without MIDI input.
*/
void plugin_run(void*, const float** inputs, float** outputs, uint32_t frames)
{
    /**
        This plugin does nothing, it just demonstrates state usage.
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

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new ExamplePluginStates();
}

void destroyPlugin(void* ptr)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    ExamplePluginStates* plugin = (ExamplePluginStates*)ptr;
    return &plugin->data;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
