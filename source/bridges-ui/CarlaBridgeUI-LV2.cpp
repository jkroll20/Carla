/*
 * Carla Bridge UI, LV2 version
 * Copyright (C) 2011-2017 Filipe Coelho <falktx@falktx.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the doc/GPL.txt file.
 */

#include "CarlaBridgeUI.hpp"
#include "CarlaLibUtils.hpp"
#include "CarlaLv2Utils.hpp"
#include "CarlaMIDI.h"
#include "LinkedList.hpp"

#include "water/files/File.h"

#include <string>
#include <vector>

#define URI_CARLA_ATOM_WORKER "http://kxstudio.sf.net/ns/carla/atomWorker"

using water::File;

CARLA_BRIDGE_START_NAMESPACE

// -----------------------------------------------------

static double gInitialSampleRate = 44100.0;

// LV2 URI Map Ids
const uint32_t CARLA_URI_MAP_ID_NULL                   =  0;
const uint32_t CARLA_URI_MAP_ID_ATOM_BLANK             =  1;
const uint32_t CARLA_URI_MAP_ID_ATOM_BOOL              =  2;
const uint32_t CARLA_URI_MAP_ID_ATOM_CHUNK             =  3;
const uint32_t CARLA_URI_MAP_ID_ATOM_DOUBLE            =  4;
const uint32_t CARLA_URI_MAP_ID_ATOM_EVENT             =  5;
const uint32_t CARLA_URI_MAP_ID_ATOM_FLOAT             =  6;
const uint32_t CARLA_URI_MAP_ID_ATOM_INT               =  7;
const uint32_t CARLA_URI_MAP_ID_ATOM_LITERAL           =  8;
const uint32_t CARLA_URI_MAP_ID_ATOM_LONG              =  9;
const uint32_t CARLA_URI_MAP_ID_ATOM_NUMBER            = 10;
const uint32_t CARLA_URI_MAP_ID_ATOM_OBJECT            = 11;
const uint32_t CARLA_URI_MAP_ID_ATOM_PATH              = 12;
const uint32_t CARLA_URI_MAP_ID_ATOM_PROPERTY          = 13;
const uint32_t CARLA_URI_MAP_ID_ATOM_RESOURCE          = 14;
const uint32_t CARLA_URI_MAP_ID_ATOM_SEQUENCE          = 15;
const uint32_t CARLA_URI_MAP_ID_ATOM_SOUND             = 16;
const uint32_t CARLA_URI_MAP_ID_ATOM_STRING            = 17;
const uint32_t CARLA_URI_MAP_ID_ATOM_TUPLE             = 18;
const uint32_t CARLA_URI_MAP_ID_ATOM_URI               = 19;
const uint32_t CARLA_URI_MAP_ID_ATOM_URID              = 20;
const uint32_t CARLA_URI_MAP_ID_ATOM_VECTOR            = 21;
const uint32_t CARLA_URI_MAP_ID_ATOM_TRANSFER_ATOM     = 22;
const uint32_t CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT    = 23;
const uint32_t CARLA_URI_MAP_ID_BUF_MAX_LENGTH         = 24;
const uint32_t CARLA_URI_MAP_ID_BUF_MIN_LENGTH         = 25;
const uint32_t CARLA_URI_MAP_ID_BUF_NOMINAL_LENGTH     = 26;
const uint32_t CARLA_URI_MAP_ID_BUF_SEQUENCE_SIZE      = 27;
const uint32_t CARLA_URI_MAP_ID_LOG_ERROR              = 28;
const uint32_t CARLA_URI_MAP_ID_LOG_NOTE               = 29;
const uint32_t CARLA_URI_MAP_ID_LOG_TRACE              = 30;
const uint32_t CARLA_URI_MAP_ID_LOG_WARNING            = 31;
const uint32_t CARLA_URI_MAP_ID_TIME_POSITION          = 32; // base type
const uint32_t CARLA_URI_MAP_ID_TIME_BAR               = 33; // values
const uint32_t CARLA_URI_MAP_ID_TIME_BAR_BEAT          = 34;
const uint32_t CARLA_URI_MAP_ID_TIME_BEAT              = 35;
const uint32_t CARLA_URI_MAP_ID_TIME_BEAT_UNIT         = 36;
const uint32_t CARLA_URI_MAP_ID_TIME_BEATS_PER_BAR     = 37;
const uint32_t CARLA_URI_MAP_ID_TIME_BEATS_PER_MINUTE  = 38;
const uint32_t CARLA_URI_MAP_ID_TIME_FRAME             = 39;
const uint32_t CARLA_URI_MAP_ID_TIME_FRAMES_PER_SECOND = 40;
const uint32_t CARLA_URI_MAP_ID_TIME_SPEED             = 41;
const uint32_t CARLA_URI_MAP_ID_TIME_TICKS_PER_BEAT    = 42;
const uint32_t CARLA_URI_MAP_ID_MIDI_EVENT             = 43;
const uint32_t CARLA_URI_MAP_ID_PARAM_SAMPLE_RATE      = 44;
const uint32_t CARLA_URI_MAP_ID_UI_WINDOW_TITLE        = 45;
const uint32_t CARLA_URI_MAP_ID_CARLA_ATOM_WORKER      = 46;
const uint32_t CARLA_URI_MAP_ID_CARLA_TRANSIENT_WIN_ID = 47;
const uint32_t CARLA_URI_MAP_ID_COUNT                  = 48;

// LV2 Feature Ids
enum CarlaLv2Features {
    // DSP features
    kFeatureIdLogs = 0,
    kFeatureIdOptions,
    kFeatureIdPrograms,
    kFeatureIdStateMakePath,
    kFeatureIdStateMapPath,
    kFeatureIdUriMap,
    kFeatureIdUridMap,
    kFeatureIdUridUnmap,
    kFeatureIdUiIdleInterface,
    kFeatureIdUiFixedSize,
    kFeatureIdUiMakeResident,
    kFeatureIdUiMakeResident2,
    kFeatureIdUiNoUserResize,
    kFeatureIdUiParent,
    kFeatureIdUiPortMap,
    kFeatureIdUiPortSubscribe,
    kFeatureIdUiResize,
    kFeatureIdUiTouch,
    kFeatureCount
};

// -------------------------------------------------------------------------

struct Lv2PluginOptions {
    enum OptIndex {
        SampleRate,
        TransientWinId,
        WindowTitle,
        Null,
        Count
    };

    double sampleRate;
    int64_t transientWinId;
    const char* windowTitle;
    LV2_Options_Option opts[Count];

    Lv2PluginOptions() noexcept
        : sampleRate(gInitialSampleRate),
          transientWinId(0),
          windowTitle(nullptr)
    {
        LV2_Options_Option& optSampleRate(opts[SampleRate]);
        optSampleRate.context = LV2_OPTIONS_INSTANCE;
        optSampleRate.subject = 0;
        optSampleRate.key     = CARLA_URI_MAP_ID_PARAM_SAMPLE_RATE;
        optSampleRate.size    = sizeof(double);
        optSampleRate.type    = CARLA_URI_MAP_ID_ATOM_DOUBLE;
        optSampleRate.value   = &sampleRate;

        LV2_Options_Option& optTransientWinId(opts[TransientWinId]);
        optTransientWinId.context = LV2_OPTIONS_INSTANCE;
        optTransientWinId.subject = 0;
        optTransientWinId.key     = CARLA_URI_MAP_ID_CARLA_TRANSIENT_WIN_ID;
        optTransientWinId.size    = sizeof(int64_t);
        optTransientWinId.type    = CARLA_URI_MAP_ID_ATOM_LONG;
        optTransientWinId.value   = &transientWinId;

        LV2_Options_Option& optWindowTitle(opts[WindowTitle]);
        optWindowTitle.context = LV2_OPTIONS_INSTANCE;
        optWindowTitle.subject = 0;
        optWindowTitle.key     = CARLA_URI_MAP_ID_UI_WINDOW_TITLE;
        optWindowTitle.size    = 0;
        optWindowTitle.type    = CARLA_URI_MAP_ID_ATOM_STRING;
        optWindowTitle.value   = nullptr;

        LV2_Options_Option& optNull(opts[Null]);
        optNull.context = LV2_OPTIONS_INSTANCE;
        optNull.subject = 0;
        optNull.key     = CARLA_URI_MAP_ID_NULL;
        optNull.size    = 0;
        optNull.type    = CARLA_URI_MAP_ID_NULL;
        optNull.value   = nullptr;
    }
};

// -------------------------------------------------------------------------

class CarlaLv2Client : public CarlaBridgeUI
{
public:
    CarlaLv2Client()
        : CarlaBridgeUI(),
          fHandle(nullptr),
          fWidget(nullptr),
          fDescriptor(nullptr),
          fRdfDescriptor(nullptr),
          fRdfUiDescriptor(nullptr),
          fLv2Options(),
          fUiOptions(),
          fCustomURIDs(CARLA_URI_MAP_ID_COUNT, std::string("urn:null")),
          fExt()
    {
        CARLA_SAFE_ASSERT(fCustomURIDs.size() == CARLA_URI_MAP_ID_COUNT);

        carla_zeroPointers(fFeatures, kFeatureCount+1);

        // ---------------------------------------------------------------
        // initialize features (part 1)

        LV2_Log_Log* const logFt = new LV2_Log_Log;
        logFt->handle            = this;
        logFt->printf            = carla_lv2_log_printf;
        logFt->vprintf           = carla_lv2_log_vprintf;

        LV2_State_Make_Path* const stateMakePathFt = new LV2_State_Make_Path;
        stateMakePathFt->handle                    = this;
        stateMakePathFt->path                      = carla_lv2_state_make_path;

        LV2_State_Map_Path* const stateMapPathFt = new LV2_State_Map_Path;
        stateMapPathFt->handle                   = this;
        stateMapPathFt->abstract_path            = carla_lv2_state_map_abstract_path;
        stateMapPathFt->absolute_path            = carla_lv2_state_map_absolute_path;

        LV2_Programs_Host* const programsFt = new LV2_Programs_Host;
        programsFt->handle                  = this;
        programsFt->program_changed         = carla_lv2_program_changed;

        LV2_URI_Map_Feature* const uriMapFt = new LV2_URI_Map_Feature;
        uriMapFt->callback_data             = this;
        uriMapFt->uri_to_id                 = carla_lv2_uri_to_id;

        LV2_URID_Map* const uridMapFt = new LV2_URID_Map;
        uridMapFt->handle             = this;
        uridMapFt->map                = carla_lv2_urid_map;

        LV2_URID_Unmap* const uridUnmapFt = new LV2_URID_Unmap;
        uridUnmapFt->handle               = this;
        uridUnmapFt->unmap                = carla_lv2_urid_unmap;

        LV2UI_Port_Map* const uiPortMapFt = new LV2UI_Port_Map;
        uiPortMapFt->handle               = this;
        uiPortMapFt->port_index           = carla_lv2_ui_port_map;

        LV2UI_Resize* const uiResizeFt    = new LV2UI_Resize;
        uiResizeFt->handle                = this;
        uiResizeFt->ui_resize             = carla_lv2_ui_resize;

        // ---------------------------------------------------------------
        // initialize features (part 2)

        for (uint32_t i=0; i < kFeatureCount; ++i)
            fFeatures[i] = new LV2_Feature;

        fFeatures[kFeatureIdLogs]->URI       = LV2_LOG__log;
        fFeatures[kFeatureIdLogs]->data      = logFt;

        fFeatures[kFeatureIdOptions]->URI    = LV2_OPTIONS__options;
        fFeatures[kFeatureIdOptions]->data   = fLv2Options.opts;

        fFeatures[kFeatureIdPrograms]->URI   = LV2_PROGRAMS__Host;
        fFeatures[kFeatureIdPrograms]->data  = programsFt;

        fFeatures[kFeatureIdStateMakePath]->URI  = LV2_STATE__makePath;
        fFeatures[kFeatureIdStateMakePath]->data = stateMakePathFt;

        fFeatures[kFeatureIdStateMapPath]->URI   = LV2_STATE__mapPath;
        fFeatures[kFeatureIdStateMapPath]->data  = stateMapPathFt;

        fFeatures[kFeatureIdUriMap]->URI     = LV2_URI_MAP_URI;
        fFeatures[kFeatureIdUriMap]->data    = uriMapFt;

        fFeatures[kFeatureIdUridMap]->URI    = LV2_URID__map;
        fFeatures[kFeatureIdUridMap]->data   = uridMapFt;

        fFeatures[kFeatureIdUridUnmap]->URI  = LV2_URID__unmap;
        fFeatures[kFeatureIdUridUnmap]->data = uridUnmapFt;

        fFeatures[kFeatureIdUiIdleInterface]->URI  = LV2_UI__idleInterface;
        fFeatures[kFeatureIdUiIdleInterface]->data = nullptr;

        fFeatures[kFeatureIdUiFixedSize]->URI      = LV2_UI__fixedSize;
        fFeatures[kFeatureIdUiFixedSize]->data     = nullptr;

        fFeatures[kFeatureIdUiMakeResident]->URI   = LV2_UI__makeResident;
        fFeatures[kFeatureIdUiMakeResident]->data  = nullptr;

        fFeatures[kFeatureIdUiMakeResident2]->URI  = LV2_UI__makeSONameResident;
        fFeatures[kFeatureIdUiMakeResident2]->data = nullptr;

        fFeatures[kFeatureIdUiNoUserResize]->URI   = LV2_UI__noUserResize;
        fFeatures[kFeatureIdUiNoUserResize]->data  = nullptr;

        fFeatures[kFeatureIdUiParent]->URI         = LV2_UI__parent;
        fFeatures[kFeatureIdUiParent]->data        = nullptr;

        fFeatures[kFeatureIdUiPortMap]->URI        = LV2_UI__portMap;
        fFeatures[kFeatureIdUiPortMap]->data       = uiPortMapFt;

        fFeatures[kFeatureIdUiPortSubscribe]->URI  = LV2_UI__portSubscribe;
        fFeatures[kFeatureIdUiPortSubscribe]->data = nullptr;

        fFeatures[kFeatureIdUiResize]->URI  = LV2_UI__resize;
        fFeatures[kFeatureIdUiResize]->data = uiResizeFt;

        fFeatures[kFeatureIdUiTouch]->URI   = LV2_UI__touch;
        fFeatures[kFeatureIdUiTouch]->data  = nullptr;
    }

    ~CarlaLv2Client() override
    {
        if (fHandle != nullptr && fDescriptor != nullptr && fDescriptor->cleanup != nullptr)
        {
            fDescriptor->cleanup(fHandle);
            fHandle = nullptr;
        }

        if (fRdfDescriptor != nullptr)
        {
            delete fRdfDescriptor;
            fRdfDescriptor = nullptr;
        }

        fRdfUiDescriptor = nullptr;

        delete (LV2_Log_Log*)fFeatures[kFeatureIdLogs]->data;
        delete (LV2_State_Make_Path*)fFeatures[kFeatureIdStateMakePath]->data;
        delete (LV2_State_Map_Path*)fFeatures[kFeatureIdStateMapPath]->data;
        delete (LV2_Programs_Host*)fFeatures[kFeatureIdPrograms]->data;
        delete (LV2_URI_Map_Feature*)fFeatures[kFeatureIdUriMap]->data;
        delete (LV2_URID_Map*)fFeatures[kFeatureIdUridMap]->data;
        delete (LV2_URID_Unmap*)fFeatures[kFeatureIdUridUnmap]->data;
        delete (LV2UI_Port_Map*)fFeatures[kFeatureIdUiPortMap]->data;
        delete (LV2UI_Resize*)fFeatures[kFeatureIdUiResize]->data;

        for (uint32_t i=0; i < kFeatureCount; ++i)
        {
            if (fFeatures[i] != nullptr)
            {
                delete fFeatures[i];
                fFeatures[i] = nullptr;
            }
        }
    }

    // ---------------------------------------------------------------------
    // UI initialization

    bool init(const int argc, const char* argv[]) override
    {
        const char* pluginURI = argv[1];
        const char* uiURI     = argv[2];

        // -----------------------------------------------------------------
        // load plugin

        Lv2WorldClass& lv2World(Lv2WorldClass::getInstance());
        lv2World.initIfNeeded(std::getenv("LV2_PATH"));

        //Lilv::Node bundleNode(lv2World.new_file_uri(nullptr, uiBundle));
        //CARLA_SAFE_ASSERT_RETURN(bundleNode.is_uri(), false);

        //CarlaString sBundle(bundleNode.as_uri());

        //if (! sBundle.endsWith("/"))
        //    sBundle += "/";

        //lv2World.load_bundle(sBundle);

        // -----------------------------------------------------------------
        // get plugin from lv2_rdf (lilv)

        fRdfDescriptor = lv2_rdf_new(pluginURI, true);
        CARLA_SAFE_ASSERT_RETURN(fRdfDescriptor != nullptr, false);

        // -----------------------------------------------------------------
        // find requested UI

        for (uint32_t i=0; i < fRdfDescriptor->UICount; ++i)
        {
            if (std::strcmp(fRdfDescriptor->UIs[i].URI, uiURI) == 0)
            {
                fRdfUiDescriptor = &fRdfDescriptor->UIs[i];
                break;
            }
        }

        CARLA_SAFE_ASSERT_RETURN(fRdfUiDescriptor != nullptr, false);

        // -----------------------------------------------------------
        // check if not resizable

#if defined(BRIDGE_COCOA) || defined(BRIDGE_HWND) || defined(BRIDGE_X11)
        // embed UIs can only be resizable if they provide resize extension
        fUiOptions.isResizable = false;
        // TODO: put this trick into main carla

        for (uint32_t i=0; i < fRdfUiDescriptor->ExtensionCount; ++i)
        {
            carla_stdout("Test UI extension %s", fRdfUiDescriptor->Extensions[i]);

            if (std::strcmp(fRdfUiDescriptor->Extensions[i], LV2_UI__resize) == 0)
            {
                fUiOptions.isResizable = true;
                break;
            }
        }
#endif

        for (uint32_t i=0; i < fRdfUiDescriptor->FeatureCount; ++i)
        {
            carla_stdout("Test UI feature %s", fRdfUiDescriptor->Features[i].URI);

            if (std::strcmp(fRdfUiDescriptor->Features[i].URI, LV2_UI__fixedSize   ) == 0 ||
                std::strcmp(fRdfUiDescriptor->Features[i].URI, LV2_UI__noUserResize) == 0)
            {
                fUiOptions.isResizable = false;
                break;
            }
        }

        carla_stdout("Is resizable => %s", bool2str(fUiOptions.isResizable));

        // -----------------------------------------------------------------
        // init UI

        if (! CarlaBridgeUI::init(argc, argv))
            return false;

        // -----------------------------------------------------------------
        // open DLL

        if (! libOpen(fRdfUiDescriptor->Binary))
        {
            carla_stderr("Failed to load UI binary, error was:\n%s", libError());
            return false;
        }

        // -----------------------------------------------------------------
        // get DLL main entry

        const LV2UI_DescriptorFunction ui_descFn = (LV2UI_DescriptorFunction)libSymbol("lv2ui_descriptor");

        if (ui_descFn == nullptr)
            return false;

        // -----------------------------------------------------------
        // get descriptor that matches URI

        uint32_t i = 0;
        while ((fDescriptor = ui_descFn(i++)))
        {
            if (std::strcmp(fDescriptor->URI, uiURI) == 0)
                break;
        }

        if (fDescriptor == nullptr)
        {
            carla_stderr("Failed to find UI descriptor");
            return false;
        }

        // -----------------------------------------------------------
        // initialize UI

#if defined(BRIDGE_COCOA) || defined(BRIDGE_HWND) || defined(BRIDGE_X11)
        fFeatures[kFeatureIdUiParent]->data = fToolkit->getContainerId();
#endif

        fHandle = fDescriptor->instantiate(fDescriptor, fRdfDescriptor->URI, fRdfUiDescriptor->Bundle, carla_lv2_ui_write_function, this, &fWidget, fFeatures);
        CARLA_SAFE_ASSERT_RETURN(fHandle != nullptr, false);

        // -----------------------------------------------------------
        // check for known extensions

        if (fDescriptor->extension_data != nullptr)
        {
            fExt.options  = (const LV2_Options_Interface*)fDescriptor->extension_data(LV2_OPTIONS__interface);
            fExt.programs = (const LV2_Programs_UI_Interface*)fDescriptor->extension_data(LV2_PROGRAMS__UIInterface);
            fExt.idle     = (const LV2UI_Idle_Interface*)fDescriptor->extension_data(LV2_UI__idleInterface);
            fExt.resize   = (const LV2UI_Resize*)fDescriptor->extension_data(LV2_UI__resize);

            // check if invalid
            if (fExt.programs != nullptr && fExt.programs->select_program == nullptr)
                fExt.programs = nullptr;
            if (fExt.idle != nullptr && fExt.idle->idle == nullptr)
                fExt.idle = nullptr;
            if (fExt.resize != nullptr && fExt.resize->ui_resize == nullptr)
                fExt.resize = nullptr;
        }

        return true;
    }

    void idleUI() override
    {
#if defined(BRIDGE_COCOA) || defined(BRIDGE_HWND) || defined(BRIDGE_X11)
        if (fHandle != nullptr && fExt.idle != nullptr)
            fExt.idle->idle(fHandle);
#endif
    }

    // ---------------------------------------------------------------------
    // UI management

    void* getWidget() const noexcept override
    {
        return fWidget;
    }

    const Options& getOptions() const noexcept override
    {
        return fUiOptions;
    }

    // ---------------------------------------------------------------------
    // DSP Callbacks

    void dspParameterChanged(const uint32_t index, const float value) override
    {
        CARLA_SAFE_ASSERT_RETURN(fHandle != nullptr,)
        CARLA_SAFE_ASSERT_RETURN(fDescriptor != nullptr,);

        if (fDescriptor->port_event == nullptr)
            return;

        fDescriptor->port_event(fHandle, index, sizeof(float), CARLA_URI_MAP_ID_NULL, &value);
    }

    void dspProgramChanged(const uint32_t) override
    {
    }

    void dspMidiProgramChanged(const uint32_t bank, const uint32_t program) override
    {
        CARLA_SAFE_ASSERT_RETURN(fHandle != nullptr,)

        if (fExt.programs == nullptr)
            return;

        fExt.programs->select_program(fHandle, bank, program);
    }

    void dspStateChanged(const char* const, const char* const) override
    {
    }

    void dspNoteReceived(const bool onOff, const uint8_t channel, const uint8_t note, const uint8_t velocity) override
    {
        CARLA_SAFE_ASSERT_RETURN(fHandle != nullptr,)
        CARLA_SAFE_ASSERT_RETURN(fDescriptor != nullptr,);

        if (fDescriptor->port_event == nullptr)
            return;

        LV2_Atom_MidiEvent midiEv;
        midiEv.atom.type = CARLA_URI_MAP_ID_MIDI_EVENT;
        midiEv.atom.size = 3;
        midiEv.data[0] = uint8_t((onOff ? MIDI_STATUS_NOTE_ON : MIDI_STATUS_NOTE_OFF) | (channel & MIDI_CHANNEL_BIT));
        midiEv.data[1] = note;
        midiEv.data[2] = velocity;

        fDescriptor->port_event(fHandle, /* TODO */ 0, lv2_atom_total_size(midiEv), CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT, &midiEv);
    }

    void dspAtomReceived(const uint32_t portIndex, const LV2_Atom* const atom) override
    {
        CARLA_SAFE_ASSERT_RETURN(fHandle != nullptr,);
        CARLA_SAFE_ASSERT_RETURN(fDescriptor != nullptr,);
        CARLA_SAFE_ASSERT_RETURN(atom != nullptr,);

        if (fDescriptor->port_event == nullptr)
            return;

        fDescriptor->port_event(fHandle, portIndex, lv2_atom_total_size(atom), CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT, atom);
    }

    void dspURIDReceived(const LV2_URID urid, const char* const uri) override
    {
        CARLA_SAFE_ASSERT_RETURN(urid == fCustomURIDs.size(),);
        CARLA_SAFE_ASSERT_RETURN(uri != nullptr && uri[0] != '\0',);

        fCustomURIDs.push_back(uri);
    }

    void uiOptionsChanged(const double sampleRate, const bool useTheme, const bool useThemeColors, const char* const windowTitle, uintptr_t transientWindowId) override
    {
        carla_debug("CarlaLv2Client::uiOptionsChanged(%g, %s, %s, \"%s\", " P_UINTPTR ")", sampleRate, bool2str(useTheme), bool2str(useThemeColors), windowTitle, transientWindowId);

        delete[] fLv2Options.windowTitle;

        fLv2Options.sampleRate     = sampleRate;
        fLv2Options.transientWinId = static_cast<int64_t>(transientWindowId);
        fLv2Options.windowTitle    = carla_strdup_safe(windowTitle);

        fUiOptions.useTheme          = useTheme;
        fUiOptions.useThemeColors    = useThemeColors;
        fUiOptions.windowTitle       = windowTitle;
        fUiOptions.transientWindowId = transientWindowId;
    }

    void uiResized(const uint width, const uint height) override
    {
        if (fHandle != nullptr && fExt.resize != nullptr)
            fExt.resize->ui_resize(fHandle, static_cast<int>(width), static_cast<int>(height));
    }

    // ---------------------------------------------------------------------

    LV2_URID getCustomURID(const char* const uri)
    {
        CARLA_SAFE_ASSERT_RETURN(uri != nullptr && uri[0] != '\0', CARLA_URI_MAP_ID_NULL);
        carla_debug("CarlaLv2Client::getCustomURID(\"%s\")", uri);

        const std::string    s_uri(uri);
        const std::ptrdiff_t s_pos(std::find(fCustomURIDs.begin(), fCustomURIDs.end(), s_uri) - fCustomURIDs.begin());

        if (s_pos <= 0 || s_pos >= INT32_MAX)
            return CARLA_URI_MAP_ID_NULL;

        const LV2_URID urid     = static_cast<LV2_URID>(s_pos);
        const LV2_URID uriCount = static_cast<LV2_URID>(fCustomURIDs.size());

        if (urid < uriCount)
            return urid;

        CARLA_SAFE_ASSERT(urid == uriCount);

        fCustomURIDs.push_back(uri);

        if (isPipeRunning())
            writeLv2UridMessage(urid, uri);

        return urid;
    }

    const char* getCustomURIDString(const LV2_URID urid) const noexcept
    {
        static const char* const sFallback = "urn:null";
        CARLA_SAFE_ASSERT_RETURN(urid != CARLA_URI_MAP_ID_NULL, sFallback);
        CARLA_SAFE_ASSERT_RETURN(urid < fCustomURIDs.size(), sFallback);
        carla_debug("CarlaLv2Client::getCustomURIDString(%i)", urid);

        return fCustomURIDs[urid].c_str();
    }

    // ---------------------------------------------------------------------

    void handleProgramChanged(const int32_t /*index*/)
    {
        if (isPipeRunning())
            writeConfigureMessage("reloadprograms", "");
    }

    uint32_t handleUiPortMap(const char* const symbol)
    {
        CARLA_SAFE_ASSERT_RETURN(symbol != nullptr && symbol[0] != '\0', LV2UI_INVALID_PORT_INDEX);
        carla_debug("CarlaLv2Client::handleUiPortMap(\"%s\")", symbol);

        for (uint32_t i=0; i < fRdfDescriptor->PortCount; ++i)
        {
            if (std::strcmp(fRdfDescriptor->Ports[i].Symbol, symbol) == 0)
                return i;
        }

        return LV2UI_INVALID_PORT_INDEX;
    }

    int handleUiResize(const int width, const int height)
    {
        CARLA_SAFE_ASSERT_RETURN(fToolkit != nullptr, 1);
        CARLA_SAFE_ASSERT_RETURN(width > 0, 1);
        CARLA_SAFE_ASSERT_RETURN(height > 0, 1);
        carla_debug("CarlaLv2Client::handleUiResize(%i, %i)", width, height);

        fToolkit->setSize(static_cast<uint>(width), static_cast<uint>(height));

        return 0;
    }

    void handleUiWrite(uint32_t rindex, uint32_t bufferSize, uint32_t format, const void* buffer)
    {
        CARLA_SAFE_ASSERT_RETURN(buffer != nullptr,);
        CARLA_SAFE_ASSERT_RETURN(bufferSize > 0,);
        carla_debug("CarlaLv2Client::handleUiWrite(%i, %i, %i, %p)", rindex, bufferSize, format, buffer);

        switch (format)
        {
        case CARLA_URI_MAP_ID_NULL: {
            CARLA_SAFE_ASSERT_RETURN(bufferSize == sizeof(float),);

            const float value(*(const float*)buffer);

            if (isPipeRunning())
                writeControlMessage(rindex, value);

        } break;

        case CARLA_URI_MAP_ID_ATOM_TRANSFER_ATOM:
        case CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT: {
            CARLA_SAFE_ASSERT_RETURN(bufferSize >= sizeof(LV2_Atom),);

            const LV2_Atom* const atom((const LV2_Atom*)buffer);

            // plugins sometimes fail on this, not good...
            CARLA_SAFE_ASSERT_INT2(bufferSize == lv2_atom_total_size(atom), bufferSize, atom->size);

            if (isPipeRunning())
                writeLv2AtomMessage(rindex, atom);
        } break;

        default:
            carla_stdout("CarlaLv2Client::handleUiWrite(%i, %i, %i:\"%s\", %p) - unknown format", rindex, bufferSize, format, carla_lv2_urid_unmap(this, format), buffer);
            break;
        }
    }

    // ---------------------------------------------------------------------

private:
    LV2UI_Handle fHandle;
    LV2UI_Widget fWidget;
    LV2_Feature* fFeatures[kFeatureCount+1];

    const LV2UI_Descriptor*   fDescriptor;
    const LV2_RDF_Descriptor* fRdfDescriptor;
    const LV2_RDF_UI*         fRdfUiDescriptor;
    Lv2PluginOptions          fLv2Options;

    Options fUiOptions;
    std::vector<std::string> fCustomURIDs;

    struct Extensions {
        const LV2_Options_Interface* options;
        const LV2_Programs_UI_Interface* programs;
        const LV2UI_Idle_Interface* idle;
        const LV2UI_Resize* resize;

        Extensions()
            : options(nullptr),
              programs(nullptr),
              idle(nullptr),
              resize(nullptr) {}
    } fExt;

    // -------------------------------------------------------------------
    // Logs Feature

    static int carla_lv2_log_printf(LV2_Log_Handle handle, LV2_URID type, const char* fmt, ...)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, 0);
        CARLA_SAFE_ASSERT_RETURN(type != CARLA_URI_MAP_ID_NULL, 0);
        CARLA_SAFE_ASSERT_RETURN(fmt != nullptr, 0);

#ifndef DEBUG
        if (type == CARLA_URI_MAP_ID_LOG_TRACE)
            return 0;
#endif

        va_list args;
        va_start(args, fmt);
        const int ret(carla_lv2_log_vprintf(handle, type, fmt, args));
        va_end(args);

        return ret;
    }

    static int carla_lv2_log_vprintf(LV2_Log_Handle handle, LV2_URID type, const char* fmt, va_list ap)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, 0);
        CARLA_SAFE_ASSERT_RETURN(type != CARLA_URI_MAP_ID_NULL, 0);
        CARLA_SAFE_ASSERT_RETURN(fmt != nullptr, 0);

#ifndef DEBUG
        if (type == CARLA_URI_MAP_ID_LOG_TRACE)
            return 0;
#endif

        int ret = 0;

        switch (type)
        {
        case CARLA_URI_MAP_ID_LOG_ERROR:
            std::fprintf(stderr, "\x1b[31m");
            ret = std::vfprintf(stderr, fmt, ap);
            std::fprintf(stderr, "\x1b[0m");
            break;

        case CARLA_URI_MAP_ID_LOG_NOTE:
            ret = std::vfprintf(stdout, fmt, ap);
            break;

        case CARLA_URI_MAP_ID_LOG_TRACE:
#ifdef DEBUG
            std::fprintf(stdout, "\x1b[30;1m");
            ret = std::vfprintf(stdout, fmt, ap);
            std::fprintf(stdout, "\x1b[0m");
#endif
            break;

        case CARLA_URI_MAP_ID_LOG_WARNING:
            ret = std::vfprintf(stderr, fmt, ap);
            break;

        default:
            break;
        }

        return ret;
    }

    // -------------------------------------------------------------------
    // Programs Feature

    static void carla_lv2_program_changed(LV2_Programs_Handle handle, int32_t index)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr,);
        carla_debug("carla_lv2_program_changed(%p, %i)", handle, index);

        ((CarlaLv2Client*)handle)->handleProgramChanged(index);
    }

    // -------------------------------------------------------------------
    // State Feature

    static char* carla_lv2_state_make_path(LV2_State_Make_Path_Handle handle, const char* path)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, nullptr);
        CARLA_SAFE_ASSERT_RETURN(path != nullptr && path[0] != '\0', nullptr);
        carla_debug("carla_lv2_state_make_path(%p, \"%s\")", handle, path);

        File file;

        if (File::isAbsolutePath(path))
            file = File(path);
        else
            file = File::getCurrentWorkingDirectory().getChildFile(path);

        file.getParentDirectory().createDirectory();

        return strdup(file.getFullPathName().toRawUTF8());
    }

    static char* carla_lv2_state_map_abstract_path(LV2_State_Map_Path_Handle handle, const char* absolute_path)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, strdup(""));
        CARLA_SAFE_ASSERT_RETURN(absolute_path != nullptr && absolute_path[0] != '\0', strdup(""));
        carla_debug("carla_lv2_state_map_abstract_path(%p, \"%s\")", handle, absolute_path);

        // may already be an abstract path
        if (! File::isAbsolutePath(absolute_path))
            return strdup(absolute_path);

        return strdup(File(absolute_path).getRelativePathFrom(File::getCurrentWorkingDirectory()).toRawUTF8());
    }

    static char* carla_lv2_state_map_absolute_path(LV2_State_Map_Path_Handle handle, const char* abstract_path)
    {
        const char* const cwd(File::getCurrentWorkingDirectory().getFullPathName().toRawUTF8());
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, strdup(cwd));
        CARLA_SAFE_ASSERT_RETURN(abstract_path != nullptr && abstract_path[0] != '\0', strdup(cwd));
        carla_debug("carla_lv2_state_map_absolute_path(%p, \"%s\")", handle, abstract_path);

        // may already be an absolute path
        if (File::isAbsolutePath(abstract_path))
            return strdup(abstract_path);

        return strdup(File::getCurrentWorkingDirectory().getChildFile(abstract_path).getFullPathName().toRawUTF8());
    }

    // -------------------------------------------------------------------
    // URI-Map Feature

    static uint32_t carla_lv2_uri_to_id(LV2_URI_Map_Callback_Data data, const char* map, const char* uri)
    {
        carla_debug("carla_lv2_uri_to_id(%p, \"%s\", \"%s\")", data, map, uri);
        return carla_lv2_urid_map((LV2_URID_Map_Handle*)data, uri);

        // unused
        (void)map;
    }

    // -------------------------------------------------------------------
    // URID Feature

    static LV2_URID carla_lv2_urid_map(LV2_URID_Map_Handle handle, const char* uri)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, CARLA_URI_MAP_ID_NULL);
        CARLA_SAFE_ASSERT_RETURN(uri != nullptr && uri[0] != '\0', CARLA_URI_MAP_ID_NULL);
        carla_debug("carla_lv2_urid_map(%p, \"%s\")", handle, uri);

        // Atom types
        if (std::strcmp(uri, LV2_ATOM__Blank) == 0)
            return CARLA_URI_MAP_ID_ATOM_BLANK;
        if (std::strcmp(uri, LV2_ATOM__Bool) == 0)
            return CARLA_URI_MAP_ID_ATOM_BOOL;
        if (std::strcmp(uri, LV2_ATOM__Chunk) == 0)
            return CARLA_URI_MAP_ID_ATOM_CHUNK;
        if (std::strcmp(uri, LV2_ATOM__Double) == 0)
            return CARLA_URI_MAP_ID_ATOM_DOUBLE;
        if (std::strcmp(uri, LV2_ATOM__Event) == 0)
            return CARLA_URI_MAP_ID_ATOM_EVENT;
        if (std::strcmp(uri, LV2_ATOM__Float) == 0)
            return CARLA_URI_MAP_ID_ATOM_FLOAT;
        if (std::strcmp(uri, LV2_ATOM__Int) == 0)
            return CARLA_URI_MAP_ID_ATOM_INT;
        if (std::strcmp(uri, LV2_ATOM__Literal) == 0)
            return CARLA_URI_MAP_ID_ATOM_LITERAL;
        if (std::strcmp(uri, LV2_ATOM__Long) == 0)
            return CARLA_URI_MAP_ID_ATOM_LONG;
        if (std::strcmp(uri, LV2_ATOM__Number) == 0)
            return CARLA_URI_MAP_ID_ATOM_NUMBER;
        if (std::strcmp(uri, LV2_ATOM__Object) == 0)
            return CARLA_URI_MAP_ID_ATOM_OBJECT;
        if (std::strcmp(uri, LV2_ATOM__Path) == 0)
            return CARLA_URI_MAP_ID_ATOM_PATH;
        if (std::strcmp(uri, LV2_ATOM__Property) == 0)
            return CARLA_URI_MAP_ID_ATOM_PROPERTY;
        if (std::strcmp(uri, LV2_ATOM__Resource) == 0)
            return CARLA_URI_MAP_ID_ATOM_RESOURCE;
        if (std::strcmp(uri, LV2_ATOM__Sequence) == 0)
            return CARLA_URI_MAP_ID_ATOM_SEQUENCE;
        if (std::strcmp(uri, LV2_ATOM__Sound) == 0)
            return CARLA_URI_MAP_ID_ATOM_SOUND;
        if (std::strcmp(uri, LV2_ATOM__String) == 0)
            return CARLA_URI_MAP_ID_ATOM_STRING;
        if (std::strcmp(uri, LV2_ATOM__Tuple) == 0)
            return CARLA_URI_MAP_ID_ATOM_TUPLE;
        if (std::strcmp(uri, LV2_ATOM__URI) == 0)
            return CARLA_URI_MAP_ID_ATOM_URI;
        if (std::strcmp(uri, LV2_ATOM__URID) == 0)
            return CARLA_URI_MAP_ID_ATOM_URID;
        if (std::strcmp(uri, LV2_ATOM__Vector) == 0)
            return CARLA_URI_MAP_ID_ATOM_VECTOR;
        if (std::strcmp(uri, LV2_ATOM__atomTransfer) == 0)
            return CARLA_URI_MAP_ID_ATOM_TRANSFER_ATOM;
        if (std::strcmp(uri, LV2_ATOM__eventTransfer) == 0)
            return CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT;

        // BufSize types
        if (std::strcmp(uri, LV2_BUF_SIZE__maxBlockLength) == 0)
            return CARLA_URI_MAP_ID_BUF_MAX_LENGTH;
        if (std::strcmp(uri, LV2_BUF_SIZE__minBlockLength) == 0)
            return CARLA_URI_MAP_ID_BUF_MIN_LENGTH;
        if (std::strcmp(uri, LV2_BUF_SIZE__nominalBlockLength) == 0)
            return CARLA_URI_MAP_ID_BUF_NOMINAL_LENGTH;
        if (std::strcmp(uri, LV2_BUF_SIZE__sequenceSize) == 0)
            return CARLA_URI_MAP_ID_BUF_SEQUENCE_SIZE;

        // Log types
        if (std::strcmp(uri, LV2_LOG__Error) == 0)
            return CARLA_URI_MAP_ID_LOG_ERROR;
        if (std::strcmp(uri, LV2_LOG__Note) == 0)
            return CARLA_URI_MAP_ID_LOG_NOTE;
        if (std::strcmp(uri, LV2_LOG__Trace) == 0)
            return CARLA_URI_MAP_ID_LOG_TRACE;
        if (std::strcmp(uri, LV2_LOG__Warning) == 0)
            return CARLA_URI_MAP_ID_LOG_WARNING;

        // Time types
        if (std::strcmp(uri, LV2_TIME__Position) == 0)
            return CARLA_URI_MAP_ID_TIME_POSITION;
        if (std::strcmp(uri, LV2_TIME__bar) == 0)
            return CARLA_URI_MAP_ID_TIME_BAR;
        if (std::strcmp(uri, LV2_TIME__barBeat) == 0)
            return CARLA_URI_MAP_ID_TIME_BAR_BEAT;
        if (std::strcmp(uri, LV2_TIME__beat) == 0)
            return CARLA_URI_MAP_ID_TIME_BEAT;
        if (std::strcmp(uri, LV2_TIME__beatUnit) == 0)
            return CARLA_URI_MAP_ID_TIME_BEAT_UNIT;
        if (std::strcmp(uri, LV2_TIME__beatsPerBar) == 0)
            return CARLA_URI_MAP_ID_TIME_BEATS_PER_BAR;
        if (std::strcmp(uri, LV2_TIME__beatsPerMinute) == 0)
            return CARLA_URI_MAP_ID_TIME_BEATS_PER_MINUTE;
        if (std::strcmp(uri, LV2_TIME__frame) == 0)
            return CARLA_URI_MAP_ID_TIME_FRAME;
        if (std::strcmp(uri, LV2_TIME__framesPerSecond) == 0)
            return CARLA_URI_MAP_ID_TIME_FRAMES_PER_SECOND;
        if (std::strcmp(uri, LV2_TIME__speed) == 0)
            return CARLA_URI_MAP_ID_TIME_SPEED;
        if (std::strcmp(uri, LV2_KXSTUDIO_PROPERTIES__TimePositionTicksPerBeat) == 0)
            return CARLA_URI_MAP_ID_TIME_TICKS_PER_BEAT;

        // Others
        if (std::strcmp(uri, LV2_MIDI__MidiEvent) == 0)
            return CARLA_URI_MAP_ID_MIDI_EVENT;
        if (std::strcmp(uri, LV2_PARAMETERS__sampleRate) == 0)
            return CARLA_URI_MAP_ID_PARAM_SAMPLE_RATE;
        if (std::strcmp(uri, LV2_UI__windowTitle) == 0)
            return CARLA_URI_MAP_ID_UI_WINDOW_TITLE;

        // Custom
        if (std::strcmp(uri, LV2_KXSTUDIO_PROPERTIES__TransientWindowId) == 0)
            return CARLA_URI_MAP_ID_CARLA_TRANSIENT_WIN_ID;
        if (std::strcmp(uri, URI_CARLA_ATOM_WORKER) == 0)
            return CARLA_URI_MAP_ID_CARLA_ATOM_WORKER;

        // Custom types
        return ((CarlaLv2Client*)handle)->getCustomURID(uri);
    }

    static const char* carla_lv2_urid_unmap(LV2_URID_Map_Handle handle, LV2_URID urid)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, nullptr);
        CARLA_SAFE_ASSERT_RETURN(urid != CARLA_URI_MAP_ID_NULL, nullptr);
        carla_debug("carla_lv2_urid_unmap(%p, %i)", handle, urid);

        // Atom types
        if (urid == CARLA_URI_MAP_ID_ATOM_BLANK)
            return LV2_ATOM__Blank;
        if (urid == CARLA_URI_MAP_ID_ATOM_BOOL)
            return LV2_ATOM__Bool;
        if (urid == CARLA_URI_MAP_ID_ATOM_CHUNK)
            return LV2_ATOM__Chunk;
        if (urid == CARLA_URI_MAP_ID_ATOM_DOUBLE)
            return LV2_ATOM__Double;
        if (urid == CARLA_URI_MAP_ID_ATOM_EVENT)
            return LV2_ATOM__Event;
        if (urid == CARLA_URI_MAP_ID_ATOM_FLOAT)
            return LV2_ATOM__Float;
        if (urid == CARLA_URI_MAP_ID_ATOM_INT)
            return LV2_ATOM__Int;
        if (urid == CARLA_URI_MAP_ID_ATOM_LITERAL)
            return LV2_ATOM__Literal;
        if (urid == CARLA_URI_MAP_ID_ATOM_LONG)
            return LV2_ATOM__Long;
        if (urid == CARLA_URI_MAP_ID_ATOM_NUMBER)
            return LV2_ATOM__Number;
        if (urid == CARLA_URI_MAP_ID_ATOM_OBJECT)
            return LV2_ATOM__Object;
        if (urid == CARLA_URI_MAP_ID_ATOM_PATH)
            return LV2_ATOM__Path;
        if (urid == CARLA_URI_MAP_ID_ATOM_PROPERTY)
            return LV2_ATOM__Property;
        if (urid == CARLA_URI_MAP_ID_ATOM_RESOURCE)
            return LV2_ATOM__Resource;
        if (urid == CARLA_URI_MAP_ID_ATOM_SEQUENCE)
            return LV2_ATOM__Sequence;
        if (urid == CARLA_URI_MAP_ID_ATOM_SOUND)
            return LV2_ATOM__Sound;
        if (urid == CARLA_URI_MAP_ID_ATOM_STRING)
            return LV2_ATOM__String;
        if (urid == CARLA_URI_MAP_ID_ATOM_TUPLE)
            return LV2_ATOM__Tuple;
        if (urid == CARLA_URI_MAP_ID_ATOM_URI)
            return LV2_ATOM__URI;
        if (urid == CARLA_URI_MAP_ID_ATOM_URID)
            return LV2_ATOM__URID;
        if (urid == CARLA_URI_MAP_ID_ATOM_VECTOR)
            return LV2_ATOM__Vector;
        if (urid == CARLA_URI_MAP_ID_ATOM_TRANSFER_ATOM)
            return LV2_ATOM__atomTransfer;
        if (urid == CARLA_URI_MAP_ID_ATOM_TRANSFER_EVENT)
            return LV2_ATOM__eventTransfer;

        // BufSize types
        if (urid == CARLA_URI_MAP_ID_BUF_MAX_LENGTH)
            return LV2_BUF_SIZE__maxBlockLength;
        if (urid == CARLA_URI_MAP_ID_BUF_MIN_LENGTH)
            return LV2_BUF_SIZE__minBlockLength;
        if (urid == CARLA_URI_MAP_ID_BUF_NOMINAL_LENGTH)
            return LV2_BUF_SIZE__nominalBlockLength;
        if (urid == CARLA_URI_MAP_ID_BUF_SEQUENCE_SIZE)
            return LV2_BUF_SIZE__sequenceSize;

        // Log types
        if (urid == CARLA_URI_MAP_ID_LOG_ERROR)
            return LV2_LOG__Error;
        if (urid == CARLA_URI_MAP_ID_LOG_NOTE)
            return LV2_LOG__Note;
        if (urid == CARLA_URI_MAP_ID_LOG_TRACE)
            return LV2_LOG__Trace;
        if (urid == CARLA_URI_MAP_ID_LOG_WARNING)
            return LV2_LOG__Warning;

        // Time types
        if (urid == CARLA_URI_MAP_ID_TIME_POSITION)
            return LV2_TIME__Position;
        if (urid == CARLA_URI_MAP_ID_TIME_BAR)
            return LV2_TIME__bar;
        if (urid == CARLA_URI_MAP_ID_TIME_BAR_BEAT)
            return LV2_TIME__barBeat;
        if (urid == CARLA_URI_MAP_ID_TIME_BEAT)
            return LV2_TIME__beat;
        if (urid == CARLA_URI_MAP_ID_TIME_BEAT_UNIT)
            return LV2_TIME__beatUnit;
        if (urid == CARLA_URI_MAP_ID_TIME_BEATS_PER_BAR)
            return LV2_TIME__beatsPerBar;
        if (urid == CARLA_URI_MAP_ID_TIME_BEATS_PER_MINUTE)
            return LV2_TIME__beatsPerMinute;
        if (urid == CARLA_URI_MAP_ID_TIME_FRAME)
            return LV2_TIME__frame;
        if (urid == CARLA_URI_MAP_ID_TIME_FRAMES_PER_SECOND)
            return LV2_TIME__framesPerSecond;
        if (urid == CARLA_URI_MAP_ID_TIME_SPEED)
            return LV2_TIME__speed;
        if (urid == CARLA_URI_MAP_ID_TIME_TICKS_PER_BEAT)
            return LV2_KXSTUDIO_PROPERTIES__TimePositionTicksPerBeat;

        // Others
        if (urid == CARLA_URI_MAP_ID_MIDI_EVENT)
            return LV2_MIDI__MidiEvent;
        if (urid == CARLA_URI_MAP_ID_PARAM_SAMPLE_RATE)
            return LV2_PARAMETERS__sampleRate;
        if (urid == CARLA_URI_MAP_ID_UI_WINDOW_TITLE)
            return LV2_UI__windowTitle;

        // Custom
        if (urid == CARLA_URI_MAP_ID_CARLA_ATOM_WORKER)
            return URI_CARLA_ATOM_WORKER;
        if (urid == CARLA_URI_MAP_ID_CARLA_TRANSIENT_WIN_ID)
            return LV2_KXSTUDIO_PROPERTIES__TransientWindowId;

        // Custom types
        return ((CarlaLv2Client*)handle)->getCustomURIDString(urid);
    }

    // -------------------------------------------------------------------
    // UI Port-Map Feature

    static uint32_t carla_lv2_ui_port_map(LV2UI_Feature_Handle handle, const char* symbol)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, LV2UI_INVALID_PORT_INDEX);
        carla_debug("carla_lv2_ui_port_map(%p, \"%s\")", handle, symbol);

        return ((CarlaLv2Client*)handle)->handleUiPortMap(symbol);
    }

    // -------------------------------------------------------------------
    // UI Resize Feature

    static int carla_lv2_ui_resize(LV2UI_Feature_Handle handle, int width, int height)
    {
        CARLA_SAFE_ASSERT_RETURN(handle != nullptr, 1);
        carla_debug("carla_lv2_ui_resize(%p, %i, %i)", handle, width, height);

        return ((CarlaLv2Client*)handle)->handleUiResize(width, height);
    }

    // -------------------------------------------------------------------
    // UI Extension

    static void carla_lv2_ui_write_function(LV2UI_Controller controller, uint32_t port_index, uint32_t buffer_size, uint32_t format, const void* buffer)
    {
        CARLA_SAFE_ASSERT_RETURN(controller != nullptr,);
        carla_debug("carla_lv2_ui_write_function(%p, %i, %i, %i, %p)", controller, port_index, buffer_size, format, buffer);

        ((CarlaLv2Client*)controller)->handleUiWrite(port_index, buffer_size, format, buffer);
    }

    CARLA_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CarlaLv2Client)
};

// -----------------------------------------------------------------------

CARLA_BRIDGE_END_NAMESPACE

// -----------------------------------------------------------------------

int main(int argc, const char* argv[])
{
    CARLA_BRIDGE_USE_NAMESPACE

    if (argc < 3)
    {
        carla_stderr("usage: %s <plugin-uri> <ui-uri>", argv[0]);
        return 1;
    }

    const bool testingModeOnly = (argc != 7);

    // try to get sampleRate value
    if (const char* const sampleRateStr = std::getenv("CARLA_SAMPLE_RATE"))
        gInitialSampleRate = std::atof(sampleRateStr);

    // Init LV2 client
    CarlaLv2Client client;

    // Load UI
    int ret;

    if (client.init(argc, argv))
    {
        client.exec(testingModeOnly);
        ret = 0;
    }
    else
    {
        ret = 1;
    }

    return ret;
}

// -----------------------------------------------------------------------
