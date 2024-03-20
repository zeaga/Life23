// Simulation.cpp

// Computes the modulus operation with a positive result even for negative numbers.
#define MOD_POSITIVE(a,b) (((a)%(b))+(b))%(b)

#include "Simulation.h"

Simulation::Simulation( int width, int height ) : grid( width, height ) {
	ResetToDefaults( );
	lastTick = GetTime( );
}

void Simulation::ResetToDefaults( ) {
	Scale = 10;
	TicksPerSecond = 15;
	PanX = 0;
	PanY = 0;
	grid.Resize( Width / Scale, Height / Scale );
	grid.EdgeBehavior = Wrap;
	grid.Randomize( );
	AliveColor = WHITE;
	DeadColor = BLACK;
	for ( size_t i = 0; i < 8; i++ ) {
		grid.Neighborhood[i] = true;
	}
	for ( size_t i = 0; i < 9; i++ ) {
		grid.BirthRule[i] = i == 3;
		grid.SurviveRule[i] = i == 2 || i == 3;
	}
	EnableGrid = false;
	RandomField = false;
	RandomEdgeBehavior = false;
	RandomColors = false;
	RandomNeighbors = false;
	RandomRules = false;
	EnableGrid = false;
	PercentFilled = 0.5f;
	DisableStrobing = false;
	PreemptiveIterations = 0;
}

void Simulation::Tick( ) {
	grid.Tick( );
}

void Simulation::UpdateKeyboard( ) {
	if ( grid.EdgeBehavior != Wrap ) {
		rawPanX = 0;
		rawPanY = 0;
		PanX = 0;
		PanY = 0;
		return;
	}
	const float panSpeed = 60.0 * GetFrameTime( );
	if ( IsKeyDown( KEY_LEFT ) )
		rawPanX -= panSpeed;
	if ( IsKeyDown( KEY_RIGHT ) )
		rawPanX += panSpeed;
	if ( IsKeyDown( KEY_UP ) )
		rawPanY -= panSpeed;
	if ( IsKeyDown( KEY_DOWN ) )
		rawPanY += panSpeed;
	PanX = MOD_POSITIVE( (int)rawPanX, grid.GetWidth( ) );
	PanY = MOD_POSITIVE( (int)rawPanY, grid.GetHeight( ) );
}

void Simulation::PlotLine( int x0, int y0, int x1, int y1, bool value, int size = 1 ) {
	float dx = abs( x1 - x0 );
	float sx = x0 < x1 ? 1 : -1;
	float dy = -abs( y1 - y0 );
	float sy = y0 < y1 ? 1 : -1;
	float error = dx + dy;
	while ( true ) {
		Plot( x0, y0, value, size );
		if ( x0 == x1 and y0 == y1 ) break;
		float e2 = 2 * error;
		if ( e2 >= dy ) {
			if ( x0 == x1 ) break;
			error += dy;
			x0 += sx;
		}
		if ( e2 <= dx ) {
			if ( y0 == y1 ) break;
			error += dx;
			y0 += sy;
		}
	}
}

void Simulation::Plot( int x, int y, bool value, int size = 1 ) {
	if ( BrushRound )
		PlotCircle( x, y, value, size );
	else
		PlotSquare( x, y, value, size );
}

void Simulation::PlotSquare( int x, int y, bool value, int size = 1 ) {
	if ( size <= 1 ) {
		grid.Set( x, y, value );
		return;
	}
	for ( int cy = y; cy <= y + size; cy++ ) {
		for ( int cx = x; cx <= x + size; cx++ ) {
			grid.Set( cx, cy, value );
		}
	}
}

void Simulation::PlotCircle( int x, int y, bool value, int size = 1 ) {
	if ( size <= 1 ) {
		grid.Set( x, y, value );
		return;
	}
	float sizeSq = size * size * 0.25;
	for ( int cy = y; cy <= y + size; cy++ ) {
		int py = abs( cy - y - size / 2 );
		for ( int cx = x; cx <= x + size; cx++ ) {
			int px = abs( cx - x - size / 2 );
			if ( px * px + py * py <= sizeSq )
				grid.Set( cx, cy, value );
		}
	}
}

void Simulation::UpdateMouse( ) {
	BrushRound ^= IsMouseButtonPressed( MOUSE_BUTTON_MIDDLE );
	BrushSize += (int)GetMouseWheelMove( );
	if ( BrushSize < 0 )
		BrushSize = 0;
	const int max = std::min( grid.GetWidth( ), grid.GetHeight( ) ) / 2;
	if ( BrushSize > max )
		BrushSize = max;
	int button = IsMouseButtonDown( MOUSE_BUTTON_LEFT ) ? MOUSE_BUTTON_LEFT : IsMouseButtonDown( MOUSE_BUTTON_RIGHT ) ? MOUSE_BUTTON_RIGHT : -1;
	bool dragging = lastButton == button;
	lastButton = button;
	if ( button < 0 ) return;
	float offset = BrushSize % 2 ? 0 : 0.5f;
	float radius = BrushSize / 2.0f;
	int ox = (int)( ( (float)GetMouseX( ) - offset ) / Scale - radius );
	int oy = (int)( ( (float)GetMouseY( ) - offset ) / Scale - radius );
	ox += PanX;
	oy += PanY;
	bool value = button == MOUSE_BUTTON_LEFT;
	if ( dragging ) {
		PlotLine( lastX, lastY, ox, oy, value, BrushSize );
	} else {
		Plot( ox, oy, value, BrushSize );
	}
	lastX = ox;
	lastY = oy;
}

void Simulation::Update( bool suppressKeyboardUpdate = false, bool suppressMouseUpdate = false ) {
	tickTime = 1.0 / TicksPerSecond;
	if ( !Paused ) {
		totalTime += GetFrameTime( );
		if ( lastTick + tickTime <= totalTime ) {
			lastTick = totalTime;
			Tick( );
			ticks++;
		}
		if ( totalTime > lastTickRateUpdate ) {
			ActualTickRate = ticks;
			lastTickRateUpdate = totalTime;
			ticks = 0;
		}
	}
	if ( Width != GetScreenWidth( ) || Height != GetScreenHeight( ) ) {
		Width = GetScreenWidth( );
		Height = GetScreenHeight( );
		grid.Resize( Width / Scale, Height / Scale );
	}
	if ( IsKeyPressed( KEY_SPACE ) ) Paused ^= true;
	if ( Paused && IsKeyPressed( KEY_F ) ) Tick( );
	if ( !suppressKeyboardUpdate ) UpdateKeyboard( );
	if ( !suppressMouseUpdate ) UpdateMouse( );
	// if ( !ImGui::GetIO( ).WantCaptureKeyboard )
	// if ( ImGui::GetIO( ).WantCaptureMouse )
}

void Simulation::DrawBrush( int x, int y, int size ) {
	if ( BrushRound ) {
		DrawCircleLines( x + size / 2, y + size / 2, (float)size / 2, GRAY );
	} else {
		DrawRectangleLines( x, y, size, size, GRAY );
	}

}

void Simulation::Draw( bool showCursor = false ) {
	for ( int y = 0; y < grid.GetHeight( ); y++ ) {
		for ( int x = 0; x < grid.GetWidth( ); x++ ) {
			if ( grid.Get( MOD_POSITIVE( x + PanX, grid.GetWidth( ) ), MOD_POSITIVE( y + PanY, grid.GetHeight( ) ) ) ) {
				DrawRectangle( x * Scale, y * Scale, Scale, Scale, AliveColor );
			}
		}
	}
	if ( EnableGrid ) {
		for ( int y = 0; y < Height; y += Scale )
			DrawLine( 0, y, Width, y, DARKGRAY );
		for ( int x = 0; x < Width; x += Scale )
			DrawLine( x, 0, x, Height, DARKGRAY );
	}
	if ( Paused )
		DrawRectangle( 0, 0, Width, Height, { 127, 127, 127, 127 } );
	int mX = GetMouseX( );
	int mY = GetMouseY( );
	bool outOfBounds = mX < 0 || mX >= Width || mY < 0 || mY >= Height;
	if ( showCursor || outOfBounds ) {
		ShowCursor( );
	} else {
		HideCursor( );
		float offset = BrushSize % 2 ? 0 : 0.5f;
		float radius = BrushSize / 2.0f;
		int x = (int)( ( (float)mX - offset ) / Scale - radius ) * Scale;
		int y = (int)( ( (float)mY - offset ) / Scale - radius ) * Scale;
		int size = (int)( 1 + radius * 2 ) * Scale;
		DrawBrush( x, y, size );
		if ( grid.EdgeBehavior == WrapSetting::Wrap ) {
			DrawBrush( x - Width, y - Height, size );
			DrawBrush( x, y - Height, size );
			DrawBrush( x + Width, y - Height, size );
			DrawBrush( x - Width, y, size );
			DrawBrush( x + Width, y, size );
			DrawBrush( x - Width, y + Height, size );
			DrawBrush( x, y + Height, size );
			DrawBrush( x + Width, y + Height, size );
		}
	}
}

Grid& Simulation::GetGrid( ) {
	return grid;
}