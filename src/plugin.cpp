#include "plugin.hpp"

Plugin* plugin_instance;

void init(Plugin* p) {

	plugin_instance = p;

	p->addModel(model_BitMixer);
	p->addModel(model_PolyXform);
	p->addModel(model_Rotor);
}
