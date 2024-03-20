// GuiManager.cpp

#include "GuiManager.h"

#include "Simulation.h"

#include <imgui.h>
#include <string>
#include "Grid.h"

// Convert from Raylib to ImGui color format
ImVec4 RlToImGuiColor( Color col ) {
	return {
		(float)col.r / 255.0f,
		(float)col.g / 255.0f,
		(float)col.b / 255.0f,
		1.0f,
	};
}

// Convert from ImGui to Raylib color format
Color ImGuiToRlColor( ImVec4 col ) {
	return {
		(unsigned char)( col.x * 255.0f ),
		(unsigned char)( col.y * 255.0f ),
		(unsigned char)( col.z * 255.0f ),
		255,
	};
}

GuiManager::GuiManager( Simulation& simulation ) {
	sim = &simulation;
	grid = &sim->GetGrid( );
}

void GuiManager::Draw( ) {
	ImGuiStyle style = ImGui::GetStyle( );
	ImGui::Begin( "Life23", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize );

	ImGui::LabelText( "FPS", std::to_string( GetFPS( ) ).c_str( ) );
	// TODO: Fix this
//	ImGui::LabelText( "TPS", std::to_string( sim->ActualTickRate ).c_str( ) );
	ImGui::InputInt( "Ticks per second", &sim->TicksPerSecond, 1, 10 );
	int newScale = sim->Scale;
	ImGui::InputInt( "Scale", &newScale, 1, 5 );
	if ( newScale < 1 )
		newScale = 1;
	if ( newScale > std::min( sim->Width, sim->Height ) )
		newScale = std::min( sim->Width, sim->Height );
	if ( sim->Scale != newScale ) {
		sim->Scale = newScale;
		grid->Resize( sim->Width / sim->Scale, sim->Height / sim->Scale );
	}
	ImGui::Checkbox( "Enable multithreading", &sim->UseMultithreading );
	ImGui::Checkbox( "Enable grid", &sim->EnableGrid );
	if ( ImGui::Button( "Reset all settings" ) )
		sim->ResetToDefaults( );
	ImGui::Separator( );

	{
		ImVec2 size = { ImGui::CalcTextSize( "Always off" ).x + style.ItemInnerSpacing.x * 2, 0 };
		switch ( grid->EdgeBehavior ) {
		case AlwaysOff:
			if ( ImGui::Button( "Always off", size ) )
				grid->EdgeBehavior = AlwaysOn;
			break;
		case AlwaysOn:
			if ( ImGui::Button( "Always on", size ) )
				grid->EdgeBehavior = Wrap;
			break;
		case Wrap:
			if ( ImGui::Button( "Wrap", size ) )
				grid->EdgeBehavior = AlwaysOff;
			break;
		default:
			break;
		}
	}

	{
		if ( ImGui::Button( sim->Paused ? "Play" : "Pause",
			{ ImGui::CalcTextSize( "Pause" ).x + style.ItemInnerSpacing.x * 2, 0 } ) )
			sim->Paused ^= true;
		if ( sim->Paused ) {
			ImGui::SameLine( );
			if ( ImGui::Button( "Tick" ) )
				grid->Tick( );
		}
		if ( ImGui::Button( "Clear" ) )
			grid->Clear( );
	}

	ImGui::Separator( );

	if ( ImGui::CollapsingHeader( "Randomizer" ) ) {
		bool random = ImGui::Button( "Randomize checked" );

		ImGui::Checkbox( "##Randomize field", &sim->RandomField );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize field" ) || ( random && sim->RandomField ) ) {
			grid->Randomize( sim->PercentFilled );
			for ( int i = 0; i < sim->PreemptiveIterations; i++ ) {
				grid->Tick( );
			}
		}

		ImGui::Checkbox( "##Randomize edge behavior", &sim->RandomEdgeBehavior );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize edge behavior" ) || ( random && sim->RandomEdgeBehavior ) ) {
			int edgeRand = rand( ) % 3;
			grid->EdgeBehavior = edgeRand == 0 ? AlwaysOff : edgeRand == 1 ? AlwaysOn : Wrap;
		}

		ImGui::Checkbox( "##Randomize colors", &sim->RandomColors );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize colors" ) || ( random && sim->RandomColors ) ) {
			sim->AliveColor.r = (unsigned char)( std::rand( ) % 255 );
			sim->AliveColor.g = (unsigned char)( std::rand( ) % 255 );
			sim->AliveColor.b = (unsigned char)( std::rand( ) % 255 );
			sim->DeadColor.r = ( sim->AliveColor.r + 127 ) % 255;
			sim->DeadColor.g = ( sim->AliveColor.g + 127 ) % 255;
			sim->DeadColor.b = ( sim->AliveColor.b + 127 ) % 255;
		}

		ImGui::Checkbox( "##Randomize neighbors", &sim->RandomNeighbors );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize neighbors" ) || ( random && sim->RandomNeighbors ) ) {
			for ( size_t i = 0; i < 8; i++ ) {
				grid->Neighborhood[i] = rand( ) % 2 == 0;
			}
		}

		ImGui::Checkbox( "##Randomize rules", &sim->RandomRules );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize rules" ) || ( random && sim->RandomRules ) ) {
			for ( size_t i = 0; i < 9; i++ ) {
				grid->BirthRule[i] = rand( ) % 2;
				grid->SurviveRule[i] = rand( ) % 2;
			}
			if ( sim->DisableStrobing )
				grid->BirthRule[0] = false;
		}

		if ( ImGui::CollapsingHeader( "Advanced" ) ) {
			float spacing = style.ItemInnerSpacing.x * 2 + 46;
			ImVec2 size = {
				ImGui::CalcTextSize( std::to_string( sim->PreemptiveIterations ).c_str( ) ).x + spacing,
				0
			};
			ImGui::PushItemWidth( size.x );
			ImGui::InputInt( "Preemptive iterations", &sim->PreemptiveIterations, 1, 10 );
			size.x = ImGui::CalcTextSize( std::to_string( sim->PercentFilled ).c_str( ) ).x + spacing;
			ImGui::PushItemWidth( size.x );
			ImGui::InputFloat( "Percent filled", &sim->PercentFilled, 0.05f, 0.2f );
			sim->PercentFilled = std::max( 0.0f, std::min( sim->PercentFilled, 1.0f ) );
			ImGui::Checkbox( "Disable strobing rules", &sim->DisableStrobing );
		}
		ImGui::Separator( );
	}

	if ( ImGui::CollapsingHeader( "Colors" ) ) {
		auto newAliveColor = RlToImGuiColor( sim->AliveColor );
		ImGui::ColorPicker3( "Alive color", (float*)&newAliveColor );
		sim->AliveColor = ImGuiToRlColor( newAliveColor );
		auto newDeadColor = RlToImGuiColor( sim->DeadColor );
		ImGui::ColorPicker3( "Dead color", (float*)&newDeadColor );
		sim->DeadColor = ImGuiToRlColor( newDeadColor );
		ImGui::Separator( );
	}

	if ( ImGui::CollapsingHeader( "Neighborhood" ) ) {
		const bool same_line[] = { false, true,	 true, false, true,  false, true, true };
		for ( size_t i = 0; i < 8; i++ ) {
			if ( same_line[i] )
				ImGui::SameLine( );
			if ( i == 4 )
				ImGui::SetCursorPosX( ImGui::GetCursorPosX( ) + ImGui::GetItemRectSize( ).x + style.CellPadding.x * 2 );
			std::string id = "##NH" + std::to_string( i );
			ImGui::Checkbox( id.c_str( ), &grid->Neighborhood[i] );
		}
		ImGui::Separator( );
	}

	if ( ImGui::CollapsingHeader( "Rules" ) ) {
		int neighborhoodSize = 0;
		for ( size_t i = 0; i < 8; i++ )
			neighborhoodSize += grid->Neighborhood[i];
		for ( size_t i = 0; i < 9; i++ ) {
			if ( i > neighborhoodSize )
				break;
			std::string id = std::to_string( i ) + "##BR" + std::to_string( i );
			ImGui::Checkbox( id.c_str( ), (bool*)&grid->BirthRule[i] );
			ImGui::SameLine( );
			id = "##SR" + std::to_string( i );
			ImGui::Checkbox( id.c_str( ), (bool*)&grid->SurviveRule[i] );
		}
		if ( ImGui::Button( "Reverse rule" ) ) {
			int mx = neighborhoodSize;
			int sz = mx + 1;
			int birth[9]{ };
			int survive[9]{ };
			for ( size_t i = 0; i < sz; i++ ) {
				birth[mx - i] = !grid->SurviveRule[i];
				survive[mx - i] = !grid->BirthRule[i];
			}
			for ( size_t i = 0; i < sz; i++ ) {
				grid->BirthRule[i] = birth[i];
				grid->SurviveRule[i] = survive[i];
			}
		}
		ImGui::Separator( );
	}

	ImGui::End( );
}