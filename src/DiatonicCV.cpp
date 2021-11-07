#include "plugin.hpp"
#include "musiclib.hpp"

struct DiatonicCV : Module {
	enum ParamIds {
		OCTAVE_PARAM,
		CHORD_PARAM,
		TYPE_PARAM,
		INVERSION_PARAM,
		VOICING_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		OCTAVE_INPUT,
		CHORD_INPUT,
		TYPE_INPUT,
		INVERSION_INPUT,
		VOICING_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		POLY_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		NUM_LIGHTS
	};

	int octave = 4;
	int chord = 0;
	int chord_type = 0;
	bool inverted = false;
	bool hasPoly = true;
	int bass_note = 0;
	float polyNotes_v[16] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
	int polyNotes[16] = {48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48};
	int polyChannels = 0;
	int inversion = 0;
	int voicing = 0;
	struct chord playing_chord;
	struct scale cmajor;

	RefreshCounter refresh;

	DiatonicCV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configParam(OCTAVE_PARAM, -4.0, 4.0, 0.0, "Octave");
		configParam(CHORD_PARAM, 0, 6.0, 0.0, "Chord (I - VII)");
		configParam(TYPE_PARAM, 0, 2.0, 0.0, "Chord Type");
		configParam(INVERSION_PARAM, 0.0, 4.0, 0.0, "Inversion");
		configParam(VOICING_PARAM, 0.0, 4.0, 0.0, "Voicing");

		configInput(POLY_INPUT, "Polyphonic");
		configInput(OCTAVE_INPUT, "Octave");
		configInput(CHORD_INPUT, "Chord");
		configInput(TYPE_INPUT, "Chord Type");
		configInput(INVERSION_INPUT, "Inversion");
		configInput(VOICING_INPUT, "Voicing");

		configOutput(POLY_OUTPUT, "Polyphonic");

		cmajor = get_scale(0,0);
	}

	void process(const ProcessArgs& args) override;
};

void DiatonicCV::process(const ProcessArgs &args){
	if (refresh.processInputs()) {
		if(inputs[POLY_INPUT].isConnected()){
			polyChannels = inputs[POLY_INPUT].getChannels();
			for (int c = 0; c < 16; c++) {
				float v = inputs[POLY_INPUT].getVoltage(c);
				polyNotes_v[c] = v;
				polyNotes[c] = voltage_to_note_with_octave(v);
			}
			//sort the notes in ascending order
			std::sort(std::begin(polyNotes), polyNotes + polyChannels);
		}else{
			//Just make it C Major
			polyChannels = 7;
			for(int t=0; t<7; t++){
				polyNotes[t] = cmajor.notes[t];
			}
		}

		//remove octave
		if(polyChannels > 0){
			int root_octave = (int)floor((float)polyNotes[0] / 12.0f);
			for (int c = 0; c < polyChannels; c++) {
				polyNotes[c] -= root_octave * 12;
			}
		}

		float octave_v = params[OCTAVE_PARAM].getValue();
		if(inputs[OCTAVE_INPUT].isConnected()){
			octave_v = inputs[OCTAVE_INPUT].getVoltage();
		}
		octave = (int)round(octave_v) + 4;

		float chord_v = params[CHORD_PARAM].getValue();
		if(inputs[CHORD_INPUT].isConnected()){
			chord_v = inputs[CHORD_INPUT].getVoltage();
		}
		chord = (int)round(clamp(chord_v, 0.0f, 6.0f));

		float type_v = params[TYPE_PARAM].getValue();
		if(inputs[TYPE_INPUT].isConnected()){
			type_v = inputs[TYPE_INPUT].getVoltage();
		}
		chord_type = (int)round(clamp(type_v, 0.0f, 2.0f));

		//inversion
		float inversion_v = params[INVERSION_PARAM].getValue();
		if(inputs[INVERSION_INPUT].isConnected()){
			inversion_v = inputs[INVERSION_INPUT].getVoltage();
		}
		inversion = (int)round(clamp(inversion_v, 0.0f, 4.0f));

		//voicing
		float voicing_v = params[VOICING_PARAM].getValue();
		if(inputs[VOICING_INPUT].isConnected()){
			voicing_v = inputs[VOICING_INPUT].getVoltage();
		}
		voicing = (int)round(clamp(voicing_v, 0.0f, 4.0f));

		//Make the chord
		playing_chord = get_diatonic_chord(polyNotes, polyChannels, octave, chord, chord_type, inversion, voicing);
	}

	if (refresh.processLights()) {

	}
	if(polyChannels > 0){
		outputs[POLY_OUTPUT].setChannels(playing_chord.num_notes);
		for(int t=0; t<playing_chord.num_notes; t++){
			outputs[POLY_OUTPUT].setVoltage(note_to_voltage(playing_chord.notes[t]),t);
		}
	}
}


struct DiatonicCVWidget : ModuleWidget {
	struct ChordDisplayWidget : TransparentWidget {
		DiatonicCV* module;
		char text[13] = "";

		ChordDisplayWidget(Vec _pos, Vec _size, DiatonicCV* _module) {
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

					if (module != NULL && module->playing_chord.num_notes > 2){
						detect_chord_name_simple(module->playing_chord,text);
					}else{
						snprintf(text, 13, "             ");
					}

					nvgText(args.vg, textPos.x, textPos.y, text, NULL);
				}
			}
			Widget::drawLayer(args, layer);
		}

	};

	DiatonicCVWidget(DiatonicCV* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/DiatonicCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const int centerX = box.size.x / 2;

		ChordDisplayWidget* display = new ChordDisplayWidget(Vec(centerX, 55), Vec(box.size.x - 5, 29), module);
		addChild(display);

		addInput(createInputCentered<PJ301MPort>(Vec(centerX, 95), module, DiatonicCV::POLY_INPUT));

		const int offsetXL = 40;
		const int spacingY = 45;
		const int y1 = 134;
		const int y2 = y1 + spacingY;
		const int y3 = y2 + spacingY;


		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,y1), module, DiatonicCV::OCTAVE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, y1), module, DiatonicCV::OCTAVE_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,y2), module, DiatonicCV::CHORD_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, y2), module, DiatonicCV::CHORD_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,y3), module, DiatonicCV::TYPE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, y3), module, DiatonicCV::TYPE_INPUT));

		static const int offsetX2 = 14;
		static const int posY = 269;

		const int offsetXL2 = 42;

		addParam(createParamCentered<RoundSmallBlackKnob>(Vec(centerX - offsetX2,posY), module, DiatonicCV::INVERSION_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL2, posY), module, DiatonicCV::INVERSION_INPUT));

		addParam(createParamCentered<RoundSmallBlackKnob>(Vec(centerX + offsetX2,posY), module, DiatonicCV::VOICING_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX + offsetXL2, posY), module, DiatonicCV::VOICING_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 332), module, DiatonicCV::POLY_OUTPUT));
	}
};


Model* modelDiatonicCV = createModel<DiatonicCV, DiatonicCVWidget>("DiatonicCV");
