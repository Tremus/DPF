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

#ifndef DISTRHO_DETAILS_HPP_INCLUDED
#define DISTRHO_DETAILS_HPP_INCLUDED

#include "extra/String.hpp"

/* --------------------------------------------------------------------------------------------------------------------
 * Audio Port Hints */

/**
   @defgroup AudioPortHints Audio Port Hints

   Various audio port hints.
   @see AudioPort::hints
   @{
 */

// Audio port can be used as control voltage (LV2 and JACK standalone only).
static constexpr const uint32_t kAudioPortIsCV = 0x1;

/**
   Audio port should be used as sidechan (LV2 and VST3 only).
   This hint should not be used with CV style ports.
   @note non-sidechain audio ports must exist in the plugin if this flag is set.
 */
static constexpr const uint32_t kAudioPortIsSidechain = 0x2;

/**
   CV port has bipolar range (-1 to +1, or -5 to +5 if scaled).
   This is merely a hint to tell the host what value range to expect.
 */
static constexpr const uint32_t kCVPortHasBipolarRange = 0x10;

/**
   CV port has negative unipolar range (-1 to 0, or -10 to 0 if scaled).
   This is merely a hint to tell the host what value range to expect.
 */
static constexpr const uint32_t kCVPortHasNegativeUnipolarRange = 0x20;

/**
   CV port has positive unipolar range (0 to +1, or 0 to +10 if scaled).
   This is merely a hint to tell the host what value range to expect.
 */
static constexpr const uint32_t kCVPortHasPositiveUnipolarRange = 0x40;

/**
   CV port has scaled range to match real values (-5 to +5v bipolar, +/-10 to 0v unipolar).
   One other range flag is required if this flag is set.

   When enabled, this makes the port a mod:CVPort, compatible with the MOD Devices platform.
 */
static constexpr const uint32_t kCVPortHasScaledRange = 0x80;

/**
   CV port is optional, allowing hosts that do no CV ports to load the plugin.
   When loaded in hosts that don't support CV, the float* buffer for this port will be null.
 */
static constexpr const uint32_t kCVPortIsOptional = 0x100;

/** @} */

/* --------------------------------------------------------------------------------------------------------------------
 * Parameter Hints */

/**
   @defgroup ParameterHints Parameter Hints

   Various parameter hints.
   @see Parameter::hints
   @{
 */

/**
   Parameter is automatable (real-time safe).
   @see Plugin::setParameterValue(uint32_t, float)
 */
static constexpr const uint32_t kParameterIsAutomatable = 0x01;
static constexpr const uint32_t kParameterIsBoolean = 0x02;
static constexpr const uint32_t kParameterIsInteger = 0x04;
static constexpr const uint32_t kParameterIsLogarithmic = 0x08;

/**
   Parameter is of output type.@n
   When unset, parameter is assumed to be of input type.

   Parameter inputs are changed by the host and typically should not be changed by the plugin.@n
   One exception is when changing programs, see Plugin::loadProgram().@n
   The other exception is with parameter change requests, see Plugin::requestParameterValueChange().@n
   Outputs are changed by the plugin and never modified by the host.

   If you are targetting VST2, make sure to order your parameters so that all inputs are before any outputs.
 */
static constexpr const uint32_t kParameterIsOutput = 0x10;

/**
   Parameter value is a trigger.@n
   This means the value resets back to its default after each process/run call.@n
   Cannot be used for output parameters.

   @note Only officially supported under LV2. For other formats DPF simulates the behaviour.
*/
static constexpr const uint32_t kParameterIsTrigger = 0x20 | kParameterIsBoolean;

/**
   Parameter should be hidden from the host and user-visible GUIs.@n
   It is still saved and handled as any regular parameter, just not visible to the user
   (for example in a host generated GUI)
 */
static constexpr const uint32_t kParameterIsHidden = 0x40;

/* --------------------------------------------------------------------------------------------------------------------
 * Base Plugin structs */

/**
   @defgroup BasePluginStructs Base Plugin Structs
   @{
 */

/**
   Parameter designation.@n
   Allows a parameter to be specially designated for a task, like bypass.

   Each designation is unique, there must be only one parameter that uses it.@n
   The use of designated parameters is completely optional.

   @note Designated parameters have strict ranges.
   @see ParameterRanges::adjustForDesignation()
 */
enum ParameterDesignation {
    kParameterDesignationNull = 0,

   /**
     Bypass designation.@n
     When on (> 0.5f), it means the plugin must run in a bypassed state.
    */
    kParameterDesignationBypass = 1
};

/**
   Predefined Port Groups Ids.

   This enumeration provides a few commonly used groups for convenient use in plugins.
   For preventing conflicts with user code, negative values are used here.
   When rolling your own port groups, you MUST start their group ids from 0 and they MUST be sequential.

   @see PortGroup
 */
enum PredefinedPortGroupsIds {
    kPortGroupNone   = 0xffffffff, // -1
    kPortGroupMono   = 0xfffffffe, // -2
    kPortGroupStereo = 0xfffffffd, // -3
};

/**
   Audio Port.

   Can be used as CV port by specifying kAudioPortIsCV in hints,@n
   but this is only supported in LV2 and JACK standalone formats.
 */
struct AudioPort {
    // @see AudioPortHints
    uint32_t hints;
    
    /**
      The group id that this audio/cv port belongs to.
      No group is assigned by default.

      You can use a group from PredefinedPortGroups or roll your own.@n
      When rolling your own port groups, you MUST start their group ids from 0 and they MUST be sequential.
      @see PortGroup, Plugin::initPortGroup
    */
    uint32_t groupId;

    String name;

   /**
      The symbol of this audio port.@n
      An audio port symbol is a short restricted name used as a machine and human readable identifier.@n
      The first character must be one of _, a-z or A-Z and subsequent characters can be from _, a-z, A-Z and 0-9.
      @note Audio port and parameter symbols MUST be unique within a plugin instance.
    */
    String symbol;

    AudioPort() noexcept
        : hints(0x0),
          groupId(kPortGroupNone),
          name(),
          symbol() {}
};

/**
   Parameter ranges.@n
   This is used to set the default, minimum and maximum values of a parameter.

   By default a parameter has 0.0 as minimum, 1.0 as maximum and 0.0 as default.@n
   When changing this struct values you must ensure maximum > minimum and default is within range.
 */
struct ParameterRanges {
    float min;
    float max;
    float defaultValue;

    // Default constructor
    constexpr ParameterRanges() noexcept
        : min(0.0f),
          max(1.0f),
          defaultValue(0.0f) {}

    // Constructor using custom values.
    constexpr ParameterRanges(const float df, const float mn, const float mx) noexcept
        : min(mn),
          max(mx),
          defaultValue(df) {}

    // Fix the default value within range.
    void fixDefault() noexcept
    {
        fixValue(defaultValue);
    }

    // Fix a value within range.
    void fixValue(float& value) const noexcept
    {
        if (value < min)
            value = min;
        else if (value > max)
            value = max;
    }

    // Get a fixed value within range.
    float getFixedValue(const float value) const noexcept
    {
        if (value <= min)
            return min;
        if (value >= max)
            return max;
        return value;
    }

    // Get a value normalized to 0.0<->1.0.
    float getNormalizedValue(const float value) const noexcept
    {
        const float normValue = (value - min) / (max - min);

        if (normValue <= 0.0f)
            return 0.0f;
        if (normValue >= 1.0f)
            return 1.0f;
        return normValue;
    }

    // Get a value normalized to 0.0<->1.0, fixed within range.
    float getFixedAndNormalizedValue(const float value) const noexcept
    {
        if (value <= min)
            return 0.0f;
        if (value >= max)
            return 1.0f;

        const float normValue = (value - min) / (max - min);

        if (normValue <= 0.0f)
            return 0.0f;
        if (normValue >= 1.0f)
            return 1.0f;

        return normValue;
    }

    // Get a proper value previously normalized to 0.0<->1.0.
    float getUnnormalizedValue(const float value) const noexcept
    {
        if (value <= 0.0f)
            return min;
        if (value >= 1.0f)
            return max;

        return value * (max - min) + min;
    }

    double getUnnormalizedValue(const double value) const noexcept
    {
        if (value <= 0.0)
            return min;
        if (value >= 1.0)
            return max;

        return value * (max - min) + min;
    }
};

/**
   Parameter enumeration value.@n
   A string representation of a plugin parameter value.@n
   Used together can be used to give meaning to parameter values, working as an enumeration.
 */
struct ParameterEnumerationValue {
    float value;
    String label;

    ParameterEnumerationValue() noexcept
        : value(0.0f),
          label() {}

    ParameterEnumerationValue(float v, const char* l) noexcept
        : value(v),
          label(l) {}
};

/**
   Details around parameter enumeration values.@n
   Wraps ParameterEnumerationValues and provides a few extra details to the host about these values.
 */
struct ParameterEnumerationValues {
    // Number of elements allocated in @values.
    uint8_t count;

   /**
      Whether the host is to be restricted to only use enumeration values.

      @note This mode is only a hint! Not all hosts and plugin formats support this mode.
    */
    bool restrictedMode;

    // Whether to take ownership of the @p values pointer. Defaults to true unless stated otherwise.
    bool deleteLater;

   /**
      Array of @ParameterEnumerationValue items.@n
      When assining this pointer manually, it must be allocated on the heap with `new ParameterEnumerationValue[count]`.@n
      The array pointer will be automatically deleted later unless @p deleteLater is set to false.
    */
    ParameterEnumerationValue* values;

    constexpr ParameterEnumerationValues() noexcept
        : count(0),
          restrictedMode(false),
          values(nullptr),
          deleteLater(true) {}

   /**
      Constructor using custom values.@n
      When using this constructor the pointer to @values MUST have been statically declared.@n
      It will not be automatically deleted later.
    */
    constexpr ParameterEnumerationValues(uint32_t c, bool r, ParameterEnumerationValue* v) noexcept
        : count(c),
          restrictedMode(r),
          values(v),
          deleteLater(false) {}

    // constexpr
    ~ParameterEnumerationValues() noexcept
    {
        if (deleteLater)
            delete[] values;
    }
};

struct Parameter {
    // Hints describing this parameter. @see ParameterHints
    uint32_t hints;
    // Full name
    String name; 
    // (Optional) The full name is used when the short one is missing.
    String shortName;
    // Unique ID. The first character must be [a-zA-Z_], and subsequent characters must be [a-zA-Z0-9_]
    String symbol;
    // (Optional) The unit of this parameter. This means something like "dB", "kHz" and "ms".@n
    String unit;
    // (Option & LV2 only)
    String description;
    ParameterRanges ranges;
    ParameterEnumerationValues enumValues;
    ParameterDesignation designation;

   /**
      MIDI CC to use by default on this parameter.@n
      A value of 0 or 32 (bank change) is considered invalid.@n
      Must also be less or equal to 120.
      @note This value is only a hint! Hosts might map it automatically or completely ignore it.
    */
    uint8_t midiCC;

   /**
      The group id that this parameter belongs to.
      No group is assigned by default.

      You can use a group from PredefinedPortGroups or roll your own.@n
      When rolling your own port groups, you MUST start their group ids from 0 and they MUST be sequential.
      @see PortGroup, Plugin::initPortGroup
    */
    uint32_t groupId;

    Parameter() noexcept
        : hints(0x0),
          name(),
          shortName(),
          symbol(),
          unit(),
          ranges(),
          enumValues(),
          designation(kParameterDesignationNull),
          midiCC(0),
          groupId(kPortGroupNone) {}

    Parameter(uint32_t h, const char* n, const char* s, const char* u, float def, float min, float max) noexcept
        : hints(h),
          name(n),
          shortName(),
          symbol(s),
          unit(u),
          ranges(def, min, max),
          enumValues(),
          designation(kParameterDesignationNull),
          midiCC(0),
          groupId(kPortGroupNone) {}

#ifdef DISTRHO_PROPER_CPP11_SUPPORT
   /**
      Constructor using custom values and enumeration.
      Assumes enumeration details should have `restrictedMode` on.
    */
    Parameter(uint32_t h, const char* n, const char* s, const char* u, float def, float min, float max,
              uint8_t evcount, ParameterEnumerationValue ev[]) noexcept
        : hints(h),
          name(n),
          shortName(),
          symbol(s),
          unit(u),
          ranges(def, min, max),
          enumValues(evcount, true, ev),
          designation(kParameterDesignationNull),
          midiCC(0),
          groupId(kPortGroupNone) {}
#endif

    void initDesignation(ParameterDesignation d) noexcept
    {
        designation = d;

        switch (d)
        {
        case kParameterDesignationNull:
            break;
        case kParameterDesignationBypass:
            hints      = kParameterIsAutomatable|kParameterIsBoolean|kParameterIsInteger;
            name       = "Bypass";
            shortName  = "Bypass";
            symbol     = "dpf_bypass";
            unit       = "";
            midiCC     = 0;
            groupId    = kPortGroupNone;
            ranges.defaultValue = 0.0f;
            ranges.min = 0.0f;
            ranges.max = 1.0f;
            break;
        }
    }
};

/**
   Port Group.@n
   Allows to group together audio/cv ports or parameters.

   Each unique group MUST have an unique symbol and a name.
   A group can be applied to both inputs and outputs (at the same time).
   The same group cannot be used in audio ports and parameters.

   When both audio and parameter groups are used, audio groups MUST be defined first.
   That is, group indexes start with audio ports, then parameters.

   An audio port group logically combines ports which should be considered part of the same stream.@n
   For example, two audio ports in a group may form a stereo stream.

   A parameter group provides meta-data to the host to indicate that some parameters belong together.

   The use of port groups is completely optional.

   @see Plugin::initPortGroup, AudioPort::group, Parameter::group
 */
struct PortGroup {
    String name;

    // Unique ID & ideally short.
    // The first character must be [a-zA-Z_], and subsequent characters must be [a-zA-Z0-9_]
    String symbol;
};

struct MidiEvent {
   /**
      Size of internal data.
    */
    static constexpr const uint32_t kDataSize = 4;

   /**
      Time offset in frames.
    */
    uint32_t frame;

   /**
      Number of bytes used.
    */
    uint32_t size;

   /**
      MIDI data.@n
      If size > kDataSize, dataExt is used (otherwise null).

      When dataExt is used, the event holder is responsible for
      keeping the pointer valid during the entirety of the run function.
    */
    uint8_t        data[kDataSize];
    const uint8_t* dataExt;
};

struct BarBeatTick {
    // Current bar. Always starts from 1
    int32_t bar;
    // Current beat. Always starts from 1
    int32_t beat;

    /**
        Current tick within beat.@n
        Should always be >= 0 and < @a ticksPerBeat.@n
        The first tick is tick '0'.
        @note Fraction part of tick is only available on some plugin formats.
    */
    double tick;
    // Number of ticks within a beat.@n
    // Usually a moderately large integer with many denominators, such as 1920.0.
    double ticksPerBeat;
    //  Number of ticks that have elapsed between frame 0 and the first beat of the current measure.
    double barStartTick;
    // Time signature "numerator".
    float timeSigNumerator;
    // Time signature "denominator".
    float timeSigDenominator;
    double bpm;
};

struct TimePosition {
    // This value is always supported
    bool isPlaying;
    // If false, this feature is unsupported and you must not read from this struct.
    bool bbtSupported;

    // This value is always supported
    // Current host transport position in frames.
    // @note This value is not always monotonic, with some plugin hosts assigning it based on a source that can accumulate rounding errors.
    uint64_t frame;

    BarBeatTick bbt;

    TimePosition() {
        memset(this, 0, sizeof(*this));
    }
};

// -----------------------------------------------------------------------

#include "DistrhoPluginInfo.h"

// -----------------------------------------------------------------------
// Check if all required macros are defined

#ifndef DISTRHO_PLUGIN_NAME
# error DISTRHO_PLUGIN_NAME undefined!
#endif

#ifndef DISTRHO_PLUGIN_NUM_INPUTS
# error DISTRHO_PLUGIN_NUM_INPUTS undefined!
#endif

#ifndef DISTRHO_PLUGIN_NUM_OUTPUTS
# error DISTRHO_PLUGIN_NUM_OUTPUTS undefined!
#endif

#ifndef DISTRHO_PLUGIN_URI
# error DISTRHO_PLUGIN_URI undefined!
#endif

// -----------------------------------------------------------------------
// Define optional macros if not done yet

#ifndef DISTRHO_PLUGIN_NUM_PARAMS
# define DISTRHO_PLUGIN_NUM_PARAMS 0
#endif

#ifndef DISTRHO_PLUGIN_HAS_UI
# define DISTRHO_PLUGIN_HAS_UI 0
#endif

#ifndef DISTRHO_PLUGIN_HAS_EXTERNAL_UI
# define DISTRHO_PLUGIN_HAS_EXTERNAL_UI 0
#endif

#ifndef DISTRHO_PLUGIN_IS_RT_SAFE
# define DISTRHO_PLUGIN_IS_RT_SAFE 0
#endif

#ifndef DISTRHO_PLUGIN_IS_SYNTH
# define DISTRHO_PLUGIN_IS_SYNTH 0
#endif

#ifndef DISTRHO_PLUGIN_WANT_DIRECT_ACCESS
# define DISTRHO_PLUGIN_WANT_DIRECT_ACCESS 0
#endif

#ifndef DISTRHO_PLUGIN_WANT_LATENCY
# define DISTRHO_PLUGIN_WANT_LATENCY 0
#endif

#ifndef DISTRHO_PLUGIN_WANT_MIDI_OUTPUT
# define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 0
#endif

#ifndef DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST
# define DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST 0
#endif

#ifndef DISTRHO_PLUGIN_WANT_TIMEPOS
# define DISTRHO_PLUGIN_WANT_TIMEPOS 0
#endif

#ifndef DISTRHO_UI_FILE_BROWSER
# if defined(DGL_FILE_BROWSER_DISABLED) || DISTRHO_PLUGIN_HAS_EXTERNAL_UI
#  define DISTRHO_UI_FILE_BROWSER 0
# else
#  define DISTRHO_UI_FILE_BROWSER 1
# endif
#endif

#ifndef DISTRHO_UI_USER_RESIZABLE
# define DISTRHO_UI_USER_RESIZABLE 0
#endif

#ifndef DISTRHO_UI_USE_NANOVG
# define DISTRHO_UI_USE_NANOVG 0
#endif

// -----------------------------------------------------------------------
// Define DISTRHO_PLUGIN_HAS_EMBED_UI if needed

#ifndef DISTRHO_PLUGIN_HAS_EMBED_UI
# if (defined(DGL_OPENGL) && defined(HAVE_OPENGL))
#  define DISTRHO_PLUGIN_HAS_EMBED_UI 1
# else
#  define DISTRHO_PLUGIN_HAS_EMBED_UI 0
# endif
#endif

// -----------------------------------------------------------------------
// Define DISTRHO_UI_URI if needed

#ifndef DISTRHO_UI_URI
# define DISTRHO_UI_URI DISTRHO_PLUGIN_URI "#DPF_UI"
#endif

// -----------------------------------------------------------------------
// Test if synth has audio outputs

#if DISTRHO_PLUGIN_IS_SYNTH && DISTRHO_PLUGIN_NUM_OUTPUTS == 0
# error Synths need audio output to work!
#endif

// -----------------------------------------------------------------------
// Enable MIDI input if synth, test if midi-input disabled when synth

#ifndef DISTRHO_PLUGIN_WANT_MIDI_INPUT
# define DISTRHO_PLUGIN_WANT_MIDI_INPUT DISTRHO_PLUGIN_IS_SYNTH
#elif DISTRHO_PLUGIN_IS_SYNTH && ! DISTRHO_PLUGIN_WANT_MIDI_INPUT
# error Synths need MIDI input to work!
#endif

// -----------------------------------------------------------------------
// Disable file browser if using external UI

#if DISTRHO_UI_FILE_BROWSER && DISTRHO_PLUGIN_HAS_EXTERNAL_UI
# warning file browser APIs do not work for external UIs
# undef DISTRHO_UI_FILE_BROWSER 0
# define DISTRHO_UI_FILE_BROWSER 0
#endif

// -----------------------------------------------------------------------
// Disable UI if DGL or external UI is not available

#if (defined(DGL_OPENGL) && ! defined(HAVE_OPENGL))
# undef DISTRHO_PLUGIN_HAS_EMBED_UI
# define DISTRHO_PLUGIN_HAS_EMBED_UI 0
#endif

#if DISTRHO_PLUGIN_HAS_UI && ! DISTRHO_PLUGIN_HAS_EMBED_UI && ! DISTRHO_PLUGIN_HAS_EXTERNAL_UI
# undef DISTRHO_PLUGIN_HAS_UI
# define DISTRHO_PLUGIN_HAS_UI 0
#endif

// -----------------------------------------------------------------------
// Make sure both default width and height are provided

#if defined(DISTRHO_UI_DEFAULT_WIDTH) && !defined(DISTRHO_UI_DEFAULT_HEIGHT)
# error DISTRHO_UI_DEFAULT_WIDTH is defined but DISTRHO_UI_DEFAULT_HEIGHT is not
#endif

#if defined(DISTRHO_UI_DEFAULT_HEIGHT) && !defined(DISTRHO_UI_DEFAULT_WIDTH)
# error DISTRHO_UI_DEFAULT_HEIGHT is defined but DISTRHO_UI_DEFAULT_WIDTH is not
#endif

// -----------------------------------------------------------------------
// Prevent users from messing about with DPF internals

#ifdef DISTRHO_UI_IS_STANDALONE
# error DISTRHO_UI_IS_STANDALONE must not be defined
#endif

// -----------------------------------------------------------------------

#endif // DISTRHO_DETAILS_HPP_INCLUDED
