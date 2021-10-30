#pragma once
#include <rack.hpp>


using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* pluginInstance;

// Declare each Model, defined in each module source file
// extern Model* modelMyModule;
extern Model* modelChordCV;
extern Model* modelScaleCV;
extern Model* modelRandomNoteCV;

static const int displayAlpha = 23;
NVGcolor prepareDisplay(NVGcontext *vg, Rect *box, int fontSize);
