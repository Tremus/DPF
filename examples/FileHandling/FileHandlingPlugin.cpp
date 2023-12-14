/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2020 Filipe Coelho <falktx@falktx.com>
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

// -----------------------------------------------------------------------------------------------------------

// Plugin to demonstrate File handling within DPF.
struct FileHandlingExamplePlugin
{
    PluginPrivateData data;
    float fParameters[kParameterCount];

    FileHandlingExamplePlugin()
    {
        std::memset(fParameters, 0, sizeof(fParameters));
    }

private:
    DISTRHO_DECLARE_NON_COPYABLE(FileHandlingExamplePlugin)
};

/* --------------------------------------------------------------------------------------------------------
 * Information */

const char* plugin_getName(void* ptr)
{
    return DISTRHO_PLUGIN_NAME;
}

const char* plugin_getLabel(void* ptr)
{
    return "FileHandling";
}

const char* plugin_getDescription(void* ptr)
{
    return "Plugin to demonstrate File handling.";
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
    return d_version(0, 0, 0);
}

int64_t plugin_getUniqueId(void* ptr)
{
    return d_cconst('d', 'F', 'i', 'H');
}

/* --------------------------------------------------------------------------------------------------------
* Init */

void plugin_initAudioPort(void* ptr, bool input, uint32_t index, AudioPort& port)
{
    // treat meter audio ports as stereo
    port.groupId = kPortGroupMono;

    // everything else is as default
    plugin_default_initAudioPort(input, index, port);
}

void plugin_initParameter(void*, uint32_t index, Parameter& param)
{
    param.hints = kParameterIsOutput | kParameterIsInteger;

    switch (index)
    {
    case kParameterFileSize1:
        param.name   = "Size #1";
        param.symbol = "size1";
        break;
    case kParameterFileSize2:
        param.name   = "Size #2";
        param.symbol = "size2";
        break;
    case kParameterFileSize3:
        param.name   = "Size #3";
        param.symbol = "size3";
        break;
    }
}

/* --------------------------------------------------------------------------------------------------------
* Internal data */

float plugin_getParameterValue(void* ptr, uint32_t index)
{
    FileHandlingExamplePlugin* plugin = (FileHandlingExamplePlugin*)ptr;
    return plugin->fParameters[index];
}

// Since we have no parameters inputs in this example, so we do nothing with the function.
void plugin_setParameterValue(void*, uint32_t, float) {}

/* --------------------------------------------------------------------------------------------------------
* Audio/MIDI Processing */

void plugin_activate(void*) {}
void plugin_deactivate(void*) {}

void plugin_run(void*, const float** inputs, float** outputs, uint32_t frames)
{
    // Bypass audio
    if (outputs[0] != inputs[0])
        std::memcpy(outputs[0], inputs[0], sizeof(float)*frames);
}

void plugin_bufferSizeChanged(void*, uint32_t newBufferSize) {}
void plugin_sampleRateChanged(void*, double newSampleRate) {}

/* ------------------------------------------------------------------------------------------------------------
 * Plugin entry point, called by DPF to create a new plugin instance. */

void* createPlugin()
{
    return new FileHandlingExamplePlugin();
}

void destroyPlugin(void* ptr)
{
    FileHandlingExamplePlugin* plugin = (FileHandlingExamplePlugin*)ptr;
    delete plugin;
}

PluginPrivateData* getPluginPrivateData(void* ptr)
{
    FileHandlingExamplePlugin* plugin = (FileHandlingExamplePlugin*)ptr;
    return &plugin->data;
}