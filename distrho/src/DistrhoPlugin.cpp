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
bool        d_nextPluginIsDummy = false;
bool        d_nextPluginIsSelfTest = false;
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

void PluginPrivateData_init(PluginPrivateData* pData, uint32_t parameterCount, uint32_t programCount, uint32_t stateCount)
{
   #if DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0
    pData->audioPorts = new AudioPortWithBusId[DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS];
   #endif

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

    if (stateCount > 0)
    {
       #if DISTRHO_PLUGIN_WANT_STATE
        pData->stateCount = stateCount;
        pData->states = new State[stateCount];
       #else
        d_stderr2("DPF warning: Plugins with state must define `DISTRHO_PLUGIN_WANT_STATE` to 1");
        DPF_ABORT
       #endif
    }

    #undef DPF_ABORT
}

/* ------------------------------------------------------------------------------------------------------------
 * Host state */

uint32_t plugin_getBufferSize(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->bufferSize;
}

double plugin_getSampleRate(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->sampleRate;
}

const char* plugin_getBundlePath(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->bundlePath;
}

bool plugin_isDummyInstance(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->isDummy;
}

bool plugin_isSelfTestInstance(void* ptr)
{
    PluginPrivateData* pData = getPluginPrivateData(ptr);
    return pData->isSelfTest;
}

#if DISTRHO_PLUGIN_WANT_TIMEPOS
const TimePosition& plugin_getTimePosition()
{
    return pData->timePosition;
}
#endif

#if DISTRHO_PLUGIN_WANT_LATENCY
void plugin_setLatency(const uint32_t frames) noexcept
{
    pData->latency = frames;
}
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
bool plugin_writeMidiEvent(const MidiEvent& midiEvent) noexcept
{
    return pData->writeMidiCallback(midiEvent);
}
#endif

#if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
bool plugin_canRequestParameterValueChanges()
{
    return pData->canRequestParameterValueChanges;
}

bool plugin_requestParameterValueChange(const uint32_t index, const float value) noexcept
{
    return pData->requestParameterValueChangeCallback(index, value);
}
#endif

#if DISTRHO_PLUGIN_WANT_STATE
bool plugin_updateStateValue(const char* const key, const char* const value) noexcept
{
    return pData->updateStateValueCallback(key, value);
}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * Init */

void plugin_initAudioPort(bool input, uint32_t index, AudioPort& port)
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

void plugin_initParameter(uint32_t, Parameter&) {}

void plugin_initPortGroup(const uint32_t groupId, PortGroup& portGroup)
{
    fillInPredefinedPortGroupData(groupId, portGroup);
}

#if DISTRHO_PLUGIN_WANT_PROGRAMS
void plugin_initProgramName(uint32_t, String&) {}
#endif

#if DISTRHO_PLUGIN_WANT_STATE
void plugin_initState(const uint32_t index, State& state)
{
    uint hints = 0x0;
    String stateKey, defaultStateValue;

   #if defined(_MSC_VER)
    #pragma warning(push)
    #pragma warning(disable:4996)
   #elif defined(__clang__)
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"
   #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
   #endif
    initState(index, stateKey, defaultStateValue);
    if (isStateFile(index))
        hints = kStateIsFilenamePath;
   #if defined(_MSC_VER)
    #pragma warning(pop)
   #elif defined(__clang__)
    #pragma clang diagnostic pop
   #elif defined(__GNUC__) && (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 6))
    #pragma GCC diagnostic pop
   #endif

    state.hints = hints;
    state.key = stateKey;
    state.label = stateKey;
    state.defaultValue = defaultStateValue;
}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * Init */

float plugin_getParameterValue(void* ptr, uint32_t) { return 0.0f; }
void plugin_setParameterValue(void* ptr, uint32_t, float) {}

#if DISTRHO_PLUGIN_WANT_PROGRAMS
void plugin_loadProgram(uint32_t) {}
#endif

#if DISTRHO_PLUGIN_WANT_FULL_STATE
String plugin_getState(const char*) { return String(); }
#endif

#if DISTRHO_PLUGIN_WANT_STATE
void plugin_setState(const char*, const char*) {}
#endif

/* ------------------------------------------------------------------------------------------------------------
 * Callbacks (optional) */

void plugin_bufferSizeChanged(void* ptr, uint32_t) {}
void plugin_sampleRateChanged(void* ptr, double) {}

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO
