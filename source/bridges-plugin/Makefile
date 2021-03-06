#!/usr/bin/make -f
# Makefile for carla-bridges-plugin #
# --------------------------------- #
# Created by falkTX
#

CWD=..
MODULENAME=carla-bridge
include $(CWD)/Makefile.mk

# ----------------------------------------------------------------------------------------------------------------------

BINDIR    := $(CWD)/../bin

ifeq ($(DEBUG),true)
OBJDIR    := $(CWD)/../build/bridges-plugin/Debug
MODULEDIR := $(CWD)/../build/modules/Debug
else
OBJDIR    := $(CWD)/../build/bridges-plugin/Release
MODULEDIR := $(CWD)/../build/modules/Release
endif

# ----------------------------------------------------------------------------------------------------------------------

BUILD_CXX_FLAGS += -DBUILD_BRIDGE -I. -I$(CWD) -I$(CWD)/backend -I$(CWD)/includes -I$(CWD)/utils -isystem $(CWD)/modules
BUILD_CXX_FLAGS += -I$(CWD)/backend/engine -I$(CWD)/backend/plugin
BUILD_CXX_FLAGS += $(LIBLO_FLAGS)

32BIT_FLAGS += -DBUILD_BRIDGE_ALTERNATIVE_ARCH
64BIT_FLAGS += -DBUILD_BRIDGE_ALTERNATIVE_ARCH

# ----------------------------------------------------------------------------------------------------------------------

LIBS_native   = $(MODULEDIR)/jackbridge.a
LIBS_posix32  = $(MODULEDIR)/jackbridge.posix32.a
LIBS_posix64  = $(MODULEDIR)/jackbridge.posix64.a
ifeq ($(BUILDING_FOR_WINDOWS),true)
LIBS_win32    = $(MODULEDIR)/jackbridge.win32.a
LIBS_win64    = $(MODULEDIR)/jackbridge.win64.a
else
LIBS_win32    = $(MODULEDIR)/jackbridge.win32e.a
LIBS_win64    = $(MODULEDIR)/jackbridge.win64e.a
endif
LINK_FLAGS   += $(JACKBRIDGE_LIBS)

LIBS_native  += $(MODULEDIR)/lilv.a
LIBS_posix32 += $(MODULEDIR)/lilv.posix32.a
LIBS_posix64 += $(MODULEDIR)/lilv.posix64.a
LIBS_win32   += $(MODULEDIR)/lilv.win32.a
LIBS_win64   += $(MODULEDIR)/lilv.win64.a
LINK_FLAGS   += $(LILV_LIBS)

LIBS_native  += $(MODULEDIR)/rtmempool.a
LIBS_posix32 += $(MODULEDIR)/rtmempool.posix32.a
LIBS_posix64 += $(MODULEDIR)/rtmempool.posix64.a
LIBS_win32   += $(MODULEDIR)/rtmempool.win32.a
LIBS_win64   += $(MODULEDIR)/rtmempool.win64.a
LINK_FLAGS   += $(RTMEMPOOL_LIBS)

LIBS_native  += $(MODULEDIR)/water.a
LIBS_posix32 += $(MODULEDIR)/water.posix32.a
LIBS_posix64 += $(MODULEDIR)/water.posix64.a
LIBS_win32   += $(MODULEDIR)/water.win32.a
LIBS_win64   += $(MODULEDIR)/water.win64.a
LINK_FLAGS   += $(WATER_LIBS)

LINK_FLAGS += $(LIBLO_LIBS)
LINK_FLAGS += $(MAGIC_LIBS)
LINK_FLAGS += $(X11_LIBS)

# ----------------------------------------------------------------------------------------------------------------------

NATIVE_BUILD_FLAGS  = $(NATIVE_PLUGINS_FLAGS)
NATIVE_LINK_FLAGS   =

NATIVE_BUILD_FLAGS += $(FLUIDSYNTH_FLAGS)
NATIVE_LINK_FLAGS  += $(FLUIDSYNTH_LIBS)

NATIVE_BUILD_FLAGS += $(LINUXSAMPLER_FLAGS)
NATIVE_LINK_FLAGS  += $(LINUXSAMPLER_LIBS)

NATIVE_LINK_FLAGS  += $(FFMPEG_LIBS)
NATIVE_LINK_FLAGS  += $(SNDFILE_LIBS)

LIBS_native        += $(MODULEDIR)/audio_decoder.a
LIBS_native        += $(MODULEDIR)/native-plugins.base.a

ifeq ($(HAVE_DGL),true)
LIBS_native        += $(MODULEDIR)/dgl.a
endif

# ----------------------------------------------------------------------------------------------------------------------

OBJS_native = \
	$(OBJDIR)/CarlaBridgePlugin.cpp.o \
	$(OBJDIR)/CarlaEngine.cpp.o \
	$(OBJDIR)/CarlaEngineClient.cpp.o \
	$(OBJDIR)/CarlaEngineData.cpp.o \
	$(OBJDIR)/CarlaEngineInternal.cpp.o \
	$(OBJDIR)/CarlaEngineOsc.cpp.o \
	$(OBJDIR)/CarlaEngineOscSend.cpp.o \
	$(OBJDIR)/CarlaEnginePorts.cpp.o \
	$(OBJDIR)/CarlaEngineThread.cpp.o \
	$(OBJDIR)/CarlaEngineJack.cpp.o \
	$(OBJDIR)/CarlaEngineBridge.cpp.o \
	$(OBJDIR)/CarlaPlugin.cpp.o \
	$(OBJDIR)/CarlaPluginBridge.cpp.o \
	$(OBJDIR)/CarlaPluginInternal.cpp.o \
	$(OBJDIR)/CarlaPluginJack.cpp.o \
	$(OBJDIR)/CarlaPluginNative.cpp.o \
	$(OBJDIR)/CarlaPluginLADSPA.cpp.o \
	$(OBJDIR)/CarlaPluginDSSI.cpp.o \
	$(OBJDIR)/CarlaPluginLV2.cpp.o \
	$(OBJDIR)/CarlaPluginVST2.cpp.o \
	$(OBJDIR)/CarlaPluginFluidSynth.cpp.o \
	$(OBJDIR)/CarlaPluginLinuxSampler.cpp.o \
	$(OBJDIR)/CarlaStandalone.cpp.o

OBJS_lv2 = $(OBJS_native) \
	$(OBJDIR)/CarlaBridgeLV2.cpp.o

OBJS_arch = \
	$(OBJDIR)/CarlaBridgePlugin.cpp.arch.o \
	$(OBJDIR)/CarlaEngine.cpp.arch.o \
	$(OBJDIR)/CarlaEngineClient.cpp.arch.o \
	$(OBJDIR)/CarlaEngineData.cpp.arch.o \
	$(OBJDIR)/CarlaEngineInternal.cpp.arch.o \
	$(OBJDIR)/CarlaEngineOsc.cpp.arch.o \
	$(OBJDIR)/CarlaEngineOscSend.cpp.arch.o \
	$(OBJDIR)/CarlaEnginePorts.cpp.arch.o \
	$(OBJDIR)/CarlaEngineThread.cpp.arch.o \
	$(OBJDIR)/CarlaEngineJack.cpp.arch.o \
	$(OBJDIR)/CarlaEngineBridge.cpp.arch.o \
	$(OBJDIR)/CarlaPlugin.cpp.arch.o \
	$(OBJDIR)/CarlaPluginBridge.cpp.arch.o \
	$(OBJDIR)/CarlaPluginInternal.cpp.arch.o \
	$(OBJDIR)/CarlaPluginJack.cpp.arch.o \
	$(OBJDIR)/CarlaPluginLADSPA.cpp.arch.o \
	$(OBJDIR)/CarlaPluginDSSI.cpp.arch.o \
	$(OBJDIR)/CarlaPluginLV2.cpp.arch.o \
	$(OBJDIR)/CarlaPluginVST2.cpp.arch.o \
	$(OBJDIR)/CarlaStandalone.cpp.arch.o

OBJS_posix32 = $(OBJS_arch:%.arch.o=%.posix32.o)
OBJS_posix64 = $(OBJS_arch:%.arch.o=%.posix64.o)
OBJS_win32   = $(OBJS_arch:%.arch.o=%.win32.o)
OBJS_win64   = $(OBJS_arch:%.arch.o=%.win64.o)

# ----------------------------------------------------------------------------------------------------------------------

all: native

# ----------------------------------------------------------------------------------------------------------------------

clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/$(MODULENAME)-native
	rm -f $(BINDIR)/$(MODULENAME)-posix32
	rm -f $(BINDIR)/$(MODULENAME)-posix64
	rm -f $(BINDIR)/$(MODULENAME)-win*.exe

debug:
	$(MAKE) DEBUG=true

# ----------------------------------------------------------------------------------------------------------------------

native:  $(BINDIR)/$(MODULENAME)-native$(APP_EXT) $(BINDIR)/$(MODULENAME)-lv2$(LIB_EXT)
posix32: $(BINDIR)/$(MODULENAME)-posix32
posix64: $(BINDIR)/$(MODULENAME)-posix64
win32:   $(BINDIR)/$(MODULENAME)-win32.exe
win64:   $(BINDIR)/$(MODULENAME)-win64.exe

# ----------------------------------------------------------------------------------------------------------------------
# linking

$(BINDIR)/$(MODULENAME)-native$(APP_EXT): $(OBJS_native) $(LIBS_native)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-native$(APP_EXT)"
	@$(CXX) $(OBJS_native) $(LIBS_START) $(LIBS_native) $(LIBS_END) $(LINK_FLAGS) $(NATIVE_LINK_FLAGS) -o $@

$(BINDIR)/$(MODULENAME)-lv2$(LIB_EXT): $(OBJS_lv2) $(LIBS_native)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-lv2$(LIB_EXT)"
	@$(CXX) $(OBJS_lv2) $(LIBS_START) $(LIBS_native) $(LIBS_END) $(SHARED) $(LINK_FLAGS) $(NATIVE_LINK_FLAGS) -o $@

$(BINDIR)/$(MODULENAME)-posix32: $(OBJS_posix32) $(LIBS_posix32)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-posix32"
	@$(CXX) $(OBJS_posix32) $(LIBS_START) $(LIBS_posix32) $(LIBS_END) $(LINK_FLAGS) $(32BIT_FLAGS) -o $@

$(BINDIR)/$(MODULENAME)-posix64: $(OBJS_posix64) $(LIBS_posix64)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-posix64"
	@$(CXX) $(OBJS_posix64) $(LIBS_START) $(LIBS_posix64) $(LIBS_END) $(LINK_FLAGS) $(64BIT_FLAGS) -o $@

$(BINDIR)/$(MODULENAME)-win32.exe: $(OBJS_win32) $(LIBS_win32)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-win32.exe"
	@$(CXX) $(OBJS_win32) $(LIBS_START) $(LIBS_win32) $(LIBS_END) $(LINK_FLAGS) $(32BIT_FLAGS) -o $@

$(BINDIR)/$(MODULENAME)-win64.exe: $(OBJS_win64) $(LIBS_win64)
	-@mkdir -p $(BINDIR)
	@echo "Linking $(MODULENAME)-win64.exe"
	@$(CXX) $(OBJS_win64) $(LIBS_START) $(LIBS_win64) $(LIBS_END) $(LINK_FLAGS) $(64BIT_FLAGS) -o $@

# ----------------------------------------------------------------------------------------------------------------------
# native

ifeq ($(MACOS),true)
$(OBJDIR)/CarlaPluginVST2.cpp.o: $(CWD)/backend/plugin/CarlaPluginVST2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPluginVST2.cpp (bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaBridgePlugin.cpp.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -ObjC++ -c -o $@
else
$(OBJDIR)/CarlaStandalone.cpp.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -c -o $@

$(OBJDIR)/CarlaBridgePlugin.cpp.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -c -o $@
endif

$(OBJDIR)/CarlaBridgeLV2.cpp.o: CarlaBridgeLV2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $<"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -c -o $@

$(OBJDIR)/CarlaPlug%.cpp.o: $(CWD)/backend/plugin/CarlaPlug%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPlug$*.cpp (bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -c -o $@

$(OBJDIR)/CarlaEng%.cpp.o: $(CWD)/backend/engine/CarlaEng%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaEng$*.cpp (bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(NATIVE_BUILD_FLAGS) -c -o $@

# ----------------------------------------------------------------------------------------------------------------------
# posix32

ifeq ($(MACOS),true)
$(OBJDIR)/CarlaPluginVST2.cpp.posix32.o: $(CWD)/backend/plugin/CarlaPluginVST2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPluginVST2.cpp (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaBridgePlugin.cpp.posix32.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.posix32.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -ObjC++ -c -o $@
else
$(OBJDIR)/CarlaBridgePlugin.cpp.posix32.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.posix32.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@
endif

$(OBJDIR)/CarlaEng%.cpp.posix32.o: $(CWD)/backend/engine/CarlaEng%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaEng$*.cpp (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaPlug%.cpp.posix32.o: $(CWD)/backend/plugin/CarlaPlug%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPlug$*.cpp (posix32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

# ----------------------------------------------------------------------------------------------------------------------
# posix64

ifeq ($(MACOS),true)
$(OBJDIR)/CarlaPluginVST2.cpp.posix64.o: $(CWD)/backend/plugin/CarlaPluginVST2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPluginVST2.cpp (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaBridgePlugin.cpp.posix64.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -ObjC++ -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.posix64.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -ObjC++ -c -o $@
else
$(OBJDIR)/CarlaBridgePlugin.cpp.posix64.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.posix64.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@
endif

$(OBJDIR)/CarlaEng%.cpp.posix64.o: $(CWD)/backend/engine/CarlaEng%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaEng$*.cpp (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaPlug%.cpp.posix64.o: $(CWD)/backend/plugin/CarlaPlug%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPlug$*.cpp (posix64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

# ----------------------------------------------------------------------------------------------------------------------
# win32

$(OBJDIR)/CarlaBridgePlugin.cpp.win32.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (win32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaEng%.cpp.win32.o: $(CWD)/backend/engine/CarlaEng%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaEng$*.cpp (win32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaPlug%.cpp.win32.o: $(CWD)/backend/plugin/CarlaPlug%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPlug$*.cpp (win32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.win32.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (win32 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(32BIT_FLAGS) -c -o $@

# ----------------------------------------------------------------------------------------------------------------------
# win64

$(OBJDIR)/CarlaBridgePlugin.cpp.win64.o: CarlaBridgePlugin.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling $< (win64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaEng%.cpp.win64.o: $(CWD)/backend/engine/CarlaEng%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaEng$*.cpp (win64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaPlug%.cpp.win64.o: $(CWD)/backend/plugin/CarlaPlug%.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaPlug$*.cpp (win64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

$(OBJDIR)/CarlaStandalone.cpp.win64.o: $(CWD)/backend/CarlaStandalone.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CarlaStandalone.cpp (win64 bridge)"
	@$(CXX) $< $(BUILD_CXX_FLAGS) $(64BIT_FLAGS) -c -o $@

# ----------------------------------------------------------------------------------------------------------------------

-include $(OBJS_lv2:%.o=%.d)
-include $(OBJS_posix32:%.o=%.d)
-include $(OBJS_posix64:%.o=%.d)
-include $(OBJS_win32:%.o=%.d)
-include $(OBJS_win64:%.o=%.d)

# ----------------------------------------------------------------------------------------------------------------------
