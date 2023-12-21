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

   The process function run() changes wherever DISTRHO_PLUGIN_WANT_MIDI_INPUT is enabled or not.@n
   When enabled it provides midi input events.
 */

/* --------------------------------------------------------------------------------------------------------
* Host state */

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

/* --------------------------------------------------------------------------------------------------------
* Information */

/**
    Get the plugin name.@n
    Returns DISTRHO_PLUGIN_NAME by default.
*/
extern const char* plugin_getName();

/**
    Get the plugin label.@n
    This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
    A plugin label follows the same rules as Parameter::symbol, with the exception that it can start with numbers.
*/
extern const char* plugin_getLabel();

/**
    Get an extensive comment/description about the plugin.@n
    Optional, returns nothing by default.
*/
extern const char* plugin_getDescription();

/**
    Get the plugin author/maker.
*/
extern const char* plugin_getMaker();

/**
    Get the plugin homepage.@n
    Optional, returns nothing by default.
*/
extern const char* plugin_getHomePage();

/**
    Get the plugin license (a single line of text or a URL).@n
    For commercial plugins this should return some short copyright information.
*/
extern const char* plugin_getLicense();

/**
    Get the plugin version, in hexadecimal.
    @see d_version()
*/
extern uint32_t plugin_getVersion();

/**
    Get the plugin unique Id.@n
    This value is used by LADSPA, DSSI and VST plugin formats.
    @see d_cconst()
*/
extern int64_t plugin_getUniqueId();

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

void plugin_default_initAudioPort(bool input, uint32_t index, AudioPort& port);
void plugin_default_initPortGroup(uint32_t groupId, PortGroup& portGroup);

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


#endif // DISTRHO_PLUGIN_HPP_INCLUDED
