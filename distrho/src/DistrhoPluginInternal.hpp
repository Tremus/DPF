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

#ifndef DISTRHO_PLUGIN_INTERNAL_HPP_INCLUDED
#define DISTRHO_PLUGIN_INTERNAL_HPP_INCLUDED

#include "DistrhoPluginInfo.h"
#include "../DistrhoPlugin.hpp"

#ifdef DISTRHO_PLUGIN_TARGET_VST3
# include "DistrhoPluginVST.hpp"
#endif

#include <set>

START_NAMESPACE_DISTRHO

// -----------------------------------------------------------------------
// Maxmimum values

static const uint32_t kMaxMidiEvents = 512;

// -----------------------------------------------------------------------
// Static data, see DistrhoPlugin.cpp

extern uint32_t    d_nextBufferSize;
extern double      d_nextSampleRate;
extern const char* d_nextBundlePath;
extern bool        d_nextCanRequestParameterValueChanges;

// -----------------------------------------------------------------------
// DSP callbacks

typedef bool (*writeMidiFunc) (void* ptr, const MidiEvent& midiEvent);
typedef bool (*requestParameterValueChangeFunc) (void* ptr, uint32_t index, float value);
typedef bool (*updateStateValueFunc) (void* ptr, const char* key, const char* value);

// -----------------------------------------------------------------------
// Helpers

struct AudioPortWithBusId : AudioPort {
    uint32_t busId;

    AudioPortWithBusId()
        : AudioPort(),
          busId(0) {}
};

struct PortGroupWithId : PortGroup {
    uint32_t groupId;

    PortGroupWithId()
        : PortGroup(),
          groupId(kPortGroupNone) {}
};

static inline
void fillInPredefinedPortGroupData(const uint32_t groupId, PortGroup& portGroup)
{
    if (groupId == kPortGroupNone)
    {
        portGroup.name.clear();
        portGroup.symbol.clear();
    }
    else if (groupId == kPortGroupMono)
    {
        portGroup.name = "Mono";
        portGroup.symbol = "dpf_mono";
    }
    else if (groupId == kPortGroupStereo)
    {
        portGroup.name = "Stereo";
        portGroup.symbol = "dpf_stereo";
    }
}

static inline
void d_strncpy(char* const dst, const char* const src, const size_t length)
{
    DISTRHO_SAFE_ASSERT_RETURN(length > 0,);

    if (const size_t len = std::min(std::strlen(src), length-1U))
    {
        std::memcpy(dst, src, len);
        dst[len] = '\0';
    }
    else
    {
        dst[0] = '\0';
    }
}

template<typename T>
static inline
void snprintf_t(char* const dst, const T value, const char* const format, const size_t size)
{
    DISTRHO_SAFE_ASSERT_RETURN(size > 0,);
    std::snprintf(dst, size-1, format, value);
    dst[size-1] = '\0';
}

static inline
void snprintf_f32(char* const dst, const float value, const size_t size)
{
    return snprintf_t<float>(dst, value, "%f", size);
}

static inline
void snprintf_f32(char* const dst, const double value, const size_t size)
{
    return snprintf_t<double>(dst, value, "%f", size);
}

static inline
void snprintf_i32(char* const dst, const int32_t value, const size_t size)
{
    return snprintf_t<int32_t>(dst, value, "%d", size);
}

static inline
void snprintf_u32(char* const dst, const uint32_t value, const size_t size)
{
    return snprintf_t<uint32_t>(dst, value, "%u", size);
}

// -----------------------------------------------------------------------
// Plugin private data

struct PluginPrivateData {
    const bool canRequestParameterValueChanges;
    bool isProcessing;

#if DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0
    AudioPortWithBusId audioPorts[DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS];
#endif

    uint32_t   parameterOffset;
#if DISTRHO_PLUGIN_NUM_PARAMS > 0
    Parameter parameters[DISTRHO_PLUGIN_NUM_PARAMS];
#endif

    uint32_t         portGroupCount;
    PortGroupWithId* portGroups;

#if DISTRHO_PLUGIN_WANT_LATENCY
    uint32_t latency;
#endif

#if DISTRHO_PLUGIN_WANT_TIMEPOS
    TimePosition timePosition;
#endif

    // Callbacks
    void*         callbacksPtr;
    writeMidiFunc writeMidiCallbackFunc;
    requestParameterValueChangeFunc requestParameterValueChangeCallbackFunc;

    // Host state
    // These values will remain constant between plugin_activate() and plugin_deactivate().
    uint32_t bufferSize;
    double   sampleRate;
    // Get the bundle path where the plugin resides.
    // Can be set to null if the plugin is not available in a bundle (if it is a single binary).
    char*    bundlePath;

    PluginPrivateData() noexcept
        : canRequestParameterValueChanges(d_nextCanRequestParameterValueChanges),
          isProcessing(false),
          parameterOffset(0),
          portGroupCount(0),
          portGroups(nullptr),
#if DISTRHO_PLUGIN_WANT_LATENCY
          latency(0),
#endif
          callbacksPtr(nullptr),
          writeMidiCallbackFunc(nullptr),
          requestParameterValueChangeCallbackFunc(nullptr),
          bufferSize(d_nextBufferSize),
          sampleRate(d_nextSampleRate),
          bundlePath(d_nextBundlePath != nullptr ? strdup(d_nextBundlePath) : nullptr)
    {
        DISTRHO_SAFE_ASSERT(bufferSize != 0);
        DISTRHO_SAFE_ASSERT(d_isNotZero(sampleRate));

#if defined(DISTRHO_PLUGIN_TARGET_DSSI) || defined(DISTRHO_PLUGIN_TARGET_LV2)
        parameterOffset += DISTRHO_PLUGIN_NUM_INPUTS + DISTRHO_PLUGIN_NUM_OUTPUTS;
# if DISTRHO_PLUGIN_WANT_LATENCY
        parameterOffset += 1;
# endif
#endif

#ifdef DISTRHO_PLUGIN_TARGET_LV2
# if (DISTRHO_PLUGIN_WANT_MIDI_INPUT || DISTRHO_PLUGIN_WANT_TIMEPOS)
        parameterOffset += 1;
# endif
# if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
        parameterOffset += 1;
# endif
#endif

#ifdef DISTRHO_PLUGIN_TARGET_VST3
        parameterOffset += kVst3InternalParameterCount;
#endif
    }

    ~PluginPrivateData() noexcept
    {
        if (portGroups != nullptr)
        {
            delete[] portGroups;
            portGroups = nullptr;
        }

        if (bundlePath != nullptr)
        {
            std::free(bundlePath);
            bundlePath = nullptr;
        }
    }

#if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
    bool writeMidiCallback(const MidiEvent& midiEvent)
    {
        if (writeMidiCallbackFunc != nullptr)
            return writeMidiCallbackFunc(callbacksPtr, midiEvent);

        return false;
    }
#endif

#if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
    bool requestParameterValueChangeCallback(const uint32_t index, const float value)
    {
        if (requestParameterValueChangeCallbackFunc != nullptr)
            return requestParameterValueChangeCallbackFunc(callbacksPtr, index, value);

        return false;
    }
#endif
};

// -----------------------------------------------------------------------
// Plugin exporter class

class PluginExporter
{
public:
    PluginExporter(void* const callbacksPtr,
                   const writeMidiFunc writeMidiCall,
                   const requestParameterValueChangeFunc requestParameterValueChangeCall)
        : fPlugin(createPlugin()),
          fData(getPluginPrivateData(fPlugin)),
          fIsActive(false)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);

#if defined(DPF_RUNTIME_TESTING) && defined(__GNUC__) && !defined(__clang__)
        /* Run-time testing build.
         * Verify that virtual functions are overriden if parameters, programs or states are in use.
         * This does not work on all compilers, but we use it purely as informational check anyway. */
        if (DISTRHO_PLUGIN_NUM_PARAMS != 0)
        {
            if ((void*)(fPlugin->*(&Plugin::initParameter)) == (void*)&Plugin::initParameter)
            {
                d_stderr2("DPF warning: Plugins with parameters must implement `initParameter`");
                abort();
            }
            if ((void*)(fPlugin->*(&Plugin::getParameterValue)) == (void*)&Plugin::getParameterValue)
            {
                d_stderr2("DPF warning: Plugins with parameters must implement `getParameterValue`");
                abort();
            }
            if ((void*)(fPlugin->*(&Plugin::setParameterValue)) == (void*)&Plugin::setParameterValue)
            {
                d_stderr2("DPF warning: Plugins with parameters must implement `setParameterValue`");
                abort();
            }
        }

#endif

#if DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0
        {
            uint32_t j=0;
# if DISTRHO_PLUGIN_NUM_INPUTS > 0
            for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_INPUTS; ++i, ++j)
                plugin_initAudioPort(fPlugin, true, i, fData->audioPorts[j]);
# endif
# if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
            for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; ++i, ++j)
                plugin_initAudioPort(fPlugin, false, i, fData->audioPorts[j]);
# endif
        }
#endif // DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0

#if DISTRHO_PLUGIN_NUM_PARAMS > 0
        for (uint32_t i=0, count = DISTRHO_PLUGIN_NUM_PARAMS; i < count; ++i)
            plugin_initParameter(fPlugin, i, fData->parameters[i]);

        {
            std::set<uint32_t> portGroupIndices;

#if DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0
            for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
                portGroupIndices.insert(fData->audioPorts[i].groupId);
#endif
            for (uint32_t i=0, count = DISTRHO_PLUGIN_NUM_PARAMS; i < count; ++i)
                portGroupIndices.insert(fData->parameters[i].groupId);

            portGroupIndices.erase(kPortGroupNone);

            if (const uint32_t portGroupSize = static_cast<uint32_t>(portGroupIndices.size()))
            {
                fData->portGroups = new PortGroupWithId[portGroupSize];
                fData->portGroupCount = portGroupSize;

                uint32_t index = 0;
                for (std::set<uint32_t>::iterator it = portGroupIndices.begin(); it != portGroupIndices.end(); ++it, ++index)
                {
                    PortGroupWithId& portGroup(fData->portGroups[index]);
                    portGroup.groupId = *it;

                    if (portGroup.groupId < portGroupSize)
                        plugin_initPortGroup(fPlugin, portGroup.groupId, portGroup);
                    else
                        fillInPredefinedPortGroupData(portGroup.groupId, portGroup);
                }
            }
        }
#endif // DISTRHO_PLUGIN_NUM_PARAMS > 0

        fData->callbacksPtr = callbacksPtr;
        fData->writeMidiCallbackFunc = writeMidiCall;
        fData->requestParameterValueChangeCallbackFunc = requestParameterValueChangeCall;
    }

    ~PluginExporter()
    {
        destroyPlugin(fPlugin);
    }

    // -------------------------------------------------------------------

    const char* getName() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getName(fPlugin);
    }

    const char* getLabel() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getLabel(fPlugin);
    }

    const char* getDescription() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getDescription(fPlugin);
    }

    const char* getMaker() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getMaker(fPlugin);
    }

    const char* getHomePage() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getHomePage(fPlugin);
    }

    const char* getLicense() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, "");

        return plugin_getLicense(fPlugin);
    }

    uint32_t getVersion() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, 0);

        return plugin_getVersion(fPlugin);
    }

    long getUniqueId() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, 0);

        return plugin_getUniqueId(fPlugin);
    }

    void* getInstancePointer() const noexcept
    {
        return fPlugin;
    }

    // -------------------------------------------------------------------

#if DISTRHO_PLUGIN_WANT_LATENCY
    uint32_t getLatency() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);

        return fData->latency;
    }
#endif

#if DISTRHO_PLUGIN_NUM_INPUTS+DISTRHO_PLUGIN_NUM_OUTPUTS > 0
    AudioPortWithBusId& getAudioPort(const bool input, const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, sFallbackAudioPort);

        if (input)
        {
# if DISTRHO_PLUGIN_NUM_INPUTS > 0
            DISTRHO_SAFE_ASSERT_RETURN(index < DISTRHO_PLUGIN_NUM_INPUTS,  sFallbackAudioPort);
# endif
        }
        else
        {
# if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
            DISTRHO_SAFE_ASSERT_RETURN(index < DISTRHO_PLUGIN_NUM_OUTPUTS, sFallbackAudioPort);
# endif
        }

        return fData->audioPorts[index + (input ? 0 : DISTRHO_PLUGIN_NUM_INPUTS)];
    }

    uint32_t getAudioPortHints(const bool input, const uint32_t index) const noexcept
    {
        return getAudioPort(input, index).hints;
    }
    
    uint32_t getAudioPortCountWithGroupId(const bool input, const uint32_t groupId) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);

        uint32_t numPorts = 0;

        if (input)
        {
           #if DISTRHO_PLUGIN_NUM_INPUTS > 0
            for (uint32_t i=0; i<DISTRHO_PLUGIN_NUM_INPUTS; ++i)
            {
                if (fData->audioPorts[i].groupId == groupId)
                    ++numPorts;
            }
           #endif
        }
        else
        {
           #if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
            for (uint32_t i=0; i<DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
            {
                if (fData->audioPorts[i + DISTRHO_PLUGIN_NUM_INPUTS].groupId == groupId)
                    ++numPorts;
            }
           #endif
        }

        return numPorts;
    }
#endif

    uint32_t getParameterCount() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);

        return DISTRHO_PLUGIN_NUM_PARAMS;
    }

    uint32_t getParameterOffset() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);

        return fData->parameterOffset;
    }

    bool isParameterInput(const uint32_t index) const noexcept
    {
        return (getParameterHints(index) & kParameterIsOutput) == 0x0;
    }

    bool isParameterOutput(const uint32_t index) const noexcept
    {
        return (getParameterHints(index) & kParameterIsOutput) != 0x0;
    }

    bool isParameterInteger(const uint32_t index) const noexcept
    {
        return (getParameterHints(index) & kParameterIsInteger) != 0x0;
    }

    bool isParameterTrigger(const uint32_t index) const noexcept
    {
        return (getParameterHints(index) & kParameterIsTrigger) == kParameterIsTrigger;
    }

    bool isParameterOutputOrTrigger(const uint32_t index) const noexcept
    {
        const uint32_t hints = getParameterHints(index);

        if (hints & kParameterIsOutput)
            return true;
        if ((hints & kParameterIsTrigger) == kParameterIsTrigger)
            return true;

        return false;
    }

#if DISTRHO_PLUGIN_NUM_PARAMS > 0
    uint32_t getParameterHints(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, 0x0);
        return fData->parameters[index].hints;
    }

    ParameterDesignation getParameterDesignation(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, kParameterDesignationNull);
        return fData->parameters[index].designation;
    }
    const String& getParameterName(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackString);
        return fData->parameters[index].name;
    }

    const String& getParameterShortName(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackString);
        return fData->parameters[index].shortName;
    }

    const String& getParameterSymbol(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackString);
        return fData->parameters[index].symbol;
    }

    const String& getParameterUnit(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackString);
        return fData->parameters[index].unit;
    }

    const String& getParameterDescription(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackString);
        return fData->parameters[index].description;
    }

    const ParameterEnumerationValues& getParameterEnumValues(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackEnumValues);
        return fData->parameters[index].enumValues;
    }

    const ParameterRanges& getParameterRanges(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, sFallbackRanges);
        return fData->parameters[index].ranges;
    }

    uint8_t getParameterMidiCC(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, 0);
        return fData->parameters[index].midiCC;
    }

    uint32_t getParameterGroupId(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, kPortGroupNone);
        return fData->parameters[index].groupId;
    }

    float getParameterDefault(const uint32_t index) const
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, 0.0f);
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, 0.0f);

        return fData->parameters[index].ranges.def;
    }
#else
    uint32_t getParameterHints(const uint32_t index) const noexcept { return 0; }
    ParameterDesignation getParameterDesignation(const uint32_t index) const noexcept { return kParameterDesignationNull; }
    const String& getParameterName(const uint32_t index) const noexcept { return sFallbackString; }
    const String& getParameterShortName(const uint32_t index) const noexcept { return sFallbackString; }
    const String& getParameterSymbol(const uint32_t index) const noexcept { return sFallbackString; }
    const String& getParameterUnit(const uint32_t index) const noexcept { return sFallbackString; }
    const String& getParameterDescription(const uint32_t index) const noexcept { return sFallbackString; }
    const ParameterEnumerationValues& getParameterEnumValues(const uint32_t index) const noexcept { return sFallbackEnumValues; }
    const ParameterRanges& getParameterRanges(const uint32_t index) const noexcept { return sFallbackRanges; }
    uint8_t getParameterMidiCC(const uint32_t index) const noexcept { return 0; }
    uint32_t getParameterGroupId(const uint32_t index) const noexcept { return kPortGroupNone; }
    float getParameterDefault(const uint32_t index) const { return 0.0f; }
#endif // DISTRHO_PLUGIN_NUM_PARAMS > 0

    float getParameterValue(const uint32_t index) const
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr, 0.0f);
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS, 0.0f);

        return plugin_getParameterValue(fPlugin, index);
    }

    void setParameterValue(const uint32_t index, const float value)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < DISTRHO_PLUGIN_NUM_PARAMS,);

        plugin_setParameterValue(fPlugin, index, value);
    }

    uint32_t getPortGroupCount() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);

        return fData->portGroupCount;
    }

    const PortGroupWithId& getPortGroupById(const uint32_t groupId) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && fData->portGroupCount != 0, sFallbackPortGroup);

        for (uint32_t i=0; i < fData->portGroupCount; ++i)
        {
            const PortGroupWithId& portGroup(fData->portGroups[i]);

            if (portGroup.groupId == groupId)
                return portGroup;
        }

        return sFallbackPortGroup;
    }

    const PortGroupWithId& getPortGroupByIndex(const uint32_t index) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr && index < fData->portGroupCount, sFallbackPortGroup);

        return fData->portGroups[index];
    }

    const String& getPortGroupSymbolForId(const uint32_t groupId) const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, sFallbackString);

        return getPortGroupById(groupId).symbol;
    }

#if DISTRHO_PLUGIN_WANT_TIMEPOS
    void setTimePosition(const TimePosition& timePosition) noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);

        std::memcpy(&fData->timePosition, &timePosition, sizeof(TimePosition));
    }
#endif

    // -------------------------------------------------------------------

    void activate()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(! fIsActive,);

        fIsActive = true;
        plugin_activate(fPlugin);
    }

    void deactivate()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fIsActive,);

        fIsActive = false;
        plugin_deactivate(fPlugin);
    }

    void deactivateIfNeeded()
    {
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);

        if (fIsActive)
        {
            fIsActive = false;
            plugin_deactivate(fPlugin);
        }
    }

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    void run(const float** const inputs, float** const outputs, const uint32_t frames,
             const MidiEvent* const midiEvents, const uint32_t midiEventCount)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);

        if (! fIsActive)
        {
            fIsActive = true;
            plugin_activate(fPlugin);
        }

        fData->isProcessing = true;
        plugin_run(fPlugin, inputs, outputs, frames, midiEvents, midiEventCount);
        fData->isProcessing = false;
    }
#else
    void run(const float** const inputs, float** const outputs, const uint32_t frames)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);

        if (! fIsActive)
        {
            fIsActive = true;
            plugin_activate(fPlugin);
        }

        fData->isProcessing = true;
        plugin_run(fPlugin, inputs, outputs, frames);
        fData->isProcessing = false;
    }
#endif

    // -------------------------------------------------------------------

    uint32_t getBufferSize() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0);
        return fData->bufferSize;
    }

    double getSampleRate() const noexcept
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr, 0.0);
        return fData->sampleRate;
    }

    void setBufferSize(const uint32_t bufferSize, const bool doCallback = false)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT(bufferSize >= 2);

        if (fData->bufferSize == bufferSize)
            return;

        fData->bufferSize = bufferSize;

        if (doCallback)
        {
            if (fIsActive) plugin_deactivate(fPlugin);
            plugin_bufferSizeChanged(fPlugin, bufferSize);
            if (fIsActive) plugin_activate(fPlugin);
        }
    }

    void setSampleRate(const double sampleRate, const bool doCallback = false)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fData != nullptr,);
        DISTRHO_SAFE_ASSERT_RETURN(fPlugin != nullptr,);
        DISTRHO_SAFE_ASSERT(sampleRate > 0.0);

        if (d_isEqual(fData->sampleRate, sampleRate))
            return;

        fData->sampleRate = sampleRate;

        if (doCallback)
        {
            if (fIsActive) plugin_deactivate(fPlugin);
            plugin_sampleRateChanged(fPlugin, sampleRate);
            if (fIsActive) plugin_activate(fPlugin);
        }
    }

    // -------------------------------------------------------------------
    // Plugin and DistrhoPlugin data

    void* const fPlugin;
    PluginPrivateData* const fData;
    bool fIsActive;

    // -------------------------------------------------------------------
    // Static fallback data, see DistrhoPlugin.cpp

private:
    static const String                     sFallbackString;
    static /* */ AudioPortWithBusId         sFallbackAudioPort;
    static const ParameterRanges            sFallbackRanges;
    static const ParameterEnumerationValues sFallbackEnumValues;
    static const PortGroupWithId            sFallbackPortGroup;

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginExporter)
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_PLUGIN_INTERNAL_HPP_INCLUDED
