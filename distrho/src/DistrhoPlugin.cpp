/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2023 Filipe Coelho <falktx@falktx.com>
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

#include "DistrhoPluginInternal.hpp"

START_NAMESPACE_DISTRHO

/* ------------------------------------------------------------------------------------------------------------
 * Static data, see DistrhoPluginInternal.hpp */

uint32_t    d_nextBufferSize = 0;
double      d_nextSampleRate = 0.0;
const char* d_nextBundlePath = nullptr;
bool        d_nextCanRequestParameterValueChanges = false;

/* ------------------------------------------------------------------------------------------------------------
 * Static fallback data, see DistrhoPluginInternal.hpp */

const String                     PluginExporter::sFallbackString;
/* */ AudioPortWithBusId         PluginExporter::sFallbackAudioPort;
const ParameterRanges            PluginExporter::sFallbackRanges;
const ParameterEnumerationValues PluginExporter::sFallbackEnumValues;
const PortGroupWithId            PluginExporter::sFallbackPortGroup;

/* ------------------------------------------------------------------------------------------------------------
 * Plugin */

void PluginPrivateData_init(PluginPrivateData* pData, uint32_t parameterCount, uint32_t programCount)
{
   #if defined(DPF_ABORT_ON_ERROR) || defined(DPF_RUNTIME_TESTING)
    #define DPF_ABORT abort();
   #else
    #define DPF_ABORT
   #endif

    if (parameterCount > 0)
    {
        pData->parameterCount = parameterCount;
        pData->parameters = new Parameter[parameterCount];
    }

    if (programCount > 0)
    {
       #if DISTRHO_PLUGIN_WANT_PROGRAMS
        pData->programCount = programCount;
        pData->programNames = new String[programCount];
       #else
        d_stderr2("DPF warning: Plugins with programs must define `DISTRHO_PLUGIN_WANT_PROGRAMS` to 1");
        DPF_ABORT
       #endif
    }

    #undef DPF_ABORT
}

/* ------------------------------------------------------------------------------------------------------------
 * Host state */

#if DISTRHO_PLUGIN_WANT_TIMEPOS
const TimePosition& plugin_getTimePosition(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->timePosition;
}
#endif

#if DISTRHO_PLUGIN_WANT_LATENCY
void plugin_setLatency(void* ptr, const uint32_t frames)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    pData->latency = frames;
}
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
bool plugin_writeMidiEvent(void* ptr, const MidiEvent& midiEvent)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->writeMidiCallback(midiEvent);
}
#endif

#if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
bool plugin_canRequestParameterValueChanges(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->canRequestParameterValueChanges;
}

bool plugin_requestParameterValueChange(void* ptr, const uint32_t index, const float value)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->requestParameterValueChangeCallback(index, value);
}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * Init */

void plugin_default_initAudioPort(bool input, uint32_t index, AudioPort& port)
{
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

void plugin_default_initPortGroup(const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
