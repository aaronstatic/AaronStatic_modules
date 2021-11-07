#include "plugin.hpp"
#include "musiclib.hpp"

struct ChordCV : Module {
	enum ParamIds {
		ROOT_PARAM,
		TYPE_PARAM,
		INVERSION_PARAM,
		VOICING_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		ROOT_INPUT,
		TYPE_INPUT,
		INVERSION_INPUT,
		VOICING_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		ENUMS(CV_OUTPUTS, 4),
		POLY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int root_semi = 0;
	int chord_type = 0;
	bool inverted = false;
	int bass_note = 0;

	ChordCV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(ROOT_PARAM, -4.0, 4.0, 0.0, "Root Note");
		configParam(TYPE_PARAM, -4.0, 4.0, -4.0, "Chord Type");
		configParam(INVERSION_PARAM, 0.0, 3.0, 0.0, "Inversion");
		configParam(VOICING_PARAM, 0.0, 4.0, 0.0, "Voicing");

		configInput(ROOT_INPUT, "1V/oct pitch");
		configInput(TYPE_INPUT, "Chord Type");
		configInput(INVERSION_INPUT, "Inversion");
		configInput(VOICING_INPUT, "Voicing");

		configOutput(POLY_OUTPUT, "Polyphonic");
	}

	void process(const ProcessArgs& args) override;
};

void ChordCV::process(const ProcessArgs &args){

	float value = params[ROOT_PARAM].getValue();
	if(inputs[ROOT_INPUT].isConnected()){
		value = inputs[ROOT_INPUT].getVoltage();
	}
	float type = params[TYPE_PARAM].getValue();
	if(inputs[TYPE_INPUT].isConnected()){
		type = inputs[TYPE_INPUT].getVoltage();
		type = clamp(type,-4.0f, 4.0f);
	}

	chord_type = (int)round(type + 4.0f);

	//quantize root note
	float octave = round(value);
	float semi = voltage_to_note(value);
	root_semi = voltage_to_note_int(value);

	//inversion
	int inversion = (int)round(params[INVERSION_PARAM].getValue());
	if(inputs[INVERSION_PARAM].isConnected()){
		inversion = (int)clamp(round(inputs[INVERSION_PARAM].getVoltage()),0.0f,3.0f);
	}
	if(inversion > 0) inverted = true; else inverted = false;

	//voicing
	int voicing = (int)round(params[VOICING_PARAM].getValue());
	if(inputs[VOICING_PARAM].isConnected()){
		voicing = (int)clamp(round(inputs[VOICING_PARAM].getVoltage()),0.0f,4.0f);
	}

	int root_note = (octave + 4) * 12 + (int)semi;

	//Make the chord
	struct chord c = get_chord(root_note,chord_type,inversion,voicing);

	if(inverted){
		bass_note = c.notes[0] % 12;
	}

	outputs[CV_OUTPUTS + 0].setVoltage(note_to_voltage(c.notes[0]));
	outputs[CV_OUTPUTS + 1].setVoltage(note_to_voltage(c.notes[1]));
	outputs[CV_OUTPUTS + 2].setVoltage(note_to_voltage(c.notes[2]));
	if(c.num_notes == 3){
		outputs[CV_OUTPUTS + 3].setVoltage(note_to_voltage(c.notes[0] + 12));
	}else{
		outputs[CV_OUTPUTS + 3].setVoltage(note_to_voltage(c.notes[3]));
	}

	outputs[POLY_OUTPUT].setChannels(c.num_notes);
	for(int t=0; t<c.num_notes; t++){
		outputs[POLY_OUTPUT].setVoltage(note_to_voltage(c.notes[t]),t);
	}
}


struct ChordCVWidget : ModuleWidget {
	struct ChordDisplayWidget : TransparentWidget {
		ChordCV* module;
		char text[13];

		ChordDisplayWidget(Vec _pos, Vec _size, ChordCV* _module) {
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
						get_chord_name(module->root_semi,module->chord_type,module->inverted,module->bass_note,text);
					}else{
						snprintf(text, 13, "             ");
					}

					nvgText(args.vg, textPos.x, textPos.y, text, NULL);
				}
			}
			Widget::drawLayer(args, layer);
		}

	};

	ChordCVWidget(ChordCV* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/ChordCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const int centerX = box.size.x / 2;

		ChordDisplayWidget* display = new ChordDisplayWidget(Vec(centerX, 55), Vec(box.size.x - 5, 29), module);
		addChild(display);

		const int offsetXL = 40;


		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,95), module, ChordCV::ROOT_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 95), module, ChordCV::ROOT_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,140), module, ChordCV::TYPE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 140), module, ChordCV::TYPE_INPUT));

		static const int spacingY2 = 46;
		static const int offsetX = 28;
		static const int offsetX2 = 14;
		static const int posY2 = 253;
		static const int posY = 180;

		const int offsetXL2 = 42;

		addParam(createParamCentered<RoundSmallBlackKnob>(Vec(centerX - offsetX2,posY), module, ChordCV::INVERSION_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL2, posY), module, ChordCV::INVERSION_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(Vec(centerX + offsetX2,posY), module, ChordCV::VOICING_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX + offsetXL2, posY), module, ChordCV::VOICING_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX - offsetX, posY2), module, ChordCV::CV_OUTPUTS + 0));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX - offsetX, posY2 + spacingY2), module, ChordCV::CV_OUTPUTS + 1));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY2), module, ChordCV::CV_OUTPUTS + 2));
		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX + offsetX, posY2 + spacingY2), module, ChordCV::CV_OUTPUTS + 3));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 332), module, ChordCV::POLY_OUTPUT));
	}
};


Model* modelChordCV = createModel<ChordCV, ChordCVWidget>("ChordCV");
