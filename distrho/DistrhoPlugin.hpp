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

#ifndef DISTRHO_PLUGIN_HPP_INCLUDED
#define DISTRHO_PLUGIN_HPP_INCLUDED

#include "DistrhoDetails.hpp"
#include "extra/LeakDetector.hpp"
#include "src/DistrhoPluginChecks.h"

START_NAMESPACE_DISTRHO

/* ------------------------------------------------------------------------------------------------------------
 * DPF Plugin */

/**
   @defgroup MainClasses Main Classes
   @{
 */

/**
   DPF Plugin class from where plugin instances are created.

   The public methods (Host state) are called from the plugin to get or set host information.@n
   They can be called from a plugin instance at anytime unless stated otherwise.@n
   All other methods are to be implemented by the plugin and will be called by the host.

   Shortly after a plugin instance is created, the various init* functions will be called by the host.@n
   Host will call activate() before run(), and deactivate() before the plugin instance is destroyed.@n
   The host may call deactivate right after activate and vice-versa, but never activate/deactivate consecutively.@n
   There is no limit on how many times run() is called, only that activate/deactivate will be called in between.

   The buffer size and sample rate values will remain constant between activate and deactivate.@n
   Buffer size is only a hint though, the host might call run() with a higher or lower number of frames.

   Some of this class functions are only available according to some macros.

   DISTRHO_PLUGIN_WANT_PROGRAMS activates program related features.@n
   When enabled you need to implement initProgramName() and loadProgram().

   DISTRHO_PLUGIN_WANT_STATE activates internal state features.@n
   When enabled you need to implement initState() and setState().

   The process function run() changes wherever DISTRHO_PLUGIN_WANT_MIDI_INPUT is enabled or not.@n
   When enabled it provides midi input events.
 */

/**
    Plugin class constructor.@n
    You must set all parameter values to their defaults, matching ParameterRanges::def.
*/
extern void PluginPrivateData_init(struct PluginPrivateData* pData, uint32_t parameterCount, uint32_t programCount, uint32_t stateCount);

/* --------------------------------------------------------------------------------------------------------
* Host state */

/**
    Get the current buffer size that will probably be used during processing, in frames.@n
    This value will remain constant between activate and deactivate.
    @note This value is only a hint!@n
        Hosts might call run() with a higher or lower number of frames.
    @see bufferSizeChanged(uint32_t)
*/
extern uint32_t plugin_getBufferSize(void*);

/**
    Get the current sample rate that will be used during processing.@n
    This value will remain constant between activate and deactivate.
    @see sampleRateChanged(double)
*/
extern double plugin_getSampleRate(void*);

/**
    Get the bundle path where the plugin resides.
    Can return null if the plugin is not available in a bundle (if it is a single binary).
    @see getBinaryFilename
    @see getResourcePath
*/
extern const char* plugin_getBundlePath(void*);

/**
    Check if this plugin instance is a "dummy" one used for plugin meta-data/information export.@n
    When true no processing will be done, the plugin is created only to extract information.@n
    In DPF, LADSPA/DSSI, VST2 and VST3 formats create one global instance per plugin binary
    while LV2 creates one when generating turtle meta-data.
*/
extern bool plugin_isDummyInstance(void*);

/**
    Check if this plugin instance is a "selftest" one used for automated plugin tests.@n
    To enable this mode build with `DPF_RUNTIME_TESTING` macro defined (i.e. set as compiler build flag),
    and run the JACK/Standalone executable with "selftest" as its only and single argument.

    A few basic DSP and UI tests will run in self-test mode, with once instance having this function returning true.@n
    You can use this chance to do a few tests of your own as well.
*/
extern bool plugin_isSelfTestInstance(void*);

#if DISTRHO_PLUGIN_WANT_TIMEPOS
/**
    Get the current host transport time position.@n
    This function should only be called during run().@n
    You can call this during other times, but the returned position is not guaranteed to be in sync.
    @note TimePosition is not supported in LADSPA and DSSI plugin formats.
*/
extern const TimePosition& plugin_getTimePosition(void*);
#endif

#if DISTRHO_PLUGIN_WANT_LATENCY
/**
    Change the plugin audio output latency to @a frames.@n
    This function should only be called in the constructor, activate() and run().
    @note This function is only available if DISTRHO_PLUGIN_WANT_LATENCY is enabled.
*/
extern void plugin_setLatency(void*, uint32_t frames);
#endif

#if DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
/**
    Write a MIDI output event.@n
    This function must only be called during run().@n
    Returns false when the host buffer is full, in which case do not call this again until the next run().
*/
extern bool plugin_writeMidiEvent(void*, const MidiEvent& midiEvent);
#endif

#if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
/**
    Check if parameter value change requests will work with the current plugin host.
    @note This function is only available if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST is enabled.
    @see requestParameterValueChange(uint32_t, float)
*/
extern bool plugin_canRequestParameterValueChanges(void*);

/**
    Request a parameter value change from the host.
    If successful, this function will automatically trigger a parameter update on the UI side as well.
    This function can fail, for example if the host is busy with the parameter for read-only automation.
    Some hosts simply do not have this functionality, which can be verified with canRequestParameterValueChanges().
    @note This function is only available if DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST is enabled.
*/
extern bool plugin_requestParameterValueChange(void*, uint32_t index, float value);
#endif

#if DISTRHO_PLUGIN_WANT_STATE
/**
    Set state value and notify the host about the change.@n
    This function will call `setState()` and also trigger an update on the UI side as necessary.@n
    It must not be called during run.@n
    The state must be host readable.
    @note this function does nothing on DSSI plugin format, as DSSI only supports UI->DSP messages.

    TODO API under construction
*/
extern bool plugin_updateStateValue(void*, const char* key, const char* value);
#endif

/* --------------------------------------------------------------------------------------------------------
* Information */

/**
    Get the plugin name.@n
    Returns DISTRHO_PLUGIN_NAME by default.
*/
extern const char* plugin_getName(void*);

/**
    Get the plugin label.@n
    This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
*/
extern const char* plugin_getLabel(void*);

/**
    Get an extensive comment/description about the plugin.@n
    Optional, returns nothing by default.
*/
extern const char* plugin_getDescription(void*);

/**
    Get the plugin author/maker.
*/
extern const char* plugin_getMaker(void*);

/**
    Get the plugin homepage.@n
    Optional, returns nothing by default.
*/
extern const char* plugin_getHomePage(void*);

/**
    Get the plugin license (a single line of text or a URL).@n
    For commercial plugins this should return some short copyright information.
*/
extern const char* plugin_getLicense(void*);

/**
    Get the plugin version, in hexadecimal.
    @see d_version()
*/
extern uint32_t plugin_getVersion(void*);

/**
    Get the plugin unique Id.@n
    This value is used by LADSPA, DSSI and VST plugin formats.
    @see d_cconst()
*/
extern int64_t plugin_getUniqueId(void*);

/* --------------------------------------------------------------------------------------------------------
* Init */

/**
    Initialize the audio port @a index.@n
    This function will be called once, shortly after the plugin is created.
*/
extern void plugin_initAudioPort(void*, bool input, uint32_t index, AudioPort& port);

/**
    Initialize the parameter @a index.@n
    This function will be called once, shortly after the plugin is created.
*/
extern void plugin_initParameter(void*, uint32_t index, Parameter& parameter);

/**
    Initialize the port group @a groupId.@n
    This function will be called once,
    shortly after the plugin is created and all audio ports and parameters have been enumerated.
*/
extern void plugin_initPortGroup(void*, uint32_t groupId, PortGroup& portGroup);

#if DISTRHO_PLUGIN_WANT_PROGRAMS
/**
    Set the name of the program @a index.@n
    This function will be called once, shortly after the plugin is created.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_PROGRAMS is enabled.
*/
extern void plugin_initProgramName(void*, uint32_t index, String& programName);
#endif

#if DISTRHO_PLUGIN_WANT_STATE
/**
    Initialize the state @a index.@n
    This function will be called once, shortly after the plugin is created.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_STATE is enabled.
*/
extern void plugin_initState(void*, uint32_t index, State& state);

DISTRHO_DEPRECATED_BY("initState(uint32_t,State&)")
void plugin_initState(uint32_t, String&, String&) {}

DISTRHO_DEPRECATED_BY("initState(uint32_t,State&)")
bool plugin_isStateFile(uint32_t) { return false; }
#endif

/* --------------------------------------------------------------------------------------------------------
* Internal data */

/**
    Get the current value of a parameter.@n
    The host may call this function from any context, including realtime processing.
*/
extern float plugin_getParameterValue(void*, uint32_t index);

/**
    Change a parameter value.@n
    The host may call this function from any context, including realtime processing.@n
    When a parameter is marked as automatable, you must ensure no non-realtime operations are performed.
    @note This function will only be called for parameter inputs.
*/
extern void plugin_setParameterValue(void*, uint32_t index, float value);

#if DISTRHO_PLUGIN_WANT_PROGRAMS
/**
    Load a program.@n
    The host may call this function from any context, including realtime processing.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_PROGRAMS is enabled.
*/
extern void plugin_loadProgram(void*, uint32_t index);
#endif

#if DISTRHO_PLUGIN_WANT_FULL_STATE
/**
    Get the value of an internal state.@n
    The host may call this function from any non-realtime context.@n
    Must be implemented by your plugin class if DISTRHO_PLUGIN_WANT_FULL_STATE is enabled.
    @note The use of this function breaks compatibility with the DSSI format.
*/
extern String plugin_getState(void*, const char* key);
#endif

#if DISTRHO_PLUGIN_WANT_STATE
/**
    Change an internal state @a key to @a value.@n
    Must be implemented by your plugin class only if DISTRHO_PLUGIN_WANT_STATE is enabled.
*/
extern void plugin_setState(void*, const char* key, const char* value);
#endif

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

/**
    Activate this plugin.
*/
extern void plugin_activate(void*);

/**
    Deactivate this plugin.
*/
extern void plugin_deactivate(void*);

#if DISTRHO_PLUGIN_WANT_MIDI_INPUT
/**
    Run/process function for plugins with MIDI input.
    @note Some parameters might be null if there are no audio inputs/outputs or MIDI events.
*/
extern void plugin_run(void*, const float** inputs, float** outputs, uint32_t frames,
                    const MidiEvent* midiEvents, uint32_t midiEventCount);
#else
/**
    Run/process function for plugins without MIDI input.
    @note Some parameters might be null if there are no audio inputs or outputs.
*/
extern void plugin_run(void*, const float** inputs, float** outputs, uint32_t frames);
#endif

/* --------------------------------------------------------------------------------------------------------
* Callbacks (optional) */

/**
    Optional callback to inform the plugin about a buffer size change.@n
    This function will only be called when the plugin is deactivated.
    @note This value is only a hint!@n
        Hosts might call run() with a higher or lower number of frames.
    @see getBufferSize()
*/
extern void plugin_bufferSizeChanged(void*, uint32_t newBufferSize);

/**
    Optional callback to inform the plugin about a sample rate change.@n
    This function will only be called when the plugin is deactivated.
    @see getSampleRate()
*/
extern void plugin_sampleRateChanged(void*, double newSampleRate);

/** @} */

/* ------------------------------------------------------------------------------------------------------------
 * Create plugin, entry point */

/**
   @defgroup EntryPoints Entry Points
   @{
 */

/**
   Create an instance of the Plugin class.@n
   This is the entry point for DPF plugins.@n
   DPF will call this to either create an instance of your plugin for the host
   or to fetch some initial information for internal caching.
 */
extern void* createPlugin();
extern void destroyPlugin(void*);
extern struct PluginPrivateData* getPluginPrivateData(void*);

/** @} */

// -----------------------------------------------------------------------------------------------------------

END_NAMESPACE_DISTRHO

#endif // DISTRHO_PLUGIN_HPP_INCLUDED
