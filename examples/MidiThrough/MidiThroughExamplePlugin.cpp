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
  Plugin that demonstrates MIDI output in DPF.
 */
struct MidiThroughExamplePlugin
{
    PluginPrivateData data;

    MidiThroughExamplePlugin()
    {
        PluginPrivateData_init(&data, 0, 0, 0);
    }

    DISTRHO_DECLARE_NON_COPYABLE(MidiThroughExamplePlugin)
};

/* --------------------------------------------------------------------------------------------------------
* Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "MidiThrough";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin that demonstrates MIDI output in DPF.";
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
    return d_cconst('d', 'M', 'T', 'r');
}

/* --------------------------------------------------------------------------------------------------------
* Init and Internal data, unused in this plugin */

void plugin_initParameter(void*, uint32_t, Parameter&) {}
void plugin_initPortGroup(void*, const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}
float plugin_getParameterValue(void* ptr, uint32_t index) { return 0; }
void plugin_setParameterValue(void* ptr, uint32_t index, float value) {}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void*) {}
void plugin_deactivate(void*) {}

/**
    Run/process function for plugins with MIDI input.
    In this case we just pass-through all MIDI events.
*/
void plugin_run(void* ptr, const float**, float**, uint32_t, const MidiEvent* midiEvents, uint32_t midiEventCount)
{
    for (uint32_t i=0; i<midiEventCount; ++i)
        plugin_writeMidiEvent(ptr, midiEvents[i]);
}

void plugin_bufferSizeChanged(void* ptr, uint32_t newBufferSize) {}
void plugin_sampleRateChanged(void* ptr, double newSampleRate) {}

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new MidiThroughExamplePlugin();
}

void destroyPlugin(void* ptr)
{
    MidiThroughExamplePlugin* plugin = (MidiThroughExamplePlugin*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    MidiThroughExamplePlugin* plugin = (MidiThroughExamplePlugin*)ptr;
    return &plugin->data;
}