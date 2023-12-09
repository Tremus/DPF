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
  Simple plugin to demonstrate parameter usage (including UI).
  The plugin will be treated as an effect, but it will not change the host audio.
 */
struct ExamplePluginParameters
{
    PluginPrivateData data;

    ExamplePluginParameters()
    {
        // 9 parameters, 2 programs, 0 states
        PluginPrivateData_init(&data, 9, 2, 0);
       /**
          Initialize all our parameters to their defaults.
          In this example all parameters have 0 as default, so we can simply zero them.
        */
        std::memset(fParamGrid, 0, sizeof(float)*9);
    }

   /**
      Our parameters are used to display a 3x3 grid like this:
       0 1 2
       3 4 5
       6 7 8

      The index matches its grid position.
    */
    float fParamGrid[9];

   /**
      Set our plugin class as non-copyable and add a leak detector just in case.
    */
    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ExamplePluginParameters)
};

/* --------------------------------------------------------------------------------------------------------
* Information */

extern const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

/**
    Get the plugin label.
    A plugin label follows the same rules as Parameter::symbol, with the exception that it can start with numbers.
*/
const char* plugin_getLabel(void* ptr)
{
    return "parameters";
}

/**
    Get an extensive comment/description about the plugin.
*/
const char* plugin_getDescription(void* ptr)
{
    return "Simple plugin to demonstrate parameter usage (including UI).\n\
The plugin will be treated as an effect, but it will not change the host audio.";
}

/**
    Get the plugin author/maker.
*/
const char* plugin_getMaker(void* ptr)
{
    return "DISTRHO";
}

/**
    Get the plugin homepage.
*/
const char* plugin_getHomePage(void* ptr)
{
    return "https://github.com/DISTRHO/DPF";
}

/**
    Get the plugin license name (a single line of text).
    For commercial plugins this should return some short copyright information.
*/
const char* plugin_getLicense(void* ptr)
{
    return "ISC";
}

/**
    Get the plugin version, in hexadecimal.
*/
uint32_t plugin_getVersion(void* ptr)
{
    return d_version(1, 0, 0);
}

/**
    Get the plugin unique Id.
    This value is used by LADSPA, DSSI and VST plugin formats.
*/
int64_t plugin_getUniqueId(void* ptr)
{
    return d_cconst('d', 'P', 'r', 'm');
}

/* --------------------------------------------------------------------------------------------------------
* Init */

enum {
    kPortGroupTop = 0,
    kPortGroupMiddle,
    kPortGroupBottom
};

/**
    Initialize the audio port @a index.@n
    This function will be called once, shortly after the plugin is created.
*/
void plugin_initAudioPort(void* ptr, bool input, uint32_t index, AudioPort& port)
{
    // treat meter audio ports as stereo
    port.groupId = kPortGroupStereo;

    // Set defaults
    if (port.hints & kAudioPortIsCV)
    {
        port.name    = input ? "CV Input " : "CV Output ";
        port.name   += String(index+1);
        port.symbol  = input ? "cv_in_" : "cv_out_";
        port.symbol += String(index+1);
    }
    else
    {
        port.name    = input ? "Audio Input " : "Audio Output ";
        port.name   += String(index+1);
        port.symbol  = input ? "audio_in_" : "audio_out_";
        port.symbol += String(index+1);
    }
}

/**
    Initialize the parameter @a index.
    This function will be called once, shortly after the plugin is created.
*/
void plugin_initParameter(void* ptr, uint32_t index, Parameter& parameter)
{
    /**
        All parameters in this plugin are similar except for name.
        As such, we initialize the common details first, then set the unique name later.
    */

    /**
        Changing parameters does not cause any realtime-unsafe operations, so we can mark them as automatable.
        Also set as boolean because they work as on/off switches.
    */
    parameter.hints = kParameterIsAutomatable|kParameterIsBoolean;

    /**
        Minimum 0 (off), maximum 1 (on).
        Default is off.
    */
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 1.0f;
    parameter.ranges.def = 0.0f;

    /**
        Set the (unique) parameter name.
        @see fParamGrid
    */
    switch (index)
    {
    case 0:
        parameter.name = "top-left";
        parameter.groupId = kPortGroupTop;
        break;
    case 1:
        parameter.name = "top-center";
        parameter.groupId = kPortGroupTop;
        break;
    case 2:
        parameter.name = "top-right";
        parameter.groupId = kPortGroupTop;
        break;
    case 3:
        parameter.name = "middle-left";
        parameter.groupId = kPortGroupMiddle;
        break;
    case 4:
        parameter.name = "middle-center";
        parameter.groupId = kPortGroupMiddle;
        break;
    case 5:
        parameter.name = "middle-right";
        parameter.groupId = kPortGroupMiddle;
        break;
    case 6:
        parameter.name = "bottom-left";
        parameter.groupId = kPortGroupBottom;
        break;
    case 7:
        parameter.name = "bottom-center";
        parameter.groupId = kPortGroupBottom;
        break;
    case 8:
        parameter.name = "bottom-right";
        parameter.groupId = kPortGroupBottom;
        break;
    }

    /**
        Our parameter names are valid symbols except for "-".
    */
    parameter.symbol = parameter.name;
    parameter.symbol.replace('-', '_');
}

/**
    Initialize the port group @a groupId.@n
    This function will be called once,
    shortly after the plugin is created and all audio ports and parameters have been enumerated.
*/
void plugin_initPortGroup(void* ptr, uint32_t groupId, PortGroup& portGroup)
{
    switch (groupId) {
    case kPortGroupTop:
        portGroup.name = "Top";
        portGroup.symbol = "top";
        break;
    case kPortGroupMiddle:
        portGroup.name = "Middle";
        portGroup.symbol = "middle";
        break;
    case kPortGroupBottom:
        portGroup.name = "Bottom";
        portGroup.symbol = "bottom";
        break;
    }
}

/**
    Set the name of the program @a index.
    This function will be called once, shortly after the plugin is created.
*/
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

/* --------------------------------------------------------------------------------------------------------
* Internal data */

/**
    Get the current value of a parameter.
*/
float plugin_plugin_getParameterValue(void* ptr, uint32_t index)
{
    ExamplePluginParameters* plugin = (ExamplePluginParameters*)ptr;

    return plugin->fParamGrid[index];
}

/**
    Change a parameter value.
*/
void plugin_plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    ExamplePluginParameters* plugin = (ExamplePluginParameters*)ptr;
    plugin->fParamGrid[index] = value;
}

/**
    Load a program.
    The host may call this function from any context, including realtime processing.
*/
void plugin_loadProgram(void* ptr, uint32_t index)
{
    ExamplePluginParameters* plugin = (ExamplePluginParameters*)ptr;

    switch (index)
    {
    case 0:
        plugin->fParamGrid[0] = 0.0f;
        plugin->fParamGrid[1] = 0.0f;
        plugin->fParamGrid[2] = 0.0f;
        plugin->fParamGrid[3] = 0.0f;
        plugin->fParamGrid[4] = 0.0f;
        plugin->fParamGrid[5] = 0.0f;
        plugin->fParamGrid[6] = 0.0f;
        plugin->fParamGrid[7] = 0.0f;
        plugin->fParamGrid[8] = 0.0f;
        break;
    case 1:
        plugin->fParamGrid[0] = 1.0f;
        plugin->fParamGrid[1] = 1.0f;
        plugin->fParamGrid[2] = 0.0f;
        plugin->fParamGrid[3] = 0.0f;
        plugin->fParamGrid[4] = 1.0f;
        plugin->fParamGrid[5] = 1.0f;
        plugin->fParamGrid[6] = 1.0f;
        plugin->fParamGrid[7] = 0.0f;
        plugin->fParamGrid[8] = 1.0f;
        break;
    }
}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

/**
    Activate this plugin.
*/
void plugin_activate(void*) {}

/**
    Deactivate this plugin.
*/
void plugin_deactivate(void*) {}

/* --------------------------------------------------------------------------------------------------------
* Process */

/**
    Run/process function for plugins without MIDI input.
*/
void plugin_run(void* ptr, const float** inputs, float** outputs, uint32_t frames)
{
    /**
        This plugin does nothing, it just demonstrates parameter usage.
        So here we directly copy inputs over outputs, leaving the audio untouched.
        We need to be careful in case the host re-uses the same buffer for both inputs and outputs.
    */
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);

    if (outputs[1] != inputs[1])
        std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);
}

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new ExamplePluginParameters();
}

void destroyPlugin(void* ptr)
{
    ExamplePluginParameters* plugin = (ExamplePluginParameters*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    ExamplePluginParameters* plugin = (ExamplePluginParameters*)ptr;
    return &plugin->data;
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
