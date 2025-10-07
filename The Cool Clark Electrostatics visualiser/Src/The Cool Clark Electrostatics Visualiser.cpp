#include "Core.h"
#include "utils.h"
#include <iostream>


int main() {
	// boot up the core
	Core core(800, 600, "The Very Cool Clark Electrostatics Visualiser");

	core.SetUp();
	core.MainLoop();

	return 0;
}