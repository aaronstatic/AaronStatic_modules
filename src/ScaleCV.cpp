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
		ENUMS(QUANTIZER_INPUTS, 4),
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		ENUMS(QUANTIZER_OUTPUTS, 4),
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

	//Quantizers
	for(int t=0; t<4; t++){
		if(inputs[QUANTIZER_INPUTS + t].isConnected() && outputs[QUANTIZER_OUTPUTS + t].isConnected()){
			float in_v = inputs[QUANTIZER_INPUTS + t].getVoltage();
			float in_octave = round(in_v) + 4;
			float in_semi = voltage_to_note(in_v);
			float lowest_dist = 12.0f;
			float out_note = 0.0f;
			for(int i=0; i<7; i++){
				float note = (float)(s.notes[i] - ((octave + 4) * 12));
				float dist = abs(note - in_semi);
				if(dist < lowest_dist){
					out_note = note;
					lowest_dist = dist;
				}
				//check one octave down
				note -= 12;
				dist = abs(note - in_semi);
				if(dist < lowest_dist){
					out_note = note;
					lowest_dist = dist;
				}
			}
			out_note += in_octave * 12.0f;
			outputs[QUANTIZER_OUTPUTS+t].setVoltage(note_to_voltage((int)out_note));
		}

	}

	outputs[POLY_OUTPUT].setChannels(7);
	for(int t=0; t<7; t++){
		outputs[POLY_OUTPUT].setVoltage(note_to_voltage(s.notes[t]),t);
	}
}


struct ScaleCVWidget : ModuleWidget {
	struct ChordDisplayWidget : TransparentWidget {
		ScaleCV* module;
		char text[13] = "";

		ChordDisplayWidget(Vec _pos, Vec _size, ScaleCV* _module) {
			box.size = _size;
			box.pos = _pos.minus(_size.div(2));
			module = _module;
		}

		void drawLayer(const DrawArgs& args, int layer) override {
			if (layer == 1) {
				std::shared_ptr<Font> font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/PixelOperator.ttf"));
				if(font){
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
			}
			Widget::drawLayer(args, layer);
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

		static const int offsetX = 28;
		static const int posY = 190;
		static const int spacingY2 = 32;
		static const int posY1 = posY + spacingY2;
		static const int posY2 = posY + (spacingY2 * 2);
		static const int posY3 = posY + (spacingY2 * 3);

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,95), module, ScaleCV::ROOT_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 95), module, ScaleCV::ROOT_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,140), module, ScaleCV::MODE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 140), module, ScaleCV::MODE_INPUT));

		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetX, posY), module, ScaleCV::QUANTIZER_INPUTS + 0));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetX, posY1), module, ScaleCV::QUANTIZER_INPUTS + 1));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetX, posY2), module, ScaleCV::QUANTIZER_INPUTS + 2));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetX, posY3), module, ScaleCV::QUANTIZER_INPUTS + 3));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY), module, ScaleCV::QUANTIZER_OUTPUTS + 0));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY1), module, ScaleCV::QUANTIZER_OUTPUTS + 1));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY2), module, ScaleCV::QUANTIZER_OUTPUTS + 2));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY3), module, ScaleCV::QUANTIZER_OUTPUTS + 3));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 332), module, ScaleCV::POLY_OUTPUT));
	}
};


Model* modelScaleCV = createModel<ScaleCV, ScaleCVWidget>("ScaleCV");
