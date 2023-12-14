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

#include "DistrhoPluginInternal.hpp"

#include "lv2/atom.h"
#include "lv2/atom-forge.h"
#include "lv2/atom-util.h"
#include "lv2/buf-size.h"
#include "lv2/data-access.h"
#include "lv2/instance-access.h"
#include "lv2/midi.h"
#include "lv2/options.h"
#include "lv2/parameters.h"
#include "lv2/patch.h"
#include "lv2/state.h"
#include "lv2/time.h"
#include "lv2/urid.h"
#include "lv2/worker.h"
#include "lv2/lv2_kxstudio_properties.h"
#include "lv2/lv2_programs.h"
#include "lv2/control-input-port-change-request.h"

#ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
# include "libmodla.h"
#endif

#include <map>

#ifndef DISTRHO_PLUGIN_URI
# error DISTRHO_PLUGIN_URI undefined!
#endif

#ifndef DISTRHO_PLUGIN_LV2_STATE_PREFIX
# define DISTRHO_PLUGIN_LV2_STATE_PREFIX "urn:distrho:"
#endif

#define DISTRHO_LV2_USE_EVENTS_IN  (DISTRHO_PLUGIN_WANT_MIDI_INPUT || DISTRHO_PLUGIN_WANT_TIMEPOS)
#define DISTRHO_LV2_USE_EVENTS_OUT DISTRHO_PLUGIN_WANT_MIDI_OUTPUT

START_NAMESPACE_DISTRHO

typedef std::map<const String, String> StringToStringMap;
typedef std::map<const LV2_URID, String> UridToStringMap;

#if ! DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
static const writeMidiFunc writeMidiCallback = nullptr;
#endif
#if ! DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
static const requestParameterValueChangeFunc requestParameterValueChangeCallback = nullptr;
#endif

// -----------------------------------------------------------------------

class PluginLv2
{
public:
    PluginLv2(const double sampleRate,
              const LV2_URID_Map* const uridMap,
              const LV2_Worker_Schedule* const worker,
              const LV2_ControlInputPort_Change_Request* const ctrlInPortChangeReq,
              const bool usingNominal)
        : fPlugin(this, writeMidiCallback, requestParameterValueChangeCallback),
          fUsingNominal(usingNominal),
#ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
          fRunCount(0),
#endif
          fPortControls(nullptr),
          fLastControlValues(nullptr),
          fSampleRate(sampleRate),
          fURIDs(uridMap),
#if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
          fCtrlInPortChangeReq(ctrlInPortChangeReq),
#endif
          fUridMap(uridMap),
          fWorker(worker)
    {
#if DISTRHO_PLUGIN_NUM_INPUTS > 0
        for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_INPUTS; ++i)
            fPortAudioIns[i] = nullptr;
#else
        fPortAudioIns = nullptr;
#endif

#if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
        for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
            fPortAudioOuts[i] = nullptr;
#else
        fPortAudioOuts = nullptr;
#endif

        if (const uint32_t count = fPlugin.getParameterCount())
        {
            fPortControls      = new float*[count];
            fLastControlValues = new float[count];

            for (uint32_t i=0; i < count; ++i)
            {
                fPortControls[i] = nullptr;
                fLastControlValues[i] = fPlugin.getParameterValue(i);
            }
        }
        else
        {
            fPortControls      = nullptr;
            fLastControlValues = nullptr;
        }

#if DISTRHO_LV2_USE_EVENTS_IN
        fPortEventsIn = nullptr;
#endif
#if DISTRHO_PLUGIN_WANT_LATENCY
        fPortLatency = nullptr;
#endif
        // unused
        (void)fWorker;

#if ! DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
        // unused
        (void)ctrlInPortChangeReq;
#endif
    }

    ~PluginLv2()
    {
        if (fPortControls != nullptr)
        {
            delete[] fPortControls;
            fPortControls = nullptr;
        }

        if (fLastControlValues)
        {
            delete[] fLastControlValues;
            fLastControlValues = nullptr;
        }
    }

    // -------------------------------------------------------------------

    bool getPortControlValue(uint32_t index, float& value) const
    {
        if (const float* control = fPortControls[index])
        {
            switch (fPlugin.getParameterDesignation(index))
            {
            default:
                value = *control;
                break;
            case kParameterDesignationBypass:
                value = 1.0f - *control;
                break;
            }

            return true;
        }

        return false;
    }

    void setPortControlValue(uint32_t index, float value)
    {
        if (float* control = fPortControls[index])
        {
            switch (fPlugin.getParameterDesignation(index))
            {
            default:
                *control = value;
                break;
            case kParameterDesignationBypass:
                *control = 1.0f - value;
                break;
            }
        }
    }

    // -------------------------------------------------------------------

    void lv2_activate()
    {
#if DISTRHO_PLUGIN_WANT_TIMEPOS
        memset(&fTimePosition, 0, sizeof(fTimePosition));

        // hosts may not send all values, resulting on some invalid data, let's reset everything
        fTimePosition.bbt.bar   = 1;
        fTimePosition.bbt.beat  = 1;
        fTimePosition.bbt.tick  = 0.0;
        fTimePosition.bbt.barStartTick = 0;
        fTimePosition.bbt.timeSigNumerator  = 4;
        fTimePosition.bbt.timeSigDenominator     = 4;
        fTimePosition.bbt.ticksPerBeat = 1920.0;
        fTimePosition.bbt.bpm = 120.0;
#endif
        fPlugin.activate();
    }

    void lv2_deactivate()
    {
        fPlugin.deactivate();
    }

    // -------------------------------------------------------------------

    void lv2_connect_port(const uint32_t port, void* const dataLocation)
    {
        uint32_t index = 0;

#if DISTRHO_PLUGIN_NUM_INPUTS > 0
        for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_INPUTS; ++i)
        {
            if (port == index++)
            {
                fPortAudioIns[i] = (const float*)dataLocation;
                return;
            }
        }
#endif

#if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
        for (uint32_t i=0; i < DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
        {
            if (port == index++)
            {
                fPortAudioOuts[i] = (float*)dataLocation;
                return;
            }
        }
#endif

#if DISTRHO_LV2_USE_EVENTS_IN
        if (port == index++)
        {
            fPortEventsIn = (LV2_Atom_Sequence*)dataLocation;
            return;
        }
#endif

#if DISTRHO_LV2_USE_EVENTS_OUT
        if (port == index++)
        {
            fEventsOutData.port = (LV2_Atom_Sequence*)dataLocation;
            return;
        }
#endif

#if DISTRHO_PLUGIN_WANT_LATENCY
        if (port == index++)
        {
            fPortLatency = (float*)dataLocation;
            return;
        }
#endif

        for (uint32_t i=0, count=fPlugin.getParameterCount(); i < count; ++i)
        {
            if (port == index++)
            {
                fPortControls[i] = (float*)dataLocation;
                return;
            }
        }
    }

    // -------------------------------------------------------------------

    void lv2_run(const uint32_t sampleCount)
    {
        // cache midi input and time position first
#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
        uint32_t midiEventCount = 0;
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT || DISTRHO_PLUGIN_WANT_TIMEPOS
        LV2_ATOM_SEQUENCE_FOREACH(fPortEventsIn, event)
        {
            if (event == nullptr)
                break;

# if DISTRHO_PLUGIN_WANT_MIDI_INPUT
            if (event->body.type == fURIDs.midiEvent)
            {
                if (midiEventCount >= kMaxMidiEvents)
                    continue;

                const uint8_t* const data((const uint8_t*)(event + 1));

                MidiEvent& midiEvent(fMidiEvents[midiEventCount++]);

                midiEvent.frame = event->time.frames;
                midiEvent.size  = event->body.size;

                if (midiEvent.size > MidiEvent::kDataSize)
                {
                    midiEvent.dataExt = data;
                    std::memset(midiEvent.data, 0, MidiEvent::kDataSize);
                }
                else
                {
                    midiEvent.dataExt = nullptr;
                    std::memcpy(midiEvent.data, data, midiEvent.size);
                }

                continue;
            }
# endif
# if DISTRHO_PLUGIN_WANT_TIMEPOS
            if (event->body.type == fURIDs.atomBlank || event->body.type == fURIDs.atomObject)
            {
                const LV2_Atom_Object* const obj((const LV2_Atom_Object*)&event->body);

                if (obj->body.otype != fURIDs.timePosition)
                    continue;

                LV2_Atom* bar = nullptr;
                LV2_Atom* barBeat = nullptr;
                LV2_Atom* beatUnit = nullptr;
                LV2_Atom* beatsPerBar = nullptr;
                LV2_Atom* beatsPerMinute = nullptr;
                LV2_Atom* frame = nullptr;
                LV2_Atom* speed = nullptr;
                LV2_Atom* ticksPerBeat = nullptr;

                lv2_atom_object_get(obj,
                                    fURIDs.timeBar, &bar,
                                    fURIDs.timeBarBeat, &barBeat,
                                    fURIDs.timeBeatUnit, &beatUnit,
                                    fURIDs.timeBeatsPerBar, &beatsPerBar,
                                    fURIDs.timeBeatsPerMinute, &beatsPerMinute,
                                    fURIDs.timeFrame, &frame,
                                    fURIDs.timeSpeed, &speed,
                                    fURIDs.timeTicksPerBeat, &ticksPerBeat,
                                    0);

                // need to handle this first as other values depend on it
                if (ticksPerBeat != nullptr)
                {
                    /**/ if (ticksPerBeat->type == fURIDs.atomDouble)
                        fLastPositionData.ticksPerBeat = ((LV2_Atom_Double*)ticksPerBeat)->body;
                    else if (ticksPerBeat->type == fURIDs.atomFloat)
                        fLastPositionData.ticksPerBeat = ((LV2_Atom_Float*)ticksPerBeat)->body;
                    else if (ticksPerBeat->type == fURIDs.atomInt)
                        fLastPositionData.ticksPerBeat = ((LV2_Atom_Int*)ticksPerBeat)->body;
                    else if (ticksPerBeat->type == fURIDs.atomLong)
                        fLastPositionData.ticksPerBeat = ((LV2_Atom_Long*)ticksPerBeat)->body;
                    else
                        d_stderr("Unknown lv2 ticksPerBeat value type");

                    if (fLastPositionData.ticksPerBeat > 0.0)
                        fTimePosition.bbt.ticksPerBeat = fLastPositionData.ticksPerBeat;
                }

                // same
                if (speed != nullptr)
                {
                    /**/ if (speed->type == fURIDs.atomDouble)
                        fLastPositionData.speed = ((LV2_Atom_Double*)speed)->body;
                    else if (speed->type == fURIDs.atomFloat)
                        fLastPositionData.speed = ((LV2_Atom_Float*)speed)->body;
                    else if (speed->type == fURIDs.atomInt)
                        fLastPositionData.speed = ((LV2_Atom_Int*)speed)->body;
                    else if (speed->type == fURIDs.atomLong)
                        fLastPositionData.speed = ((LV2_Atom_Long*)speed)->body;
                    else
                        d_stderr("Unknown lv2 speed value type");

                    fTimePosition.isPlaying = d_isNotZero(fLastPositionData.speed);
                }

                if (bar != nullptr)
                {
                    /**/ if (bar->type == fURIDs.atomDouble)
                        fLastPositionData.bar = ((LV2_Atom_Double*)bar)->body;
                    else if (bar->type == fURIDs.atomFloat)
                        fLastPositionData.bar = ((LV2_Atom_Float*)bar)->body;
                    else if (bar->type == fURIDs.atomInt)
                        fLastPositionData.bar = ((LV2_Atom_Int*)bar)->body;
                    else if (bar->type == fURIDs.atomLong)
                        fLastPositionData.bar = ((LV2_Atom_Long*)bar)->body;
                    else
                        d_stderr("Unknown lv2 bar value type");

                    if (fLastPositionData.bar >= 0)
                        fTimePosition.bbt.bar = fLastPositionData.bar + 1;
                }

                if (barBeat != nullptr)
                {
                    /**/ if (barBeat->type == fURIDs.atomDouble)
                        fLastPositionData.barBeat = ((LV2_Atom_Double*)barBeat)->body;
                    else if (barBeat->type == fURIDs.atomFloat)
                        fLastPositionData.barBeat = ((LV2_Atom_Float*)barBeat)->body;
                    else if (barBeat->type == fURIDs.atomInt)
                        fLastPositionData.barBeat = ((LV2_Atom_Int*)barBeat)->body;
                    else if (barBeat->type == fURIDs.atomLong)
                        fLastPositionData.barBeat = ((LV2_Atom_Long*)barBeat)->body;
                    else
                        d_stderr("Unknown lv2 barBeat value type");

                    if (fLastPositionData.barBeat >= 0.0f)
                    {
                        const double rest = std::fmod(fLastPositionData.barBeat, 1.0f);
                        fTimePosition.bbt.beat = std::round(fLastPositionData.barBeat - rest + 1.0);
                        fTimePosition.bbt.tick = rest * fTimePosition.bbt.ticksPerBeat;
                    }
                }

                if (beatUnit != nullptr)
                {
                    /**/ if (beatUnit->type == fURIDs.atomDouble)
                        fLastPositionData.beatUnit = ((LV2_Atom_Double*)beatUnit)->body;
                    else if (beatUnit->type == fURIDs.atomFloat)
                        fLastPositionData.beatUnit = ((LV2_Atom_Float*)beatUnit)->body;
                    else if (beatUnit->type == fURIDs.atomInt)
                        fLastPositionData.beatUnit = ((LV2_Atom_Int*)beatUnit)->body;
                    else if (beatUnit->type == fURIDs.atomLong)
                        fLastPositionData.beatUnit = ((LV2_Atom_Long*)beatUnit)->body;
                    else
                        d_stderr("Unknown lv2 beatUnit value type");

                    if (fLastPositionData.beatUnit > 0)
                        fTimePosition.bbt.timeSigDenominator = fLastPositionData.beatUnit;
                }

                if (beatsPerBar != nullptr)
                {
                    /**/ if (beatsPerBar->type == fURIDs.atomDouble)
                        fLastPositionData.beatsPerBar = ((LV2_Atom_Double*)beatsPerBar)->body;
                    else if (beatsPerBar->type == fURIDs.atomFloat)
                        fLastPositionData.beatsPerBar = ((LV2_Atom_Float*)beatsPerBar)->body;
                    else if (beatsPerBar->type == fURIDs.atomInt)
                        fLastPositionData.beatsPerBar = ((LV2_Atom_Int*)beatsPerBar)->body;
                    else if (beatsPerBar->type == fURIDs.atomLong)
                        fLastPositionData.beatsPerBar = ((LV2_Atom_Long*)beatsPerBar)->body;
                    else
                        d_stderr("Unknown lv2 beatsPerBar value type");

                    if (fLastPositionData.beatsPerBar > 0.0f)
                        fTimePosition.bbt.timeSigNumerator = fLastPositionData.beatsPerBar;
                }

                if (beatsPerMinute != nullptr)
                {
                    /**/ if (beatsPerMinute->type == fURIDs.atomDouble)
                        fLastPositionData.beatsPerMinute = ((LV2_Atom_Double*)beatsPerMinute)->body;
                    else if (beatsPerMinute->type == fURIDs.atomFloat)
                        fLastPositionData.beatsPerMinute = ((LV2_Atom_Float*)beatsPerMinute)->body;
                    else if (beatsPerMinute->type == fURIDs.atomInt)
                        fLastPositionData.beatsPerMinute = ((LV2_Atom_Int*)beatsPerMinute)->body;
                    else if (beatsPerMinute->type == fURIDs.atomLong)
                        fLastPositionData.beatsPerMinute = ((LV2_Atom_Long*)beatsPerMinute)->body;
                    else
                        d_stderr("Unknown lv2 beatsPerMinute value type");

                    if (fLastPositionData.beatsPerMinute > 0.0f)
                    {
                        fTimePosition.bbt.bpm = fLastPositionData.beatsPerMinute;

                        if (d_isNotZero(fLastPositionData.speed))
                            fTimePosition.bbt.bpm *= std::abs(fLastPositionData.speed);
                    }
                }

                if (frame != nullptr)
                {
                    /**/ if (frame->type == fURIDs.atomDouble)
                        fLastPositionData.frame = ((LV2_Atom_Double*)frame)->body;
                    else if (frame->type == fURIDs.atomFloat)
                        fLastPositionData.frame = ((LV2_Atom_Float*)frame)->body;
                    else if (frame->type == fURIDs.atomInt)
                        fLastPositionData.frame = ((LV2_Atom_Int*)frame)->body;
                    else if (frame->type == fURIDs.atomLong)
                        fLastPositionData.frame = ((LV2_Atom_Long*)frame)->body;
                    else
                        d_stderr("Unknown lv2 frame value type");

                    if (fLastPositionData.frame >= 0)
                        fTimePosition.frame = fLastPositionData.frame;
                }

                fTimePosition.bbt.barStartTick = fTimePosition.bbt.ticksPerBeat*
                                                 fTimePosition.bbt.timeSigNumerator*
                                                 (fTimePosition.bbt.bar-1);

                fTimePosition.bbtSupported = (fLastPositionData.beatsPerMinute > 0.0 &&
                                              fLastPositionData.beatUnit > 0 &&
                                              fLastPositionData.beatsPerBar > 0.0f);

                fPlugin.setTimePosition(fTimePosition);

                continue;
            }
# endif
        }
#endif
        // Check for updated parameters
        float curValue;

        for (uint32_t i=0, count=fPlugin.getParameterCount(); i < count; ++i)
        {
            if (!getPortControlValue(i, curValue))
                continue;

            if (fPlugin.isParameterInput(i) && d_isNotEqual(fLastControlValues[i], curValue))
            {
                fLastControlValues[i] = curValue;

                fPlugin.setParameterValue(i, curValue);
            }
        }

        // Run plugin
        if (sampleCount != 0)
        {
           #ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
            fRunCount = mod_license_run_begin(fRunCount, sampleCount);
           #endif

           #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
            fPlugin.run(fPortAudioIns, fPortAudioOuts, sampleCount, fMidiEvents, midiEventCount);
           #else
            fPlugin.run(fPortAudioIns, fPortAudioOuts, sampleCount);
           #endif

           #ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
            for (uint32_t i=0; i<DISTRHO_PLUGIN_NUM_OUTPUTS; ++i)
                mod_license_run_silence(fRunCount, fPortAudioOuts[i], sampleCount, i);
           #endif

           #if DISTRHO_PLUGIN_WANT_TIMEPOS
            // update timePos for next callback
            if (d_isNotZero(fLastPositionData.speed))
            {
                if (fLastPositionData.speed > 0.0)
                {
                    // playing forwards
                    fLastPositionData.frame += sampleCount;
                }
                else
                {
                    // playing backwards
                    fLastPositionData.frame -= sampleCount;

                    if (fLastPositionData.frame < 0)
                        fLastPositionData.frame = 0;
                }

                fTimePosition.frame = fLastPositionData.frame;

                if (fTimePosition.bbtSupported)
                {
                    const double beatsPerMinute = fLastPositionData.beatsPerMinute * fLastPositionData.speed;
                    const double framesPerBeat  = 60.0 * fSampleRate / beatsPerMinute;
                    const double addedBarBeats  = double(sampleCount) / framesPerBeat;

                    if (fLastPositionData.barBeat >= 0.0f)
                    {
                        fLastPositionData.barBeat = std::fmod(fLastPositionData.barBeat+addedBarBeats,
                                                              (double)fLastPositionData.beatsPerBar);

                        const double rest = std::fmod(fLastPositionData.barBeat, 1.0f);
                        fTimePosition.bbt.beat = std::round(fLastPositionData.barBeat - rest + 1.0);
                        fTimePosition.bbt.tick = rest * fTimePosition.bbt.ticksPerBeat;

                        if (fLastPositionData.bar >= 0)
                        {
                            fLastPositionData.bar += std::floor((fLastPositionData.barBeat+addedBarBeats)/
                                                             fLastPositionData.beatsPerBar);

                            if (fLastPositionData.bar < 0)
                                fLastPositionData.bar = 0;

                            fTimePosition.bbt.bar = fLastPositionData.bar + 1;

                            fTimePosition.bbt.barStartTick = fTimePosition.bbt.ticksPerBeat*
                                                             fTimePosition.bbt.timeSigNumerator*
                                                            (fTimePosition.bbt.bar-1);
                        }
                    }

                    fTimePosition.bbt.bpm = std::abs(beatsPerMinute);
                }

                fPlugin.setTimePosition(fTimePosition);
            }
           #endif
        }

        updateParameterOutputsAndTriggers();

       #if DISTRHO_LV2_USE_EVENTS_OUT
        fEventsOutData.endRun();
       #endif
    }

    // -------------------------------------------------------------------

    uint32_t lv2_get_options(LV2_Options_Option* const /*options*/)
    {
        // currently unused
        return LV2_OPTIONS_ERR_UNKNOWN;
    }

    uint32_t lv2_set_options(const LV2_Options_Option* const options)
    {
        for (int i=0; options[i].key != 0; ++i)
        {
            if (options[i].key == fUridMap->map(fUridMap->handle, LV2_BUF_SIZE__nominalBlockLength))
            {
                if (options[i].type == fURIDs.atomInt)
                {
                    const int32_t bufferSize(*(const int32_t*)options[i].value);
                    fPlugin.setBufferSize(bufferSize, true);
                }
                else
                {
                    d_stderr("Host changed nominalBlockLength but with wrong value type");
                }
            }
            else if (options[i].key == fUridMap->map(fUridMap->handle, LV2_BUF_SIZE__maxBlockLength) && ! fUsingNominal)
            {
                if (options[i].type == fURIDs.atomInt)
                {
                    const int32_t bufferSize(*(const int32_t*)options[i].value);
                    fPlugin.setBufferSize(bufferSize, true);
                }
                else
                {
                    d_stderr("Host changed maxBlockLength but with wrong value type");
                }
            }
            else if (options[i].key == fUridMap->map(fUridMap->handle, LV2_PARAMETERS__sampleRate))
            {
                if (options[i].type == fURIDs.atomFloat)
                {
                    const float sampleRate(*(const float*)options[i].value);
                    fSampleRate = sampleRate;
                    fPlugin.setSampleRate(sampleRate, true);
                }
                else
                {
                    d_stderr("Host changed sampleRate but with wrong value type");
                }
            }
        }

        return LV2_OPTIONS_SUCCESS;
    }

    // -------------------------------------------------------------------

   #if DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
    void* lv2_get_instance_pointer()
    {
        return fPlugin.getInstancePointer();
    }
   #endif

    // -------------------------------------------------------------------

private:
    PluginExporter fPlugin;
    const bool fUsingNominal; // if false use maxBlockLength

   #ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
    uint32_t fRunCount;
   #endif

    // LV2 ports
   #if DISTRHO_PLUGIN_NUM_INPUTS > 0
    const float*  fPortAudioIns[DISTRHO_PLUGIN_NUM_INPUTS];
   #else
    const float** fPortAudioIns;
   #endif
   #if DISTRHO_PLUGIN_NUM_OUTPUTS > 0
    float*  fPortAudioOuts[DISTRHO_PLUGIN_NUM_OUTPUTS];
   #else
    float** fPortAudioOuts;
   #endif
    float** fPortControls;
   #if DISTRHO_LV2_USE_EVENTS_IN
    LV2_Atom_Sequence* fPortEventsIn;
   #endif
   #if DISTRHO_PLUGIN_WANT_LATENCY
    float* fPortLatency;
   #endif

    // Temporary data
    float* fLastControlValues;
    double fSampleRate;
   #if DISTRHO_PLUGIN_WANT_MIDI_INPUT
    MidiEvent fMidiEvents[kMaxMidiEvents];
   #endif
   #if DISTRHO_PLUGIN_WANT_TIMEPOS
    TimePosition fTimePosition;

    struct Lv2PositionData {
        int64_t  bar;
        float    barBeat;
        uint32_t beatUnit;
        float    beatsPerBar;
        float    beatsPerMinute;
        int64_t  frame;
        double   speed;
        double   ticksPerBeat;

        Lv2PositionData()
            : bar(-1),
              barBeat(-1.0f),
              beatUnit(0),
              beatsPerBar(0.0f),
              beatsPerMinute(0.0f),
              frame(-1),
              speed(0.0),
              ticksPerBeat(-1.0) {}

    } fLastPositionData;
   #endif

   #if DISTRHO_LV2_USE_EVENTS_OUT
    struct Lv2EventsOutData {
        uint32_t capacity, offset;
        LV2_Atom_Sequence* port;

        Lv2EventsOutData()
            : capacity(0),
              offset(0),
              port(nullptr) {}

        void initIfNeeded(const LV2_URID uridAtomSequence)
        {
            if (capacity != 0)
                return;

            capacity = port->atom.size;

            port->atom.size = sizeof(LV2_Atom_Sequence_Body);
            port->atom.type = uridAtomSequence;
            port->body.unit = 0;
            port->body.pad  = 0;
        }

        void growBy(const uint32_t size)
        {
            offset += size;
            port->atom.size += size;
        }

        void endRun()
        {
            capacity = 0;
            offset = 0;
        }

    } fEventsOutData;
   #endif

    // LV2 URIDs
    struct URIDs {
        const LV2_URID_Map* _uridMap;
        LV2_URID atomBlank;
        LV2_URID atomObject;
        LV2_URID atomDouble;
        LV2_URID atomFloat;
        LV2_URID atomInt;
        LV2_URID atomLong;
        LV2_URID atomPath;
        LV2_URID atomSequence;
        LV2_URID atomString;
        LV2_URID atomURID;
        LV2_URID dpfKeyValue;
        LV2_URID midiEvent;
        LV2_URID patchSet;
        LV2_URID patchProperty;
        LV2_URID patchValue;
        LV2_URID timePosition;
        LV2_URID timeBar;
        LV2_URID timeBarBeat;
        LV2_URID timeBeatUnit;
        LV2_URID timeBeatsPerBar;
        LV2_URID timeBeatsPerMinute;
        LV2_URID timeTicksPerBeat;
        LV2_URID timeFrame;
        LV2_URID timeSpeed;

        URIDs(const LV2_URID_Map* const uridMap)
            : _uridMap(uridMap),
              atomBlank(map(LV2_ATOM__Blank)),
              atomObject(map(LV2_ATOM__Object)),
              atomDouble(map(LV2_ATOM__Double)),
              atomFloat(map(LV2_ATOM__Float)),
              atomInt(map(LV2_ATOM__Int)),
              atomLong(map(LV2_ATOM__Long)),
              atomPath(map(LV2_ATOM__Path)),
              atomSequence(map(LV2_ATOM__Sequence)),
              atomString(map(LV2_ATOM__String)),
              atomURID(map(LV2_ATOM__URID)),
              dpfKeyValue(map(DISTRHO_PLUGIN_LV2_STATE_PREFIX "KeyValueState")),
              midiEvent(map(LV2_MIDI__MidiEvent)),
              patchSet(map(LV2_PATCH__Set)),
              patchProperty(map(LV2_PATCH__property)),
              patchValue(map(LV2_PATCH__value)),
              timePosition(map(LV2_TIME__Position)),
              timeBar(map(LV2_TIME__bar)),
              timeBarBeat(map(LV2_TIME__barBeat)),
              timeBeatUnit(map(LV2_TIME__beatUnit)),
              timeBeatsPerBar(map(LV2_TIME__beatsPerBar)),
              timeBeatsPerMinute(map(LV2_TIME__beatsPerMinute)),
              timeTicksPerBeat(map(LV2_KXSTUDIO_PROPERTIES__TimePositionTicksPerBeat)),
              timeFrame(map(LV2_TIME__frame)),
              timeSpeed(map(LV2_TIME__speed)) {}

        inline LV2_URID map(const char* const uri) const
        {
            return _uridMap->map(_uridMap->handle, uri);
        }
    } fURIDs;

    // LV2 features
   #if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
    const LV2_ControlInputPort_Change_Request* const fCtrlInPortChangeReq;
   #endif
    const LV2_URID_Map* const fUridMap;
    const LV2_Worker_Schedule* const fWorker;

    void updateParameterOutputsAndTriggers()
    {
        float curValue;

        for (uint32_t i=0, count=fPlugin.getParameterCount(); i < count; ++i)
        {
            if (fPlugin.isParameterOutput(i))
            {
                curValue = fLastControlValues[i] = fPlugin.getParameterValue(i);

                setPortControlValue(i, curValue);
            }
            else if ((fPlugin.getParameterHints(i) & kParameterIsTrigger) == kParameterIsTrigger)
            {
                // NOTE: host is responsible for auto-updating control port buffers
            }
        }

       #if DISTRHO_PLUGIN_WANT_LATENCY
        if (fPortLatency != nullptr)
            *fPortLatency = fPlugin.getLatency();
       #endif
    }

   #if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
    bool requestParameterValueChange(const uint32_t index, const float value)
    {
        if (fCtrlInPortChangeReq == nullptr)
            return false;
        return fCtrlInPortChangeReq->request_change(fCtrlInPortChangeReq->handle, index, value);
    }

    static bool requestParameterValueChangeCallback(void* const ptr, const uint32_t index, const float value)
    {
        return (((PluginLv2*)ptr)->requestParameterValueChange(index, value) == 0);
    }
   #endif

   #if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
    bool writeMidi(const MidiEvent& midiEvent)
    {
        DISTRHO_SAFE_ASSERT_RETURN(fEventsOutData.port != nullptr, false);

        fEventsOutData.initIfNeeded(fURIDs.atomSequence);

        const uint32_t capacity = fEventsOutData.capacity;
        const uint32_t offset = fEventsOutData.offset;

        if (sizeof(LV2_Atom_Event) + midiEvent.size > capacity - offset)
            return false;

        LV2_Atom_Event* const aev = (LV2_Atom_Event*)(LV2_ATOM_CONTENTS(LV2_Atom_Sequence, fEventsOutData.port) + offset);
        aev->time.frames = midiEvent.frame;
        aev->body.type   = fURIDs.midiEvent;
        aev->body.size   = midiEvent.size;
        std::memcpy(LV2_ATOM_BODY(&aev->body),
                    midiEvent.size > MidiEvent::kDataSize ? midiEvent.dataExt : midiEvent.data,
                    midiEvent.size);

        fEventsOutData.growBy(lv2_atom_pad_size(sizeof(LV2_Atom_Event) + midiEvent.size));

        return true;
    }

    static bool writeMidiCallback(void* ptr, const MidiEvent& midiEvent)
    {
        return ((PluginLv2*)ptr)->writeMidi(midiEvent);
    }
   #endif
};

// -----------------------------------------------------------------------

static LV2_Handle lv2_instantiate(const LV2_Descriptor*, double sampleRate, const char* bundlePath, const LV2_Feature* const* features)
{
    const LV2_Options_Option* options = nullptr;
    const LV2_URID_Map*       uridMap = nullptr;
    const LV2_Worker_Schedule* worker = nullptr;
    const LV2_ControlInputPort_Change_Request* ctrlInPortChangeReq = nullptr;

    for (int i=0; features[i] != nullptr; ++i)
    {
        if (std::strcmp(features[i]->URI, LV2_OPTIONS__options) == 0)
            options = (const LV2_Options_Option*)features[i]->data;
        else if (std::strcmp(features[i]->URI, LV2_URID__map) == 0)
            uridMap = (const LV2_URID_Map*)features[i]->data;
        else if (std::strcmp(features[i]->URI, LV2_WORKER__schedule) == 0)
            worker = (const LV2_Worker_Schedule*)features[i]->data;
        else if (std::strcmp(features[i]->URI, LV2_CONTROL_INPUT_PORT_CHANGE_REQUEST_URI) == 0)
            ctrlInPortChangeReq = (const LV2_ControlInputPort_Change_Request*)features[i]->data;
    }

    if (options == nullptr)
    {
        d_stderr("Options feature missing, cannot continue!");
        return nullptr;
    }

    if (uridMap == nullptr)
    {
        d_stderr("URID Map feature missing, cannot continue!");
        return nullptr;
    }

#ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
    mod_license_check(features, DISTRHO_PLUGIN_URI);
#endif

    d_nextBufferSize = 0;
    bool usingNominal = false;

    for (int i=0; options[i].key != 0; ++i)
    {
        if (options[i].key == uridMap->map(uridMap->handle, LV2_BUF_SIZE__nominalBlockLength))
        {
            if (options[i].type == uridMap->map(uridMap->handle, LV2_ATOM__Int))
            {
                d_nextBufferSize = *(const int*)options[i].value;
                usingNominal = true;
            }
            else
            {
                d_stderr("Host provides nominalBlockLength but has wrong value type");
            }
            break;
        }

        if (options[i].key == uridMap->map(uridMap->handle, LV2_BUF_SIZE__maxBlockLength))
        {
            if (options[i].type == uridMap->map(uridMap->handle, LV2_ATOM__Int))
                d_nextBufferSize = *(const int*)options[i].value;
            else
                d_stderr("Host provides maxBlockLength but has wrong value type");

            // no break, continue in case host supports nominalBlockLength
        }
    }

    if (d_nextBufferSize == 0)
    {
        d_stderr("Host does not provide nominalBlockLength or maxBlockLength options");
        d_nextBufferSize = 2048;
    }

    d_nextSampleRate = sampleRate;
    d_nextBundlePath = bundlePath;
    d_nextCanRequestParameterValueChanges = ctrlInPortChangeReq != nullptr;

    return new PluginLv2(sampleRate, uridMap, worker, ctrlInPortChangeReq, usingNominal);
}

#define instancePtr ((PluginLv2*)instance)

static void lv2_connect_port(LV2_Handle instance, uint32_t port, void* dataLocation)
{
    instancePtr->lv2_connect_port(port, dataLocation);
}

static void lv2_activate(LV2_Handle instance)
{
    instancePtr->lv2_activate();
}

static void lv2_run(LV2_Handle instance, uint32_t sampleCount)
{
    instancePtr->lv2_run(sampleCount);
}

static void lv2_deactivate(LV2_Handle instance)
{
    instancePtr->lv2_deactivate();
}

static void lv2_cleanup(LV2_Handle instance)
{
    delete instancePtr;
}

// -----------------------------------------------------------------------

static uint32_t lv2_get_options(LV2_Handle instance, LV2_Options_Option* options)
{
    return instancePtr->lv2_get_options(options);
}

static uint32_t lv2_set_options(LV2_Handle instance, const LV2_Options_Option* options)
{
    return instancePtr->lv2_set_options(options);
}

// -----------------------------------------------------------------------

#if DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
static void* lv2_get_instance_pointer(LV2_Handle instance)
{
    return instancePtr->lv2_get_instance_pointer();
}
#endif

// -----------------------------------------------------------------------

static const void* lv2_extension_data(const char* uri)
{
    static const LV2_Options_Interface options = { lv2_get_options, lv2_set_options };

    if (std::strcmp(uri, LV2_OPTIONS__interface) == 0)
        return &options;

#if DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
    struct LV2_DirectAccess_Interface {
        void* (*get_instance_pointer)(LV2_Handle handle);
    };

    static const LV2_DirectAccess_Interface directaccess = { lv2_get_instance_pointer };

    if (std::strcmp(uri, DISTRHO_PLUGIN_LV2_STATE_PREFIX "direct-access") == 0)
        return &directaccess;
#endif

#ifdef DISTRHO_PLUGIN_LICENSED_FOR_MOD
    return mod_license_interface(uri);
#else
    return nullptr;
#endif
}

#undef instancePtr

// -----------------------------------------------------------------------

static const LV2_Descriptor sLv2Descriptor = {
    DISTRHO_PLUGIN_URI,
    lv2_instantiate,
    lv2_connect_port,
    lv2_activate,
    lv2_run,
    lv2_deactivate,
    lv2_cleanup,
    lv2_extension_data
};

// -----------------------------------------------------------------------

END_NAMESPACE_DISTRHO

DISTRHO_PLUGIN_EXPORT
const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
    USE_NAMESPACE_DISTRHO
    return (index == 0) ? &sLv2Descriptor : nullptr;
}

// -----------------------------------------------------------------------
