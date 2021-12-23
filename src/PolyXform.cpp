#include "plugin.hpp"



/* ==== Math ==== */

/*
note: matrices are store in row major order here

memory
------>
*  *  *  *
*  *  *  *
*  *  *  *
*  *  *  *

A         B                out
m=3 n=2   n=2 p=4          m=3 p=4
*  *                       *  *  *  *
*  *      *  *  *  *       *  *  *  *
*  *      *  *  *  *       *  *  *  *

*/

// out = AB, like in math, means first B then A
static inline void matrix_mul(float* out, float* A, float* B, int m, int n, int p) {
    for (int i = 0; i < m; i++) {
        for (int k = 0; k < n; k++) {
            for (int j = 0; j < p; j++) {
                out[i * p + j] += A[i * n + k] * B[k * p + j];
            }
        }
    }
}







/* ==== Module ==== */

struct PolyXform: Module {

    enum ParamId {
        MATRIX1_ROW,
        MATRIX1_COL,
        MATRIX2_ROW,
        MATRIX2_COL,
        SCALE1,
        SCALE2,
        PARAMS_LEN
    };

    enum InputId {
        POLY_INPUT1,
        POLY_INPUT2,
        INPUTS_LEN
    };

    enum OutputId {
        OUTPUT,
        OUTPUTS_LEN
    };

    enum LightId {
        ENUMS(MATRIX_LIGHTS, 16 * 2),
        LIGHTS_LEN
    };

    PolyXform() {

        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(MATRIX1_ROW,   1, 4, 2, "Matrix 1 Row");
        configParam(MATRIX1_COL,   1, 4, 2, "Matrix 1 Column");
        configParam(MATRIX2_ROW,   1, 4, 2, "Matrix 2 Row");
        configParam(MATRIX2_COL,   1, 4, 2, "Matrix 2 Column");
        configParam(SCALE1,        0, 1, 0.5, "Scale 1");
        configParam(SCALE2,        0, 1, 0.5, "Scale 2");
        configInput(POLY_INPUT1, "Matrix 1");
        configInput(POLY_INPUT2, "Matrix 2");
        configOutput(OUTPUT,     "Matrix");

        paramQuantities[MATRIX1_ROW]->snapEnabled = true;
        paramQuantities[MATRIX1_COL]->snapEnabled = true;
        paramQuantities[MATRIX2_ROW]->snapEnabled = true;
        paramQuantities[MATRIX2_COL]->snapEnabled = true;

        divider.setDivision(16);
    }

	dsp::ClockDivider divider;

    void process(const ProcessArgs& args) override {
        
        float matrix1   [16] = {0};
        float matrix2   [16] = {0};
        float matrix_out[16] = {0};

        int row1 = (int) params[MATRIX1_ROW].getValue();
        int col1 = (int) params[MATRIX1_COL].getValue();
        int row2 = (int) params[MATRIX2_ROW].getValue();
        int col2 = (int) params[MATRIX2_COL].getValue();

        float scale1 = params[SCALE1].getValue();
        float scale2 = params[SCALE2].getValue();
        
        int min_side = col1 < row2 ? col1 : row2; // get the n in m n p, choose min of those because we want data to be fully filled
		for (int i = 0; i < row1 * min_side; i++)  matrix1[i] = inputs[POLY_INPUT1].getVoltage(i) * scale1;
        for (int i = 0; i < min_side * col2; i++)  matrix2[i] = inputs[POLY_INPUT2].getVoltage(i) * scale2;

        matrix_mul(matrix_out, matrix1, matrix2, row1, min_side, col2);

        int out_count = row1 * col2;
        outputs[OUTPUT].channels = out_count; 
        for (int i = 0; i < out_count; i++) {
            outputs[OUTPUT].setVoltage(matrix_out[i], i);
        }

        /* ---- Set Lights ---- */
		if (!divider.process()) return;

        float dt = args.sampleTime * divider.getDivision();
        for (int i = 0; i < 16 * 2; i++) {
            lights[MATRIX_LIGHTS + i].setSmoothBrightness(0, dt);
        }

        for (int i = 0; i < row1; i++) {
            for (int j = 0; j < col2; j++) {
                float v = matrix_out[i * col2 + j] / 10.0;
                lights[MATRIX_LIGHTS + (i * 4 + j) * 2    ].setSmoothBrightness( v, dt);
                lights[MATRIX_LIGHTS + (i * 4 + j) * 2 + 1].setSmoothBrightness(-v, dt);
            }
        }
    }
};


struct PolyXform_Widget: ModuleWidget {

    PolyXform_Widget(PolyXform* module) {

        // we use px here
        // 1HP        = 15px
        // Height: 3U = 380px
        // 6HP = 15px * 6 = 90px

        setModule(module);
        setPanel(createPanel(asset::plugin(plugin_instance, "res/PolyXform.svg")));

        // "C++ Template is so great!!! It reduces code duplication!!!"
        #define make_param(name, widget, x, y)  addParam(createParamCentered<widget>(Vec(x, y), module, PolyXform::name))
        #define make_input(name, widget, x, y)  addInput(createInputCentered<widget>(Vec(x, y), module, PolyXform::name))
        #define make_output(name, widget, x, y) addOutput(createOutputCentered<widget>(Vec(x, y), module, PolyXform::name))

        make_param(MATRIX1_ROW, Nepeta_SmallKnob, 25, 380 * 0.35);
        make_param(MATRIX1_COL, Nepeta_SmallKnob, 65, 380 * 0.35);
        make_param(MATRIX2_ROW, Nepeta_SmallKnob, 25, 380 * 0.45);
        make_param(MATRIX2_COL, Nepeta_SmallKnob, 65, 380 * 0.45);

        make_input(POLY_INPUT1, Nepeta_Port,      25, 380 * 0.6);
        make_param(SCALE1,      Nepeta_SmallKnob, 65, 380 * 0.6);

        make_input(POLY_INPUT2, Nepeta_Port,      25, 380 * 0.7);
        make_param(SCALE2,      Nepeta_SmallKnob, 65, 380 * 0.7);

        make_output(OUTPUT,     Nepeta_Port,      45, 380 * 0.85);
        
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                addChild(createLightCentered<MediumSimpleLight<GreenRedLight>>(
                    Vec(22.5 + j * 15, 54 + i * 15), 
                    module, 
                    PolyXform::MATRIX_LIGHTS + 2 * (i * 4 + j)
                ));
            }
        }

        #undef make_param
        #undef make_input
        #undef make_output
    }
};



Model* model_PolyXform = createModel<PolyXform, PolyXform_Widget>("PolyXform");

