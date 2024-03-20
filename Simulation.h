// Simulation.h

#pragma once

#include <raylib.h>

#include "Grid.h"

class Simulation {
public:

#pragma region Simulation methods
	// Initialize simulation
	Simulation( int width, int height );

	// Reset all settings to default values
	void ResetToDefaults( );

	// Perform one tick of the simulation
	void Tick( );

	// Update keyboard input for simulation
	void UpdateKeyboard( );

	// Update mouse input for simulation
	void UpdateMouse( );

	// Update simulation state
	void Update( bool suppressKeyboardUpdate, bool suppressMouseUpdate );

	void DrawBrush( int x, int y, int size );

	// Draw the game grid and cells
	void Draw( bool showCursor );

	Grid& GetGrid( );
#pragma endregion

#pragma region Simulation variables
	int Width = 1600;					// Window width
	int Height = 900;					// Window height
	int BrushSize = 0;					// Size of the brush for drawing cells
	bool BrushRound = false;			// 
	bool Paused = false;				// Whether the simulation is paused

	int ActualTickRate{};
	Color AliveColor{};
	Color DeadColor{};

	int Scale{};
	int TicksPerSecond{};
	float PercentFilled{};
	int PanX{};
	int PanY{};

	bool EnableGrid{};
	bool RandomField{};
	bool RandomEdgeBehavior{};
	bool RandomColors{};
	bool RandomNeighbors{};
	bool RandomRules{};
	bool DisableStrobing{};
	int PreemptiveIterations{};

	bool UseMultithreading{};
#pragma endregion

private:
	void PlotLine( int x0, int y0, int x1, int y1, bool value, int size );
	void Plot( int x, int y, bool value, int size );
	void PlotSquare( int x, int y, bool value, int size );
	void PlotCircle( int x, int y, bool value, int size );
	Grid grid;
	double lastTick{};
	double tickTime{};
	int ticks{};
	double lastTickRateUpdate{};
	double totalTime{};
	double lastTime;
	float rawPanX{};
	float rawPanY{};
	int lastButton{};
	int lastX{};
	int lastY{};
};