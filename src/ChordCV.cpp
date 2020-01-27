#include "plugin.hpp"


struct ChordCV : Module {
	enum ParamIds {
		ROOT_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CVOUT1_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	ChordCV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROOT_PARAM, -4.0, 4.0, 0.0, "");
	}

	void process(const ProcessArgs& args) override {
		float note = params[ROOT_PARAM].getValue();
		float octave = round(note);
		float rest = note - 1.0*octave;
		float semi = round( rest*12.0 );
		float quantized = 1.0*octave + semi/12.0;

		outputs[CVOUT1_OUTPUT].setVoltage(quantized);
	}
};


struct ChordCVWidget : ModuleWidget {
	ChordCVWidget(ChordCV* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChordCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(9.889, 18.836)), module, ChordCV::ROOT_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(10.253, 123.306)), module, ChordCV::CVOUT1_OUTPUT));
	}
};


Model* modelChordCV = createModel<ChordCV, ChordCVWidget>("ChordCV");
