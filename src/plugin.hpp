/* ==== Dealing With API and C++ ==== */

#pragma once
#include <rack.hpp>

using namespace rack;

// Declare the Plugin, defined in plugin.cpp
extern Plugin* plugin_instance; 

// Declare each Model, defined in each module source file
extern Model* model_BitMixer;
extern Model* model_PolyXform;
extern Model* model_Rotor;




/* ==== Common Widgets ==== */

// todo: we don't use namespace rack here, find out if this is bad

struct Nepeta_SmallKnob: app::SvgKnob {
    Nepeta_SmallKnob() {
        minAngle = -0.8 * M_PI;
        maxAngle =  0.8 * M_PI;
        setSvg(Svg::load(asset::plugin(plugin_instance, "res/Widgets/SmallKnob.svg")));
    }
};

struct Nepeta_BigKnob: app::SvgKnob {
    Nepeta_BigKnob() {
        minAngle = -0.75 * M_PI;
        maxAngle =  0.75 * M_PI;
        setSvg(Svg::load(asset::plugin(plugin_instance, "res/Widgets/BigKnob.svg")));
    }
};

struct Nepeta_Port: app::SvgPort {
    Nepeta_Port() {
        setSvg(Svg::load(asset::plugin(plugin_instance, "res/Widgets/Port.svg")));
    }
};

