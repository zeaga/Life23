#include "Game.h"

#include "rlImGui.h"

#include <imgui.h>
#include <raylib.h>
#include <rlgl.h>

#include <string>
#include "Life23.h"

int width = 1600;
int height = 900;
int brushSize = 0;
bool paused = false;

Grid* grid = new Grid( 160, 90 );
int actualTickRate{ };
Color aliveColor = WHITE;
Color deadColor = BLACK;

int scale{ };
int ticksPerSecond{ };
float percentFilled{ };
int panX{ };
int panY{ };
bool enableGrid{ };
bool randomField{ };
bool randomEdgeBehavior{ };
bool randomColors{ };
bool randomNeighbors{ };
bool randomRules{ };
bool disableStrobing{ };
int preemptiveIterations{ };

inline int mod( int a, int b ) {
	return ( ( a % b ) + b ) % b;
}

ImVec4 RlToImGuiColor( Color col ) {
	return {
		(float)col.r / 255.0f,
		(float)col.g / 255.0f,
		(float)col.b / 255.0f,
		1.0f,
	};
}

Color ImGuiToRlColor( ImVec4 col ) {
	return {
		(unsigned char)( col.x * 255.0f ),
		(unsigned char)( col.y * 255.0f ),
		(unsigned char)( col.z * 255.0f ),
		255,
	};
}

void ResetToDefaults( ) {
	scale = 10;
	ticksPerSecond = 15;
	panX = 0;
	panY = 0;
	grid->Resize( width / scale, height / scale );
	grid->EdgeBehavior = Wrap;
	grid->Randomize( );
	aliveColor = WHITE;
	deadColor = BLACK;
	for ( size_t i = 0; i < 8; i++ ) {
		grid->Neighborhood[i] = true;
	}
	for ( size_t i = 0; i < 9; i++ ) {
		grid->BirthRule[i] = i == 3;
		grid->SurviveRule[i] = i == 2 || i == 3;
	}
	enableGrid = false;
	randomField = false;
	randomEdgeBehavior = false;
	randomColors = false;
	randomNeighbors = false;
	randomRules = false;
	enableGrid = false;
	percentFilled = 0.5f;
	disableStrobing = false;
	preemptiveIterations = 0;
}

void Start( ) {
	ResetToDefaults( );
}

void Tick( ) {
	grid->Tick( );
}

void UpdateInput( ) {
	if ( !ImGui::GetIO( ).WantCaptureKeyboard ) {
		if ( IsKeyDown( KEY_LEFT ) )
			panX--;
		if ( IsKeyDown( KEY_RIGHT ) )
			panX++;
		if ( IsKeyDown( KEY_UP ) )
			panY--;
		if ( IsKeyDown( KEY_DOWN ) )
			panY++;
		panX = mod( panX, grid->GetWidth( ) );
		panY = mod( panY, grid->GetHeight( ) );
	}
	if ( grid->EdgeBehavior != Wrap ) {
		panX = 0;
		panY = 0;
	}
	if ( ImGui::GetIO( ).WantCaptureMouse )
		return;
	brushSize += GetMouseWheelMove( );
	if ( brushSize < 0 )
		brushSize = 0;
	const int max = std::min( grid->GetWidth( ), grid->GetHeight( ) ) / 2;
	if ( brushSize > max )
		brushSize = max;
	int button = IsMouseButtonDown( MOUSE_BUTTON_LEFT ) ? 1 : IsMouseButtonDown( MOUSE_BUTTON_RIGHT ) ? 2 : 0;
	if ( button != 0 ) {
		float offset = brushSize % 2 ? 0 : 0.5f;
		float radius = brushSize / 2.0f;
		int ox = (int)( ( (float)GetMouseX( ) - offset ) / scale - radius );
		int oy = (int)( ( (float)GetMouseY( ) - offset ) / scale - radius );
		int size = ( 1 + radius * 2 );
		for ( int y = oy; y <= oy + brushSize; y++ ) {
			for ( int x = ox; x <= ox + brushSize; x++ ) {
				grid->Set( x, y, button == 1 );
			}
		}
	}
}

void Update( ) {
	if ( width != GetScreenWidth( ) || height != GetScreenHeight( ) ) {
		width = GetScreenWidth( );
		height = GetScreenHeight( );
		grid->Resize( width / scale, height / scale );
	}
	if ( IsKeyPressed( KEY_SPACE ) )
		paused ^= true;
	if ( paused && IsKeyPressed( KEY_F ) )
		Tick( );
	UpdateInput( );
}

void DrawGame( ) {
	for ( int y = 0; y < grid->GetHeight( ); y++ ) {
		for ( int x = 0; x < grid->GetWidth( ); x++ ) {
			if ( grid->Get( mod( x + panX, grid->GetWidth( ) ), mod( y + panY, grid->GetHeight( ) ) ) ) {
				DrawRectangle( x * scale, y * scale, scale, scale, aliveColor );
			}
		}
	}
	if ( enableGrid ) {
		for ( size_t y = 0; y < height; y += scale )
			DrawLine( 0, y, width, y, DARKGRAY );
		for ( size_t x = 0; x < width; x += scale )
			DrawLine( x, 0, x, height, DARKGRAY );
	}
	if ( paused )
		DrawRectangle( 0, 0, width, height, { 127, 127, 127, 127 } );
	int mX = GetMouseX( );
	int mY = GetMouseY( );
	bool outOfBounds = mX < 0 || mX >= width || mY < 0 || mY >= height;
	if ( ImGui::GetIO( ).WantCaptureMouse || outOfBounds ) {
		ShowCursor( );
	} else {
		HideCursor( );
		float offset = brushSize % 2 ? 0 : 0.5f;
		float radius = brushSize / 2.0f;
		int x = (int)( ( (float)mX - offset ) / scale - radius ) * scale;
		int y = (int)( ( (float)mY - offset ) / scale - radius ) * scale;
		int size = ( 1 + radius * 2 ) * scale;
		DrawRectangleLines( x, y, size, size, GRAY );
		if ( grid->EdgeBehavior == WrapSetting::Wrap ) {
			DrawRectangleLines( x - width, y - height, size, size, GRAY );
			DrawRectangleLines( x, y - height, size, size, GRAY );
			DrawRectangleLines( x + width, y - height, size, size, GRAY );
			DrawRectangleLines( x - width, y, size, size, GRAY );
			DrawRectangleLines( x + width, y, size, size, GRAY );
			DrawRectangleLines( x - width, y + height, size, size, GRAY );
			DrawRectangleLines( x, y + height, size, size, GRAY );
			DrawRectangleLines( x + width, y + height, size, size, GRAY );
		}
	}
}

void DrawUi( ) {
	ImGuiStyle style = ImGui::GetStyle( );
	ImGui::Begin( "Life23", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize );

	ImGui::LabelText( "FPS", std::to_string( GetFPS( ) ).c_str( ) );
	ImGui::LabelText( "TPS", std::to_string( actualTickRate ).c_str( ) );
	ImGui::InputInt( "Ticks per second", &ticksPerSecond, 1, 10 );
	int newScale = scale;
	ImGui::InputInt( "Scale", &newScale, 1, 5 );
	if ( newScale < 1 )
		newScale = 1;
	if ( newScale > std::min( width, height ) )
		newScale = std::min( width, height );
	if ( scale != newScale ) {
		scale = newScale;
		grid->Resize( width / scale, height / scale );
	}
	ImGui::Checkbox( "Enable grid", &enableGrid );
	if ( ImGui::Button( "Reset all settings" ) )
		ResetToDefaults( );
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
		if ( ImGui::Button( paused ? "Play" : "Pause",
			{ ImGui::CalcTextSize( "Pause" ).x + style.ItemInnerSpacing.x * 2, 0 } ) )
			paused ^= true;
		if ( paused ) {
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

		ImGui::Checkbox( "##Randomize field", &randomField );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize field" ) || ( random && randomField ) ) {
			grid->Randomize( percentFilled );
			for ( int i = 0; i < preemptiveIterations; i++ ) {
				grid->Tick( );
			}
		}

		ImGui::Checkbox( "##Randomize edge behavior", &randomEdgeBehavior );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize edge behavior" ) || ( random && randomEdgeBehavior ) ) {
			int edgeRand = rand( ) % 3;
			grid->EdgeBehavior = edgeRand == 0 ? AlwaysOff : edgeRand == 1 ? AlwaysOn : Wrap;
		}

		ImGui::Checkbox( "##Randomize colors", &randomColors );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize colors" ) || ( random && randomColors ) ) {
			aliveColor.r = (unsigned char)( std::rand( ) % 255 );
			aliveColor.g = (unsigned char)( std::rand( ) % 255 );
			aliveColor.b = (unsigned char)( std::rand( ) % 255 );
			deadColor.r = ( aliveColor.r + 127 ) % 255;
			deadColor.g = ( aliveColor.g + 127 ) % 255;
			deadColor.b = ( aliveColor.b + 127 ) % 255;
		}

		ImGui::Checkbox( "##Randomize neighbors", &randomNeighbors );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize neighbors" ) || ( random && randomNeighbors ) ) {
			for ( size_t i = 0; i < 8; i++ ) {
				grid->Neighborhood[i] = rand( ) % 2 == 0;
			}
		}

		ImGui::Checkbox( "##Randomize rules", &randomRules );
		ImGui::SameLine( );
		if ( ImGui::Button( "Randomize rules" ) || ( random && randomRules ) ) {
			for ( size_t i = 0; i < 9; i++ ) {
				grid->BirthRule[i] = rand( ) % 2;
				grid->SurviveRule[i] = rand( ) % 2;
			}
			if ( disableStrobing )
				grid->BirthRule[0] = false;
		}

		if ( ImGui::CollapsingHeader( "Advanced" ) ) {
			float spacing = style.ItemInnerSpacing.x * 2 + 46;
			ImVec2 size = {
				ImGui::CalcTextSize( std::to_string( preemptiveIterations ).c_str( ) ).x + spacing,
				0
			};
			ImGui::PushItemWidth( size.x );
			ImGui::InputInt( "Preemptive iterations", &preemptiveIterations, 1, 10 );
			size.x = ImGui::CalcTextSize( std::to_string( percentFilled ).c_str( ) ).x + spacing;
			ImGui::PushItemWidth( size.x );
			ImGui::InputFloat( "Percent filled", &percentFilled, 0.05f, 0.2f );
			percentFilled = std::max( 0.0f, std::min( percentFilled, 1.0f ) );
			ImGui::Checkbox( "Disable strobing rules", &disableStrobing );
		}
		ImGui::Separator( );
	}

	if ( ImGui::CollapsingHeader( "Colors" ) ) {
		auto newAliveColor = RlToImGuiColor( aliveColor );
		ImGui::ColorPicker3( "Alive color", (float*)&newAliveColor );
		aliveColor = ImGuiToRlColor( newAliveColor );
		auto newDeadColor = RlToImGuiColor( deadColor );
		ImGui::ColorPicker3( "Dead color", (float*)&newDeadColor );
		deadColor = ImGuiToRlColor( newDeadColor );
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

void Draw( ) {
	ClearBackground( deadColor );
	DrawGame( );
	DrawUi( );
}

void Close( ) { }

void Run( ) {
	SetConfigFlags( FLAG_WINDOW_RESIZABLE );
	InitWindow( width, height, "Life23" );
	// SetTargetFPS( 60 );
	Start( );
	double lastTick = GetTime( );
	double tickTime;
	int ticks = 0;
	int lastTickRateUpdate = 0;
	double totalTime = 0;
	double lastTime = 0;
	rlImGuiSetup( true );
	while ( !WindowShouldClose( ) ) {
		Update( );
		tickTime = 1.0 / ticksPerSecond;
		if ( !paused ) {
			totalTime += GetFrameTime( );
			if ( lastTick + tickTime <= totalTime ) {
				lastTick = totalTime;
				Tick( );
				ticks++;
			}
			if ( (int)totalTime > lastTickRateUpdate ) {
				actualTickRate = ticks;
				lastTickRateUpdate = totalTime;
				ticks = 0;
			}
		}
		BeginDrawing( );
		rlImGuiBegin( );
		Draw( );
		rlImGuiEnd( );
		DrawCircle( GetMouseX( ) - 4, GetMouseY( ) - 4, 4, { 255, 0, 255, 255 } );
		EndDrawing( );
	}
	rlImGuiShutdown( );
	Close( );
	CloseWindow( );
}

int main( int argc, char* argv[] ) {
	Run( );
	return 0;
}