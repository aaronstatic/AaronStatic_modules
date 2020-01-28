#include "plugin.hpp"


struct Scratch : Module {
	enum ParamIds {
		ROOT_PARAM_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		NUM_INPUTS
	};
	enum OutputIds {
		CVOUT1_OUTPUT_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	Scratch() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROOT_PARAM_PARAM, 0.f, 1.f, 0.f, "");
	}

	void process(const ProcessArgs& args) override {
	}
};


struct ScratchWidget : ModuleWidget {
	ScratchWidget(Scratch* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/Scratch.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(20.32, 25.639)), module, Scratch::ROOT_PARAM_PARAM));

		addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(20.32, 119.596)), module, Scratch::CVOUT1_OUTPUT_OUTPUT));
	}
};


Model* modelScratch = createModel<Scratch, ScratchWidget>("Scratch");