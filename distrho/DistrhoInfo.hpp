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

#ifdef DOXYGEN

#include "src/DistrhoDefines.h"


/* ------------------------------------------------------------------------------------------------------------
 * Intro */

/**
   @mainpage DISTRHO %Plugin Framework

   DISTRHO %Plugin Framework (or @b DPF for short)
   is a plugin framework designed to make development of new plugins an easy and enjoyable task.@n
   It allows developers to create plugins with custom UIs using a simple C++ API.@n
   The framework facilitates exporting various different plugin formats from the same code-base.

   DPF can build for LADSPA, DSSI, LV2, VST2, VST3 and CLAP formats.@n
   A JACK/Standalone mode is also available, allowing you to quickly test plugins.

   @section Macros
   You start by creating a "DistrhoPluginInfo.h" file describing the plugin via macros, see @ref PluginMacros.@n
   This file is included during compilation of the main DPF code to select which features to activate for each plugin format.

   For example, a plugin (with %UI) that use states will require LV2 hosts to support Atom and Worker extensions for
   message passing from the %UI to the (DSP) plugin.@n
   If your plugin does not make use of states, the Worker extension is not set as a required feature.

   @section Plugin
   The next step is to create your plugin code by subclassing DPF's Plugin class.@n
   You need to pass the number of parameters in the constructor and also the number of programs and states, if any.

   @section Examples
   Let's begin with some examples.@n
   Here is one of a stereo audio plugin that simply mutes the host output:
   @code
   /* DPF plugin include */
   #include "DistrhoPlugin.hpp"

   /**
      Our custom plugin class.
      Subclassing `Plugin` from DPF is how this all works.

      By default, only information-related functions and `run` are pure virtual (that is, must be reimplemented).
      When enabling certain features (such as programs or states, more on that below), a few extra functions also need to be reimplemented.
    */
   class MutePlugin : public Plugin
   {
   public:
     /**
        Plugin class constructor.
      */
      MutePlugin()
          : Plugin(0, 0, 0) // 0 parameters, 0 programs and 0 states
      {
      }

   protected:
     /* ----------------------------------------------------------------------------------------
      * Information */

     /**
        Get the plugin label.
        This label is a short restricted name consisting of only _, a-z, A-Z and 0-9 characters.
      */
      const char* getLabel() const override
      {
          return "Mute";
      }

     /**
        Get the plugin author/maker.
      */
      const char* getMaker() const override
      {
          return "DPF";
      }

     /**
        Get the plugin license name (a single line of text).
        For commercial plugins this should return some short copyright information.
      */
      const char* getLicense() const override
      {
          return "MIT";
      }

     /**
        Get the plugin version, in hexadecimal.
      */
      uint32_t getVersion() const override
      {
          return d_version(1, 0, 0);
      }

     /**
        Get the plugin unique Id.
        This value is used by LADSPA, DSSI, VST2 and VST3 plugin formats.
      */
      int64_t getUniqueId() const override
      {
          return d_cconst('M', 'u', 't', 'e');
      }

     /* ----------------------------------------------------------------------------------------
      * Audio/MIDI Processing */

     /**
        Run/process function for plugins without MIDI input.
      */
      void run(const float**, float** outputs, uint32_t frames) override
      {
          // get the left and right audio outputs
          float* const outL = outputs[0];
          float* const outR = outputs[1];

          // mute audio
          std::memset(outL, 0, sizeof(float)*frames);
          std::memset(outR, 0, sizeof(float)*frames);
      }
   };

   /**
      Create an instance of the Plugin class.
      This is the entry point for DPF plugins.
      DPF will call this to either create an instance of your plugin for the host or to fetch some initial information for internal caching.
    */
   Plugin* createPlugin()
   {
      return new MutePlugin();
   }
   @endcode

   See the Plugin class for more information.

   @section Parameters
   A plugin is nothing without parameters.@n
   In DPF parameters can be inputs or outputs.@n
   They have hints to describe how they behave plus a name and a symbol identifying them.@n
   Parameters also have 'ranges' - a minimum, maximum and default value.

   Input parameters are by default "read-only": the plugin can read them but not change them.
   (there are exceptions and possibly a request to the host to change values, more on that below)@n
   It's the host responsibility to save, restore and set input parameters.

   Output parameters can be changed at anytime by the plugin.@n
   The host will simply read their values and never change them.

   Here's an example of an audio plugin that has 1 input parameter:
   @code
   class GainPlugin : public Plugin
   {
   public:
     /**
        Plugin class constructor.
        You must set all parameter values to their defaults, matching ParameterRanges::defaultValue.
      */
      GainPlugin()
          : Plugin(1, 0, 0), // 1 parameter, 0 programs and 0 states
            fGain(1.0f)
      {
      }

   protected:
     /* ----------------------------------------------------------------------------------------
      * Information */

      const char* getLabel() const override
      {
          return "Gain";
      }

      const char* getMaker() const override
      {
          return "DPF";
      }

      const char* getLicense() const override
      {
          return "MIT";
      }

      uint32_t getVersion() const override
      {
          return d_version(1, 0, 0);
      }

      int64_t getUniqueId() const override
      {
          return d_cconst('G', 'a', 'i', 'n');
      }

     /* ----------------------------------------------------------------------------------------
      * Init */

     /**
        Initialize a parameter.
        This function will be called once, shortly after the plugin is created.
      */
      void initParameter(uint32_t index, Parameter& parameter) override
      {
          // we only have one parameter so we can skip checking the index

          parameter.hints      = kParameterIsAutomatable;
          parameter.name       = "Gain";
          parameter.symbol     = "gain";
          parameter.ranges.min = 0.0f;
          parameter.ranges.max = 2.0f;
          parameter.ranges.defaultValue = 1.0f;
      }

     /* ----------------------------------------------------------------------------------------
      * Internal data */

     /**
        Get the current value of a parameter.
      */
      float getParameterValue(uint32_t index) const override
      {
          // same as before, ignore index check

          return fGain;
      }

     /**
        Change a parameter value.
      */
      void setParameterValue(uint32_t index, float value) override
      {
          // same as before, ignore index check

          fGain = value;
      }

     /* ----------------------------------------------------------------------------------------
      * Audio/MIDI Processing */

      void run(const float**, float** outputs, uint32_t frames) override
      {
          // get the mono input and output
          const float* const in  = inputs[0];
          /* */ float* const out = outputs[0];

          // apply gain against all samples
          for (uint32_t i=0; i < frames; ++i)
              out[i] = in[i] * fGain;
      }

   private:
      float fGain;
   };
   @endcode

   See the Parameter struct for more information about parameters.

   This is a work-in-progress documentation page. MIDI, Latency, Time-Position and UI are still TODO.
*/

#if 0
   @section MIDI
   describe them

   @section Latency
   describe it

   @section Time-Position
   describe it

   @section UI
   describe them
#endif

/* ------------------------------------------------------------------------------------------------------------
 * Plugin Macros */

/**
   @defgroup PluginMacros Plugin Macros

   C Macros that describe your plugin. (defined in the "DistrhoPluginInfo.h" file)

   With these macros you can tell the host what features your plugin requires.@n
   Depending on which macros you enable, new functions will be available to call and/or override.

   All values are either integer or strings.@n
   For boolean-like values 1 means 'on' and 0 means 'off'.

   The values defined in this group are for documentation purposes only.@n
   All macros are disabled by default.

   Only 4 macros are required, they are:
    - @ref DISTRHO_PLUGIN_NAME
    - @ref DISTRHO_PLUGIN_NUM_INPUTS
    - @ref DISTRHO_PLUGIN_NUM_OUTPUTS
    - @ref DISTRHO_PLUGIN_URI
   
   Additionally, @ref DISTRHO_PLUGIN_CLAP_ID is required if building CLAP plugins.
   @{
 */

/**
   The plugin name.@n
   This is used to identify your plugin before a Plugin instance can be created.
   @note This macro is required.
 */
#define DISTRHO_PLUGIN_NAME "Plugin Name"

/**
   Number of audio inputs the plugin has.
   @note This macro is required.
 */
#define DISTRHO_PLUGIN_NUM_INPUTS 2

/**
   Number of audio outputs the plugin has.
   @note This macro is required.
 */
#define DISTRHO_PLUGIN_NUM_OUTPUTS 2

/**
   The plugin URI when exporting in LV2 format.
   @note This macro is required.
 */
#define DISTRHO_PLUGIN_URI "urn:distrho:name"

/**
   Whether the plugin has a custom %UI.
   @see DISTRHO_UI_USE_NANOVG
   @see UI
 */
#define DISTRHO_PLUGIN_HAS_UI 1

/**
   Whether the plugin processing is realtime-safe.@n
   TODO - list rtsafe requirements
 */
#define DISTRHO_PLUGIN_IS_RT_SAFE 1

/**
   Whether the plugin is a synth.@n
   @ref DISTRHO_PLUGIN_WANT_MIDI_INPUT is automatically enabled when this is too.
   @see DISTRHO_PLUGIN_WANT_MIDI_INPUT
 */
#define DISTRHO_PLUGIN_IS_SYNTH 1

/**
   Request the minimum buffer size for the input and output event ports.@n
   Currently only used in LV2, with a default value of 2048 if unset.
 */
#define DISTRHO_PLUGIN_MINIMUM_BUFFER_SIZE 2048

/**
   Whether the plugin has an LV2 modgui.

   This will simply add a "rdfs:seeAlso <modgui.ttl>" on the LV2 manifest.@n
   It is up to you to create this file.
 */
#define DISTRHO_PLUGIN_USES_MODGUI 0

/**
   Enable direct access between the %UI and plugin code.
   @see UI::getPluginInstancePointer()
   @note DO NOT USE THIS UNLESS STRICTLY NECESSARY!!
         Try to avoid it at all costs!
 */
#define DISTRHO_PLUGIN_WANT_DIRECT_ACCESS 0

/**
   Whether the plugin introduces latency during audio or midi processing.
   @see Plugin::setLatency(uint32_t)
 */
#define DISTRHO_PLUGIN_WANT_LATENCY 1

/**
   Whether the plugin wants MIDI input.@n
   This is automatically enabled if @ref DISTRHO_PLUGIN_IS_SYNTH is true.
 */
#define DISTRHO_PLUGIN_WANT_MIDI_INPUT 1

/**
   Whether the plugin wants MIDI output.
   @see Plugin::writeMidiEvent(const MidiEvent&)
 */
#define DISTRHO_PLUGIN_WANT_MIDI_OUTPUT 1

/**
   Whether the plugin wants to change its own parameter inputs.@n
   Not all hosts or plugin formats support this,
   so Plugin::canRequestParameterValueChanges() can be used to query support at runtime.
   @see Plugin::requestParameterValueChange(uint32_t, float)
 */
#define DISTRHO_PLUGIN_WANT_PARAMETER_VALUE_CHANGE_REQUEST 1

/**
   Whether the plugin wants time position information from the host.
   @see Plugin::getTimePosition()
 */
#define DISTRHO_PLUGIN_WANT_TIMEPOS 1

/**
   Whether the %UI uses a custom toolkit implementation based on OpenGL.@n
   When enabled, the macros @ref DISTRHO_UI_CUSTOM_INCLUDE_PATH and @ref DISTRHO_UI_CUSTOM_WIDGET_TYPE are required.
 */
#define DISTRHO_UI_USE_CUSTOM 1

/**
   The include path to the header file used by the custom toolkit implementation.
   This path must be relative to dpf/distrho/DistrhoUI.hpp
   @see DISTRHO_UI_USE_CUSTOM
 */
#define DISTRHO_UI_CUSTOM_INCLUDE_PATH

/**
   The top-level-widget typedef to use for the custom toolkit.
   This widget class MUST be a subclass of DGL TopLevelWindow class.
   It is recommended that you keep this widget class inside the DGL namespace,
   and define widget type as e.g. MyCustomTopLevelWidget.
   @see DISTRHO_UI_USE_CUSTOM
 */
#define DISTRHO_UI_CUSTOM_WIDGET_TYPE

/**
   Default UI width to use when creating initial and temporary windows.@n
   Setting this macro allows to skip a temporary UI from being created in certain VST2 and VST3 hosts.
   (which would normally be done for knowing the UI size before host creates a window for it)

   Value must match 1x scale factor.

   When this macro is defined, the companion DISTRHO_UI_DEFAULT_HEIGHT macro must be defined as well.
 */
#define DISTRHO_UI_DEFAULT_WIDTH 300

/**
   Default UI height to use when creating initial and temporary windows.@n
   Setting this macro allows to skip a temporary UI from being created in certain VST2 and VST3 hosts.
   (which would normally be done for knowing the UI size before host creates a window for it)

   Value must match 1x scale factor.

   When this macro is defined, the companion DISTRHO_UI_DEFAULT_WIDTH macro must be defined as well.
 */
#define DISTRHO_UI_DEFAULT_HEIGHT 300

/**
   Whether the %UI uses NanoVG for drawing instead of the default raw OpenGL calls.@n
   When enabled your %UI instance will subclass @ref NanoWidget instead of @ref Widget.
 */
#define DISTRHO_UI_USE_NANOVG 1

/**
   Whether the %UI is resizable to any size by the user.@n
   By default this is false, and resizing is only allowed under the plugin UI control,@n
   Enabling this options makes it possible for the user to resize the plugin UI at anytime.
   @see UI::setGeometryConstraints(uint32_t, uint32_t, bool, bool)
 */
#define DISTRHO_UI_USER_RESIZABLE 1

/**
   The %UI URI when exporting in LV2 format.@n
   By default this is set to @ref DISTRHO_PLUGIN_URI with "#UI" as suffix.
 */
#define DISTRHO_UI_URI DISTRHO_PLUGIN_URI "#UI"

/**
   Custom LV2 category for the plugin.@n
   This is a single string, and can be one of the following values:

      - lv2:AllpassPlugin
      - lv2:AmplifierPlugin
      - lv2:AnalyserPlugin
      - lv2:BandpassPlugin
      - lv2:ChorusPlugin
      - lv2:CombPlugin
      - lv2:CompressorPlugin
      - lv2:ConstantPlugin
      - lv2:ConverterPlugin
      - lv2:DelayPlugin
      - lv2:DistortionPlugin
      - lv2:DynamicsPlugin
      - lv2:EQPlugin
      - lv2:EnvelopePlugin
      - lv2:ExpanderPlugin
      - lv2:FilterPlugin
      - lv2:FlangerPlugin
      - lv2:FunctionPlugin
      - lv2:GatePlugin
      - lv2:GeneratorPlugin
      - lv2:HighpassPlugin
      - lv2:InstrumentPlugin
      - lv2:LimiterPlugin
      - lv2:LowpassPlugin
      - lv2:MIDIPlugin
      - lv2:MixerPlugin
      - lv2:ModulatorPlugin
      - lv2:MultiEQPlugin
      - lv2:OscillatorPlugin
      - lv2:ParaEQPlugin
      - lv2:PhaserPlugin
      - lv2:PitchPlugin
      - lv2:ReverbPlugin
      - lv2:SimulatorPlugin
      - lv2:SpatialPlugin
      - lv2:SpectralPlugin
      - lv2:UtilityPlugin
      - lv2:WaveshaperPlugin

   See http://lv2plug.in/ns/lv2core for more information.
 */
#define DISTRHO_PLUGIN_LV2_CATEGORY "lv2:Plugin"

/**
   Custom VST3 categories for the plugin.@n
   This is a single concatenated string of categories, separated by a @c |.

   Each effect category can be one of the following values:

      - Fx
      - Fx|Ambisonics
      - Fx|Analyzer
      - Fx|Delay
      - Fx|Distortion
      - Fx|Dynamics
      - Fx|EQ
      - Fx|Filter
      - Fx|Instrument
      - Fx|Instrument|External
      - Fx|Spatial
      - Fx|Generator
      - Fx|Mastering
      - Fx|Modulation
      - Fx|Network
      - Fx|Pitch Shift
      - Fx|Restoration
      - Fx|Reverb
      - Fx|Surround
      - Fx|Tools

   Each instrument category can be one of the following values:

      - Instrument
      - Instrument|Drum
      - Instrument|External
      - Instrument|Piano
      - Instrument|Sampler
      - Instrument|Synth
      - Instrument|Synth|Sampler

   And extra categories possible for any plugin type:

      - Mono
      - Stereo
 */
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Stereo"

/**
   Custom CLAP features for the plugin.@n
   This is a list of features defined as a string array body, without the terminating @c , or nullptr.

   A top-level category can be set as feature and be one of the following values:

      - instrument
      - audio-effect
      - note-effect
      - analyzer

   The following sub-categories can also be set:

      - synthesizer
      - sampler
      - drum
      - drum-machine

      - filter
      - phaser
      - equalizer
      - de-esser
      - phase-vocoder
      - granular
      - frequency-shifter
      - pitch-shifter

      - distortion
      - transient-shaper
      - compressor
      - limiter

      - flanger
      - chorus
      - delay
      - reverb

      - tremolo
      - glitch

      - utility
      - pitch-correction
      - restoration

      - multi-effects

      - mixing
      - mastering

   And finally the following audio capabilities can be set:

      - mono
      - stereo
      - surround
      - ambisonic
*/
#define DISTRHO_PLUGIN_CLAP_FEATURES "audio-effect", "stereo"

/**
   The plugin id when exporting in CLAP format, in reverse URI form.
   @note This macro is required when building CLAP plugins
*/
#define DISTRHO_PLUGIN_CLAP_ID "studio.kx.distrho.effect"

/** @} */

/* ------------------------------------------------------------------------------------------------------------
 * Plugin Macros */

/**
   @defgroup ExtraPluginMacros Extra Plugin Macros

   C Macros to customize DPF behaviour.

   These are macros that do not set plugin features or information, but instead change DPF internals.
   They are all optional.

   Unless stated otherwise, values are assumed to be a simple/empty define.
   @{
 */

/**
   Whether to enable runtime plugin tests.@n
   This will check, during initialization of the plugin, if parameters, programs and states are setup properly.@n
   Useful to enable as part of CI, can safely be skipped.@n
   Under DPF makefiles this can be enabled by using `make DPF_RUNTIME_TESTING=true`.

   @note Some checks are only available with the GCC compiler,
         for detecting if a virtual function has been reimplemented.
 */
#define DPF_RUNTIME_TESTING

/**
   Whether to show parameter outputs in the VST2 plugins.@n
   This is disabled (unset) by default, as the VST2 format has no notion of read-only parameters.
 */
#define DPF_VST_SHOW_PARAMETER_OUTPUTS

/**
   Disable all file browser related code.@n
   Must be set as compiler macro when building DGL. (e.g. `CXXFLAGS="-DDGL_FILE_BROWSER_DISABLED"`)
 */
#define DGL_FILE_BROWSER_DISABLED

/**
   Disable resource files, like internally used fonts.@n
   Must be set as compiler macro when building DGL. (e.g. `CXXFLAGS="-DDGL_NO_SHARED_RESOURCES"`)
 */
#define DGL_NO_SHARED_RESOURCES

/**
   Whether to use OpenGL3 instead of the default OpenGL2 compatility profile.
   Under DPF makefiles this can be enabled by using `make USE_OPENGL3=true` on the dgl build step.

   @note This is experimental and incomplete, contributions are welcome and appreciated.
 */
#define DGL_USE_OPENGL3

/** @} */

/* ------------------------------------------------------------------------------------------------------------
 * Namespace Macros */

/**
   @defgroup NamespaceMacros Namespace Macros

   C Macros to use and customize DPF namespaces.

   These are macros that serve as helpers around C++ namespaces, and also as a way to set custom namespaces during a build.
   @{
 */

/* TODO
 *
 * DISTRHO_MACRO_AS_STRING_VALUE
 * DISTRHO_MACRO_AS_STRING
 * DISTRHO_PROPER_CPP11_SUPPORT
 * DONT_SET_USING_DISTRHO_NAMESPACE
 *
 */

// -----------------------------------------------------------------------------------------------------------


#endif // DOXYGEN
