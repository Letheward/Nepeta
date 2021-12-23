#include "plugin.hpp"



/* ==== Math ==== */

struct Vector3 {float     x,  y,  z;};
struct Rotor3D {float s, yz, zx, xy;};

// R* v R
Vector3 v3_rotate(Vector3 v, Rotor3D r) {

    // temp result, a vector and a trivector
    float _x   =  v.x * r.s  - v.y * r.xy + v.z * r.zx;
    float _y   =  v.y * r.s  - v.z * r.yz + v.x * r.xy;
    float _z   =  v.z * r.s  - v.x * r.zx + v.y * r.yz;
    float _xyz = -v.x * r.yz - v.y * r.zx - v.z * r.xy;

    // trivector in result will always be 0
    return (Vector3) {
        _x * r.s - _y * r.xy + _z * r.zx - _xyz * r.yz,
        _y * r.s - _z * r.yz + _x * r.xy - _xyz * r.zx,
        _z * r.s - _x * r.zx + _y * r.yz - _xyz * r.xy
    };
}

Rotor3D r3d_mul(Rotor3D a, Rotor3D b) {
    return (Rotor3D) {
        a.s * b.s  - a.yz * b.yz - a.zx * b.zx - a.xy * b.xy,
        a.s * b.yz + a.yz * b.s  - a.zx * b.xy + a.xy * b.zx,
        a.s * b.zx + a.zx * b.s  - a.xy * b.yz + a.yz * b.xy,
        a.s * b.xy + a.xy * b.s  - a.yz * b.zx + a.zx * b.yz
    };
}

Rotor3D r3d_normalize(Rotor3D r) {
    float l = sqrtf(r.s * r.s + r.yz * r.yz + r.zx * r.zx + r.xy * r.xy);
    if (l == 0) return (Rotor3D) {1, 0, 0, 0};
    return (Rotor3D) {r.s / l, r.yz / l, r.zx / l, r.xy / l};
}




/* ==== Module ==== */

struct RotorModule: Module {

    enum ParamId {
        ROTOR_S,
        ROTOR_YZ,
        ROTOR_ZX,
        ROTOR_XY,
        PARAMS_LEN
    };

    enum InputId {
        ROTOR_INPUT_S,
        ROTOR_INPUT_YZ,
        ROTOR_INPUT_ZX,
        ROTOR_INPUT_XY,
        INPUT_X,
        INPUT_Y,
        INPUT_Z,
        INPUTS_LEN
    };

    enum OutputId {
        OUTPUT_X,
        OUTPUT_Y,
        OUTPUT_Z,
        OUTPUTS_LEN
    };

    enum LightId {
        LIGHTS_LEN
    };
    
    int input_mode = 0;

    RotorModule() {

        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(ROTOR_S,  -1, 1, 0,  "s");
        configParam(ROTOR_YZ, -1, 1, 0, "yz");
        configParam(ROTOR_ZX, -1, 1, 0, "zx");
        configParam(ROTOR_XY, -1, 1, 0, "xy");

        configInput(ROTOR_INPUT_S,   "s");
        configInput(ROTOR_INPUT_YZ, "yz");
        configInput(ROTOR_INPUT_ZX, "zx");
        configInput(ROTOR_INPUT_XY, "xy");

        configInput(INPUT_X, "x");
        configInput(INPUT_Y, "y");
        configInput(INPUT_Z, "z");

        configOutput(OUTPUT_X, "x");
        configOutput(OUTPUT_Y, "y");
        configOutput(OUTPUT_Z, "z");
    }

    void process(const ProcessArgs& args) override {
        
        float offset = (float) input_mode;

        Rotor3D rotor_input = {
            inputs[ROTOR_INPUT_S ].isConnected() ? (inputs[ROTOR_INPUT_S ].getVoltage() / 5.0f - offset) : 0.0f,
            inputs[ROTOR_INPUT_YZ].isConnected() ? (inputs[ROTOR_INPUT_YZ].getVoltage() / 5.0f - offset) : 0.0f,
            inputs[ROTOR_INPUT_ZX].isConnected() ? (inputs[ROTOR_INPUT_ZX].getVoltage() / 5.0f - offset) : 0.0f,
            inputs[ROTOR_INPUT_XY].isConnected() ? (inputs[ROTOR_INPUT_XY].getVoltage() / 5.0f - offset) : 0.0f,
        };

        Rotor3D rotor_param = {
            params[ROTOR_S ].getValue(),
            params[ROTOR_YZ].getValue(),
            params[ROTOR_ZX].getValue(),
            params[ROTOR_XY].getValue(),
        };

        Vector3 vector_input = {
            inputs[INPUT_X].getVoltage(),
            inputs[INPUT_Y].getVoltage(),
            inputs[INPUT_Z].getVoltage(),
        };

        Rotor3D rotor = r3d_mul(r3d_normalize(rotor_input), r3d_normalize(rotor_param));
        Vector3 out   = v3_rotate(vector_input, rotor);

        outputs[OUTPUT_X].setVoltage(out.x);
        outputs[OUTPUT_Y].setVoltage(out.y);
        outputs[OUTPUT_Z].setVoltage(out.z);
    }
};


struct Rotor_Widget: ModuleWidget {

    Rotor_Widget(RotorModule* module) {

        // we use px here
        // 1HP        = 15px
        // Height: 3U = 380px
        // 6HP = 15px * 6 = 90px

        setModule(module);
        setPanel(createPanel(asset::plugin(plugin_instance, "res/Rotor.svg")));

        // "C++ Template is so great!!! It reduces code duplication!!!"
        #define make_param(name, widget, x, y)  addParam(createParamCentered<widget>(Vec(x, y), module, RotorModule::name))
        #define make_input(name, widget, x, y)  addInput(createInputCentered<widget>(Vec(x, y), module, RotorModule::name))
        #define make_output(name, widget, x, y) addOutput(createOutputCentered<widget>(Vec(x, y), module, RotorModule::name))

        make_input(ROTOR_INPUT_S,  Nepeta_Port,      25, 380 * 0.15);
        make_input(ROTOR_INPUT_YZ, Nepeta_Port,      65, 380 * 0.15);
        make_input(ROTOR_INPUT_ZX, Nepeta_Port,      25, 380 * 0.25);
        make_input(ROTOR_INPUT_XY, Nepeta_Port,      65, 380 * 0.25);

        make_param(ROTOR_S,        Nepeta_SmallKnob, 25, 380 * 0.35);
        make_param(ROTOR_YZ,       Nepeta_SmallKnob, 65, 380 * 0.35);
        make_param(ROTOR_ZX,       Nepeta_SmallKnob, 25, 380 * 0.45);
        make_param(ROTOR_XY,       Nepeta_SmallKnob, 65, 380 * 0.45);

        make_input(  INPUT_X,      Nepeta_Port,      25, 380 * 0.65);
        make_input(  INPUT_Y,      Nepeta_Port,      25, 380 * 0.75);
        make_input(  INPUT_Z,      Nepeta_Port,      25, 380 * 0.85);
        make_output(OUTPUT_X,      Nepeta_Port,      65, 380 * 0.65);
        make_output(OUTPUT_Y,      Nepeta_Port,      65, 380 * 0.75);
        make_output(OUTPUT_Z,      Nepeta_Port,      65, 380 * 0.85);

        #undef make_param
        #undef make_input
        #undef make_output
    }

    void appendContextMenu(Menu* menu) override {

        RotorModule* module = dynamic_cast<RotorModule*> (this->module);
        assert(module);

        menu->addChild(new MenuSeparator);
        menu->addChild(createIndexSubmenuItem(
            "Rotor Input Mode",
            {"Bipolar", "Unipolar"},
            [=]() {return module->input_mode;},
            [=](int i) {module->input_mode = i;}
        ));
    }
};

Model* model_Rotor = createModel<RotorModule, Rotor_Widget>("Rotor");

