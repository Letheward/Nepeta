#include "plugin.hpp"



/* ==== Utils ==== */

typedef unsigned int u32;

static inline u32 bit_shift(u32 in, int n) {
    if (n >= 0) return in >>  n;
    else        return in << -n;
}

// maybe use asm instruction?
static inline u32 bit_rotate(u32 in, int n) {
    if (n == 0) return in; // shift by 32 (see below) is UB so we need a 0 check, also maybe faster?
    if (n >  0) return (in >>  n) | (in << (32 - n));
    else        return (in << -n) | (in >> (32 + n));
}

static inline float sqrt_clamp(float in) {
    float out = abs(in);
    if (out > 1000000000) return clamp(in, -5.0, 5.0);
    while (out > 5.0) out = sqrtf(out);
    return in > 0 ? out : -out;
}




/* ==== Module ==== */

struct BitMixer: Module {

    enum ParamId {
        MIX_MODE,
        SIGN_MODE,
        GAIN1,
        GAIN2,
        BIT_SHIFT1,
        BIT_SHIFT2,
        OUT_GAIN,
        PARAMS_LEN
    };

    enum InputId {
        DATA_INPUT1,
        DATA_INPUT2,
        INPUTS_LEN
    };

    enum OutputId {
        OUTPUT,
        OUTPUTS_LEN
    };

    enum LightId {
        LIGHTS_LEN
    };

    BitMixer() {

        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(MIX_MODE,      1, 8, 1, "Main Mix Mode");
        configParam(SIGN_MODE,     0, 5, 0, "Sign Mix Mode");
        configParam(BIT_SHIFT1, -31, 31, 0, "Bit Shift 1");
        configParam(BIT_SHIFT2, -31, 31, 0, "Bit Shift 2");
        configParam(GAIN1,         0, 2, 1, "Gain 1",   " dB", -10, 40);
        configParam(GAIN2,         0, 2, 1, "Gain 2",   " dB", -10, 40);
        configParam(OUT_GAIN,      0, 2, 1, "Out Gain", " dB", -10, 40);
        configInput(DATA_INPUT1, "Data 1");
        configInput(DATA_INPUT2, "Data 2");
        configOutput(OUTPUT,     "Mix");

        paramQuantities[MIX_MODE  ]->snapEnabled = true;
        paramQuantities[SIGN_MODE ]->snapEnabled = true;
        paramQuantities[BIT_SHIFT1]->snapEnabled = true;
        paramQuantities[BIT_SHIFT2]->snapEnabled = true;
    }

    int shift_mode = 0;
    int clip_mode  = 0;

    void process(const ProcessArgs& args) override {

        /* ---- Get Data ---- */
        float out_gain  =       params[OUT_GAIN ].getValue();
        int   mix_mode  = (int) params[MIX_MODE ].getValue();
        int   sign_mode = (int) params[SIGN_MODE].getValue();

        u32 sign1, sign2;
        u32 data1, data2;
        {
            float value1 = inputs[DATA_INPUT1].getVoltage() * params[GAIN1].getValue();
            float value2 = inputs[DATA_INPUT2].getVoltage() * params[GAIN2].getValue();
            int   shift1 = (int) params[BIT_SHIFT1].getValue();
            int   shift2 = (int) params[BIT_SHIFT2].getValue();

            u32* p1 = (u32*) &value1;
            u32* p2 = (u32*) &value2;
            data1 = *p1;
            data2 = *p2;

            sign1 = data1 & 0x80000000;
            sign2 = data2 & 0x80000000;

            if (shift_mode) {
                data1 = bit_shift(data1, shift1);
                data2 = bit_shift(data2, shift2);
            } else {
                data1 = bit_rotate(data1, shift1);
                data2 = bit_rotate(data2, shift2);
            }
        }

        /* ---- Process ---- */
        u32 bits = 0;
        switch (mix_mode) {
            case 1: bits = (data1 > data2) ? data1 : data2; break;
            case 5: bits = data2 ? (data1 / data2) : data1; break;
            case 2: bits = data1 + data2; break;
            case 3: bits = data1 - data2; break;
            case 4: bits = data1 * data2; break;
            case 6: bits = data1 & data2; break;
            case 7: bits = data1 | data2; break;
            case 8: bits = data1 ^ data2; break;
            default: break;
        }

        if (sign_mode) {

            bits = bits << 1 >> 1;
            switch (sign_mode) {
                case 1: bits |= sign1;         break;
                case 2: bits |= sign2;         break;
                case 3: bits |= sign1 & sign2; break;
                case 4: bits |= sign1 | sign2; break;
                case 5: bits |= sign1 ^ sign2; break;
                default:                       break;
            }
        }

        /* ---- Post ---- */
        float out;
        {
            float* p = (float*) &bits;
            out = *p;
        }

        out *= out_gain;
        switch (clip_mode) {
            case 0:  out = clamp(out,  -5.0,  5.0); break;
            case 1:  out = tanhf(out * 0.2) * 5.0;  break; // precision lost?
            case 2:  out = sqrt_clamp(out);         break;
            case 3:  out = clamp(out,   0.0, 10.0); break;
            default: out = clamp(out, -10.0, 10.0); break;
        }
        outputs[OUTPUT].setVoltage(out);
    }
};


struct BitMixer_Widget: ModuleWidget {

    BitMixer_Widget(BitMixer* module) {

        // we use px here
        // 1HP        = 15px
        // Height: 3U = 380px
        // 6HP = 15px * 6 = 90px

        setModule(module);
        setPanel(createPanel(asset::plugin(plugin_instance, "res/BitMixer.svg")));

        // "C++ Template is so great!!! It reduces code duplication!!!"
        #define make_param(name, widget, x, y)  addParam(createParamCentered<widget>(Vec(x, y), module, BitMixer::name))
        #define make_input(name, widget, x, y)  addInput(createInputCentered<widget>(Vec(x, y), module, BitMixer::name))
        #define make_output(name, widget, x, y) addOutput(createOutputCentered<widget>(Vec(x, y), module, BitMixer::name))

        make_param(MIX_MODE,    Nepeta_BigKnob,   45, 380 * 0.2);

        make_param(SIGN_MODE,   Nepeta_SmallKnob, 25, 380 * 0.35);
        make_param(OUT_GAIN,    Nepeta_SmallKnob, 65, 380 * 0.35);
        make_param(BIT_SHIFT1,  Nepeta_SmallKnob, 25, 380 * 0.45);
        make_param(BIT_SHIFT2,  Nepeta_SmallKnob, 65, 380 * 0.45);

        make_input(DATA_INPUT1, Nepeta_Port,      25, 380 * 0.6);
        make_param(GAIN1,       Nepeta_SmallKnob, 65, 380 * 0.6);

        make_input(DATA_INPUT2, Nepeta_Port,      25, 380 * 0.7);
        make_param(GAIN2,       Nepeta_SmallKnob, 65, 380 * 0.7);

        make_output(OUTPUT,     Nepeta_Port,      45, 380 * 0.85);

        #undef make_param
        #undef make_input
        #undef make_output
    }

    void appendContextMenu(Menu* menu) override {

        BitMixer* module = dynamic_cast<BitMixer*> (this->module);
        assert(module);

        menu->addChild(new MenuSeparator);
        menu->addChild(createIndexSubmenuItem(
            "Bit Shift Mode",
            {"rotate", "shift"},
            [=]() {return module->shift_mode;},
            [=](int i) {module->shift_mode = i;}
        ));
        menu->addChild(createIndexSubmenuItem(
            "Clipping Mode",
            {"hard", "tanh", "sqrt", "CV"},
            [=]() {return module->clip_mode;},
            [=](int i) {module->clip_mode = i;}
        ));
    }
};

Model* model_BitMixer = createModel<BitMixer, BitMixer_Widget>("BitMixer");

