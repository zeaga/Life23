// GuiManager.h

#pragma once

#include "Simulation.h"
#include "Grid.h"

class GuiManager {
public:
	GuiManager( Simulation& simulation );

	// Draw user interface elements
	void Draw( );

private:
	Simulation* sim;
	Grid* grid;
};