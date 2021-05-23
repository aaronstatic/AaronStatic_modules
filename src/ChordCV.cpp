#include "plugin.hpp"


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
		configParam(ROOT_PARAM, -4.0, 4.0, 0.0, "");
		configParam(TYPE_PARAM, -4.0, 4.0, -4.0, "");
		configParam(INVERSION_PARAM, 0.0, 3.0, 0.0, "");
		configParam(VOICING_PARAM, 0.0, 4.0, 0.0, "");
	}

	void process(const ProcessArgs& args) override;
};

inline float note_to_voltage(float &v) {
	float octave = floor(v / 12.0) - 4.0;
	float semi = round((int)v % 12);

	return 1.0*octave + semi/12.0;
}

inline float voltage_to_note(float &value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	float semi = round( rest*12.0 );
	return semi;
}

inline int voltage_to_note_int(float &value) {
	float octave = round(value);
	float rest = value - 1.0*octave;
	int semi = (int)round( rest*12.0 );
	if(semi < 0) semi += 12;
	return semi;
}

void ChordCV::process(const ProcessArgs &args){
	int degrees[9][3] = {
		{4,7,0}, //Major
		{3,7,0}, //Minor
		{4,7,10}, //Dominant 7
		{3,7,10}, //Minor 7
		{4,7,11}, //Major 7
		{2,7,0}, //sus2
		{5,7,0}, //sus4
		{3,6,0}, //dim
		{4,8,0} //aug
	};
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

	//Make the chord
	int * chord = degrees[chord_type];

	float root_note = (octave + 4.0) * 12.0 + semi;
	float third_note = root_note + (float)chord[0];
	float fifth_note = root_note + (float)chord[1];
	float seventh_note = root_note + (float)chord[2];

	float root_v = note_to_voltage(root_note);
	float third_v = note_to_voltage(third_note);
	float fifth_v = note_to_voltage(fifth_note);
	float seventh_v=0.0;
	if(chord[2] == 0){
		if(inversion == 3){
			inversion = 2;
		}
		if(inversion == 1){
			root_v += 1.0;
			float v = root_v;
			root_v = third_v;
			third_v = fifth_v;
			fifth_v = v;
		}
		if(inversion == 2){
			root_v += 1.0;
			third_v += 1.0;
			float v = root_v;
			root_v = fifth_v;
			fifth_v = third_v;
			third_v = v;
		}

		if(voicing == 1){
			root_v -= 1.0;
		}
		if(voicing == 2){
			root_v -= 1.0;
			fifth_v -= 1.0;
		}
		if(voicing == 3 || voicing == 4){
			root_v -= 1.0;
			third_v += 1.0;
		}

		bass_note = voltage_to_note_int(root_v);
		seventh_v = root_v + 1.0;
		outputs[POLY_OUTPUT].setChannels(3);
	}else{
		seventh_v = note_to_voltage(seventh_note);
		if(inversion == 1){
			root_v += 1.0;
			float v = root_v;
			root_v = third_v;
			third_v = fifth_v;
			fifth_v = seventh_v;
			seventh_v = v;
		}
		if(inversion == 2){
			root_v += 1.0;
			third_v += 1.0;
			float v = third_v;
			float r = root_v;
			root_v = fifth_v;
			fifth_v = r;
			third_v = seventh_v;
			seventh_v = v;
		}
		if(inversion == 3){
			root_v += 1.0;
			third_v += 1.0;
			fifth_v += 1.0;
			float v = fifth_v;
			float r = root_v;
			root_v = seventh_v;
			seventh_v = v;
			fifth_v = third_v;
			third_v = r;
		}

		if(voicing == 1){
			root_v -= 1.0;
		}
		if(voicing == 2){
			root_v -= 1.0;
			fifth_v -= 1.0;
		}
		if(voicing == 3){
			root_v -= 1.0;
			third_v += 1.0;
		}
		if(voicing == 4){
			root_v -= 1.0;
			seventh_v -= 1.0;
		}

		bass_note = voltage_to_note_int(root_v);
		outputs[POLY_OUTPUT].setChannels(4);
	}

	outputs[CV_OUTPUTS + 0].setVoltage(root_v);
	outputs[CV_OUTPUTS + 1].setVoltage(third_v);
	outputs[CV_OUTPUTS + 2].setVoltage(fifth_v);
	outputs[CV_OUTPUTS + 3].setVoltage(seventh_v);

	outputs[POLY_OUTPUT].setVoltage(root_v,0);
	outputs[POLY_OUTPUT].setVoltage(third_v,1);
	outputs[POLY_OUTPUT].setVoltage(fifth_v,2);
	if(chord[2] > 0){
		outputs[POLY_OUTPUT].setVoltage(seventh_v,3);
	}
}


struct ChordCVWidget : ModuleWidget {
	struct ChordDisplayWidget : TransparentWidget {
		ChordCV* module;
		std::shared_ptr<Font> font;
		char text[10];

		ChordDisplayWidget(Vec _pos, Vec _size, ChordCV* _module) {
			box.size = _size;
			box.pos = _pos.minus(_size.div(2));
			module = _module;
			font = APP->window->loadFont(asset::plugin(pluginInstance, "res/fonts/PixelOperator.ttf"));
		}

		void chordName() {
			if (module != NULL){
				static const char * noteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
				static const char * chordTypes[] = {
				  "",
				  "m",
				  "7",
				  "m7",
				  "maj7",
				  "sus2",
				  "sus4",
				  "dim",
				  "+"
				};

				int note = module->root_semi;
				int type = module->chord_type;
				char inv[4];
				if(module->inverted){
					sprintf(inv,"/%s",noteNames[module->bass_note]);
				}
				sprintf(text, "%s%s%s", noteNames[note], chordTypes[type], inv);

			}else{
				snprintf(text, 9, "         ");
			}
		}

		void draw(const DrawArgs &args) override {
			NVGcolor textColor = prepareDisplay(args.vg, &box, 22);
			nvgFontFaceId(args.vg, font->handle);
			nvgTextLetterSpacing(args.vg, -1.5);
			nvgTextAlign(args.vg, NVG_ALIGN_CENTER);

			Vec textPos = Vec(box.size.x/2, 21.0f);
			nvgFillColor(args.vg, textColor);
			chordName();
			nvgText(args.vg, textPos.x, textPos.y, text, NULL);
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

		ChordDisplayWidget* display = new ChordDisplayWidget(Vec(centerX, 55), Vec(76, 29), module);
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

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 330), module, ChordCV::POLY_OUTPUT));
	}
};


Model* modelChordCV = createModel<ChordCV, ChordCVWidget>("ChordCV");
