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

#include "DistrhoPluginInfo.h"
#include "DistrhoPlugin.hpp"
#include "src/DistrhoPluginInternal.hpp"


/**
  Plugin to show how to get some basic information sent to the UI.
 */
struct InfoExamplePlugin
{
    PluginPrivateData data;

    InfoExamplePlugin()
    {
        PluginPrivateData_init(&data, 0);

        // clear all parameters
        std::memset(fParameters, 0, sizeof(fParameters));

        // we can know some things right at the start
        fParameters[kParameterBufferSize] = data.bufferSize;
        fParameters[kParameterCanRequestParameterValueChanges] = plugin_canRequestParameterValueChanges(this);
    }

    // Parameters
    float fParameters[kParameterCount];

    DISTRHO_DECLARE_NON_COPYABLE(InfoExamplePlugin)
};

/* --------------------------------------------------------------------------------------------------------
 * Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "Info";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin to show how to get some basic information sent to the UI.";
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
    return d_cconst('d', 'N', 'f', 'o');
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
    parameter.hints      = kParameterIsAutomatable|kParameterIsOutput;
    parameter.ranges.def = 0.0f;
    parameter.ranges.min = 0.0f;
    parameter.ranges.max = 16777216.0f;

    switch (index)
    {
    case kParameterBufferSize:
        parameter.name   = "BufferSize";
        parameter.symbol = "buffer_size";
        break;
    case kParameterCanRequestParameterValueChanges:
        parameter.name   = "Parameter Changes";
        parameter.symbol = "parameter_changes";
        parameter.hints |= kParameterIsBoolean;
        parameter.ranges.max = 1.0f;
        break;
    case kParameterTimePlaying:
        parameter.name   = "TimePlaying";
        parameter.symbol = "time_playing";
        parameter.hints |= kParameterIsBoolean;
        parameter.ranges.max = 1.0f;
        break;
    case kParameterTimeFrame:
        parameter.name   = "TimeFrame";
        parameter.symbol = "time_frame";
        break;
    case kParameterTimeValidBBT:
        parameter.name   = "TimeValidBBT";
        parameter.symbol = "time_validbbt";
        parameter.hints |= kParameterIsBoolean;
        parameter.ranges.max = 1.0f;
        break;
    case kParameterTimeBar:
        parameter.name   = "TimeBar";
        parameter.symbol = "time_bar";
        break;
    case kParameterTimeBeat:
        parameter.name   = "TimeBeat";
        parameter.symbol = "time_beat";
        break;
    case kParameterTimeTick:
        parameter.name   = "TimeTick";
        parameter.symbol = "time_tick";
        break;
    case kParameterTimeBarStartTick:
        parameter.name   = "TimeBarStartTick";
        parameter.symbol = "time_barstarttick";
        break;
    case kParameterTimeBeatsPerBar:
        parameter.name   = "TimeBeatsPerBar";
        parameter.symbol = "time_beatsperbar";
        break;
    case kParameterTimeBeatType:
        parameter.name   = "TimeBeatType";
        parameter.symbol = "time_beattype";
        break;
    case kParameterTimeTicksPerBeat:
        parameter.name   = "TimeTicksPerBeat";
        parameter.symbol = "time_ticksperbeat";
        break;
    case kParameterTimeBeatsPerMinute:
        parameter.name   = "TimeBeatsPerMinute";
        parameter.symbol = "time_beatsperminute";
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
    InfoExamplePlugin* plugin = (InfoExamplePlugin*)ptr;
    return plugin->fParameters[index];
}

void plugin_setParameterValue(void* ptr, uint32_t index, float value)
{
    // this is only called for input parameters, which we have none of.
}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void* ptr) {}
void plugin_deactivate(void*) {}

/**
    Run/process function for plugins without MIDI input.
    @note Some parameters might be null if there are no audio inputs or outputs.
*/
void plugin_run(void* ptr, const float** inputs, float** outputs, uint32_t frames)
{
    InfoExamplePlugin* plugin = (InfoExamplePlugin*)ptr;
    /**
        This plugin does nothing, it just demonstrates information usage.
        So here we directly copy inputs over outputs, leaving the audio untouched.
        We need to be careful in case the host re-uses the same buffer for both inputs and outputs.
    */
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);

    if (outputs[1] != inputs[1])
        std::memcpy(outputs[1], inputs[1], sizeof(float)*frames);

    // get time position
    const TimePosition& timePos(plugin_getTimePosition(ptr));

    // set basic values
    plugin->fParameters[kParameterTimePlaying]  = timePos.playing ? 1.0f : 0.0f;
    plugin->fParameters[kParameterTimeFrame]    = timePos.frame;
    plugin->fParameters[kParameterTimeValidBBT] = timePos.bbt.valid ? 1.0f : 0.0f;

    // set bbt
    if (timePos.bbt.valid)
    {
        plugin->fParameters[kParameterTimeBar]            = timePos.bbt.bar;
        plugin->fParameters[kParameterTimeBeat]           = timePos.bbt.beat;
        plugin->fParameters[kParameterTimeTick]           = timePos.bbt.tick;
        plugin->fParameters[kParameterTimeBarStartTick]   = timePos.bbt.barStartTick;
        plugin->fParameters[kParameterTimeBeatsPerBar]    = timePos.bbt.beatsPerBar;
        plugin->fParameters[kParameterTimeBeatType]       = timePos.bbt.beatType;
        plugin->fParameters[kParameterTimeTicksPerBeat]   = timePos.bbt.ticksPerBeat;
        plugin->fParameters[kParameterTimeBeatsPerMinute] = timePos.bbt.beatsPerMinute;
    }
    else
    {
        plugin->fParameters[kParameterTimeBar]            = 0.0f;
        plugin->fParameters[kParameterTimeBeat]           = 0.0f;
        plugin->fParameters[kParameterTimeTick]           = 0.0f;
        plugin->fParameters[kParameterTimeBarStartTick]   = 0.0f;
        plugin->fParameters[kParameterTimeBeatsPerBar]    = 0.0f;
        plugin->fParameters[kParameterTimeBeatType]       = 0.0f;
        plugin->fParameters[kParameterTimeTicksPerBeat]   = 0.0f;
        plugin->fParameters[kParameterTimeBeatsPerMinute] = 0.0f;
    }
}

/* --------------------------------------------------------------------------------------------------------
* Callbacks (optional) */

void plugin_bufferSizeChanged(void* ptr, uint32_t newBufferSize)
{
    InfoExamplePlugin* plugin = (InfoExamplePlugin*)ptr;
    plugin->fParameters[kParameterBufferSize] = newBufferSize;
}

void plugin_sampleRateChanged(void* ptr, double newSampleRate) {}

// -------------------------------------------------------------------------------------------------------

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new InfoExamplePlugin();
}

void destroyPlugin(void* ptr)
{
    InfoExamplePlugin* plugin = (InfoExamplePlugin*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    InfoExamplePlugin* plugin = (InfoExamplePlugin*)ptr;
    return &plugin->data;
}