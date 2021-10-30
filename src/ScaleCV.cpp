#include "plugin.hpp"
#include "musiclib.hpp"

struct ScaleCV : Module {
	enum ParamIds {
		ROOT_PARAM,
		MODE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ROOT_INPUT,
		MODE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int root_semi = 0;
	int mode = 0;

	ScaleCV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROOT_PARAM, -4.0, 4.0, 0.0, "Root Note");
		configParam(MODE_PARAM, -4.0, 4.0, -4.0, "Mode");

		configInput(ROOT_INPUT, "1V/oct pitch");
		configInput(MODE_INPUT, "Mode");

		configOutput(POLY_OUTPUT, "Polyphonic");
	}

	void process(const ProcessArgs& args) override;
};

void ScaleCV::process(const ProcessArgs &args){
	float value = params[ROOT_PARAM].getValue();
	if(inputs[ROOT_INPUT].isConnected()){
		value = inputs[ROOT_INPUT].getVoltage();
	}
	float mode_val = params[MODE_PARAM].getValue();
	if(inputs[MODE_INPUT].isConnected()){
		mode_val = inputs[MODE_INPUT].getVoltage();
	}
	mode_val = clamp(mode_val,-4.0f, 2.0f);
	mode = (int)floor(mode_val + 4.0f);

	//quantize root note
	float octave = round(value);
	float semi = voltage_to_note(value);
	root_semi = voltage_to_note_int(value);
	int root_note = (octave + 4) * 12 + (int)semi;

	//Make the scale
	struct scale s = get_scale(root_note, mode);

	outputs[POLY_OUTPUT].setChannels(7);
	for(int t=0; t<7; t++){
		outputs[POLY_OUTPUT].setVoltage(note_to_voltage(s.notes[t]),t);
	}
}


struct ScaleCVWidget : ModuleWidget {
	struct ChordDisplayWidget : TransparentWidget {
		ScaleCV* module;
		std::shared_ptr<Font> font;
		char text[13];

		ChordDisplayWidget(Vec _pos, Vec _size, ScaleCV* _module) {
			box.size = _size;
			box.pos = _pos.minus(_size.div(2));
			module = _module;
			font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/PixelOperator.ttf"));
		}

		void draw(const DrawArgs &args) override {
			NVGcolor textColor = prepareDisplay(args.vg, &box, 22);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, -1.5);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			Vec textPos = Vec(box.size.x/2, 21.0f);
			nvgFillColor(args.vg, textColor);

			if (module != NULL){
				get_scale_name(module->root_semi,module->mode,text);
			}else{
				snprintf(text, 13, "             ");
			}

			nvgText(args.vg, textPos.x, textPos.y, text, NULL);
		}

	};

	ScaleCVWidget(ScaleCV* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ScaleCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const int centerX = box.size.x / 2;

		ChordDisplayWidget* display = new ChordDisplayWidget(Vec(centerX, 55), Vec(box.size.x - 5, 29), module);
		addChild(display);

		const int offsetXL = 40;


		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,95), module, ScaleCV::ROOT_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 95), module, ScaleCV::ROOT_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,140), module, ScaleCV::MODE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 140), module, ScaleCV::MODE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 330), module, ScaleCV::POLY_OUTPUT));
	}
};


Model* modelScaleCV = createModel<ScaleCV, ScaleCVWidget>("ScaleCV");
