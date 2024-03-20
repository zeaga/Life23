// Life23.cpp

#include "Grid.h"

#include "rlImGui.h"

#include <imgui.h>
#include <raylib.h>
#include <rlgl.h>

#include <string>

#include "Simulation.h"
#include "GuiManager.h"

int main( int argc, char* argv[] ) {
	SetConfigFlags( FLAG_WINDOW_RESIZABLE );
	InitWindow( 1600, 900, "Life23" );
	// SetTargetFPS( 60 );
	Simulation sim( 160, 90 );
	GuiManager gui( sim );
	rlImGuiSetup( true );
	while ( !WindowShouldClose( ) ) {
		sim.Update( ImGui::GetIO( ).WantCaptureKeyboard, ImGui::GetIO( ).WantCaptureMouse );
		BeginDrawing( );
		rlImGuiBegin( );
		ClearBackground( BLACK );
		sim.Draw( ImGui::GetIO( ).WantCaptureMouse );
		gui.Draw( );
		rlImGuiEnd( );
		DrawCircle( GetMouseX( ), GetMouseY( ), 4, { 255, 255, 255, 255 } );
		DrawCircle( GetMouseX( ), GetMouseY( ), 3, { 255, 0, 255, 255 } );
		EndDrawing( );
	}
	rlImGuiShutdown( );
	CloseWindow( );
	return 0;
}