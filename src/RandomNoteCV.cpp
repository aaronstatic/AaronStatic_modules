#include "plugin.hpp"
#include "musiclib.hpp"

struct RandomNoteCV : Module {
	enum ParamIds {
		TRIGGER_PARAM,
		BIAS_PARAM,
		RANGE_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		POLY_INPUT,
		TRIGGER_INPUT,
		BIAS_INPUT,
		RANGE_INPUT,
		NUM_INPUTS
	};
	enum OutputIds {
		NOTE_OUTPUT,
		NUM_OUTPUTS
	};
	enum LightIds {
		TRIGGER_LIGHT,
		NUM_LIGHTS
	};

	Trigger randomTrigger;
	int playing_note = 48;
	float playing_voltage = note_to_voltage(playing_note);
	float triggerLight = 0.0f;
	float range = 1.0f;
	float bias = 0.5f;
	double sampleTime;
	float polyNotes_v[16] = {0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f};
	int polyNotes[16] = {48,48,48,48,48,48,48,48,48,48,48,48,48,48,48,48};
	int polyChannels = 0;

	bool hasPoly = false;

	RefreshCounter refresh;

	RandomNoteCV() {
		config(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS);
		configButton(TRIGGER_PARAM);
		configParam(BIAS_PARAM, -4.0, 4.0, 0.0, "Root Bias");
		configParam(RANGE_PARAM, -4.0, 4.0, -3.0, "Range");

		configInput(POLY_INPUT, "1V/oct Poly");
		configInput(TRIGGER_INPUT, "Trigger");
		configInput(BIAS_INPUT, "Root Bias");
		configInput(RANGE_INPUT, "Range");

		configOutput(NOTE_OUTPUT, "1v/oct");
	}

	void process(const ProcessArgs& args) override;
};

void RandomNoteCV::process(const ProcessArgs &args){
	sampleTime = 1.0 / (double)(APP->engine->getSampleRate());

	if (refresh.processInputs()) {
		if(inputs[POLY_INPUT].isConnected()){
			hasPoly = true;
			polyChannels = inputs[POLY_INPUT].getChannels();
			for (int c = 0; c < 16; c++) {
				float v = inputs[POLY_INPUT].getVoltage(c);
				polyNotes_v[c] = v;
				polyNotes[c] = voltage_to_note_int(v);
			}
		}else{
			hasPoly = false;
		}

		float range_v = params[RANGE_PARAM].getValue();
		if(inputs[RANGE_INPUT].isConnected()){
			range_v = inputs[RANGE_INPUT].getVoltage();
		}
		range = 1.0f + (range_v + 4.0f);

		float bias_v = params[BIAS_PARAM].getValue();
		if(inputs[BIAS_INPUT].isConnected()){
			bias_v = inputs[BIAS_INPUT].getVoltage();
		}
		bias = (bias_v + 4.0f) / 8.0f;
	}

	if (randomTrigger.process(inputs[TRIGGER_INPUT].getVoltage() + params[TRIGGER_PARAM].getValue())) {
		triggerLight = 1.0f;
		//Select a new note
		if(!hasPoly){
			playing_note = 36 + (int)round(random::uniform() * (range * 12.0f));
			playing_voltage = note_to_voltage(playing_note);
		}else{
			float root_octave = round(polyNotes_v[0]) + 4.0f;
			int index = (int)round(random::uniform() * (float)polyChannels);
			if(bias > 0.0f && random::uniform() < bias){
				index = 0;
			}
			int octave = (int)root_octave + (int)round(random::uniform() * range);
			playing_note = polyNotes[index] + (12 * octave);
			playing_voltage = note_to_voltage(playing_note);
		}
	}

	// lights
	if (refresh.processLights()) {
		// Trigger light
		lights[TRIGGER_LIGHT].setSmoothBrightness(triggerLight, (float)sampleTime * (RefreshCounter::displayRefreshStepSkips >> 2));
		triggerLight = 0.0f;
	}

	outputs[NOTE_OUTPUT].setVoltage(playing_voltage);
}


struct RandomNoteCVWidget : ModuleWidget {
	struct NoteDisplayWidget : TransparentWidget {
		RandomNoteCV* module;
		std::shared_ptr<Font> font;
		char text[2];

		NoteDisplayWidget(Vec _pos, Vec _size, RandomNoteCV* _module) {
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
				get_note_name(module->playing_note,text);
			}else{
				snprintf(text, 1, " ");
			}

			nvgText(args.vg, textPos.x, textPos.y, text, NULL);
		}

	};

	RandomNoteCVWidget(RandomNoteCV* module) {
		setModule(module);
		setPanel(APP->window->loadSvg(asset::plugin(pluginInstance, "res/RandomNoteCV.svg")));

		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, 0)));
		addChild(createWidget<ScrewSilver>(Vec(RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));
		addChild(createWidget<ScrewSilver>(Vec(box.size.x - 2 * RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)));

		const int centerX = box.size.x / 2;

		NoteDisplayWidget* display = new NoteDisplayWidget(Vec(centerX, 55), Vec(76, 29), module);
		addChild(display);

		const int offsetXL = 40;

		addInput(createInputCentered<PJ301MPort>(Vec(centerX, 95), module, RandomNoteCV::POLY_INPUT));

		addParam(createParamCentered<LEDBezel>(Vec(centerX,155), module, RandomNoteCV::TRIGGER_PARAM));
		addChild(createLightCentered<LEDBezelLight<GreenLight>>(Vec(centerX,155), module, RandomNoteCV::TRIGGER_LIGHT));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 155), module, RandomNoteCV::TRIGGER_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,200), module, RandomNoteCV::BIAS_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 200), module, RandomNoteCV::BIAS_INPUT));

		addParam(createParamCentered<Rogan2PWhite>(Vec(centerX,245), module, RandomNoteCV::RANGE_PARAM));
		addInput(createInputCentered<PJ301MPort>(Vec(centerX - offsetXL, 245), module, RandomNoteCV::RANGE_INPUT));

		addOutput(createOutputCentered<PJ301MPort>(Vec(centerX, 330), module, RandomNoteCV::NOTE_OUTPUT));
	}
};


Model* modelRandomNoteCV = createModel<RandomNoteCV, RandomNoteCVWidget>("RandomNoteCV");
