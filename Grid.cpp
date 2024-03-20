// Grid.cpp

#include "Grid.h"

#include <thread>
#include <iostream>

Grid::Grid( int width, int height ) {
	for ( size_t i = 0; i < 8; i++ )
		Neighborhood[i] = true;
	BirthRule[3] = 1;
	SurviveRule[3] = 1;
	SurviveRule[2] = 1;
	Width = width;
	Height = height;
	Front.resize( static_cast<std::vector<Cell, std::allocator<Cell>>::size_type>( width ) * height );
	Back.resize( Front.size( ) );
}

int Grid::GetWidth( ) {
	return Width;
}

int Grid::GetHeight( ) {
	return Height;
}

void Grid::Resize( int newWidth, int newHeight ) {
	if ( Width == newWidth && Height == newHeight )
		return;
	/*	
	int cloneWidth = std::min( Width, newWidth );
	int cloneHeight = std::min( Height, newHeight );
	std::vector<Cell> clone = std::vector<Cell>( cloneWidth * cloneHeight );
	for ( size_t y = 0; y < cloneHeight; y++ ) {
		for ( size_t x = 0; x < cloneWidth; x++ ) {
			clone[y * cloneWidth + x] = Get( x, y );
		}
	}
	*/	
	Width = newWidth;
	Height = newHeight;
	Front.resize( static_cast<std::vector<Cell, std::allocator<Cell>>::size_type>( newWidth ) * newHeight, 0 );
	Back.resize( Front.size( ), 0 );
	Clear( );
	/*
	for ( size_t y = 0; y < cloneHeight; y++ ) {
		for ( size_t x = 0; x < cloneWidth; x++ ) {
			Set( x, y, clone[y * cloneWidth + x] );
		}
	}
	*/
}

inline bool Grid::InGrid( int x, int y ) {
	return ( x >= 0 ) && ( y >= 0 ) && ( x < Width ) && ( y < Height );
}

int Grid::Convolute( int x, int y ) {
	int sum = 0;
	const int x_lookup[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int y_lookup[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	for ( size_t i = 0; i < 8; i++ ) {
		if ( !Neighborhood[i] ) continue;
		int dx = x_lookup[i];
		int dy = y_lookup[i];
		int xx = x + dx;
		int yy = y + dy;

		// Boundary check
		if ( InGrid( xx, yy ) ) {
			// Avoid function call to Get, directly access Front vector
			sum += Front[GetIdx( xx, yy )];
		} else if ( EdgeBehavior == Wrap ) {
			// If wrapping is enabled, adjust coordinates accordingly
			sum += Front[GetIdx( MOD_POSITIVE( xx, Width ), MOD_POSITIVE( yy, Height ) )];
		}
	}
	return sum;
}

void Grid::TickWithMultithreading( ) {
	const int numThreads = std::thread::hardware_concurrency( );
	const int chunkSize = Height / numThreads;

	std::vector<std::thread> threads( numThreads );

	for ( int i = 0; i < numThreads; i++ ) {
		threads[i] = std::thread( [=]( ) {
			// Compute start and end rows for this thread
			int startRow = i * chunkSize;
			int endRow = ( i == numThreads - 1 ) ? Height : ( i + 1 ) * chunkSize;
			// Update cells for this portion of the grid
			for ( int y = startRow; y < endRow; y++ ) {
				for ( int x = 0; x < Width; x++ ) {
					int c = Convolute( x, y );
					int i = y * Width + x;
					Back[i] = Front[i] ? SurviveRule[c] : BirthRule[c];
				}
			}
		} );
	}

	for ( auto& thread : threads ) thread.join( );

	std::swap( Front, Back );
}

void Grid::Tick( ) {
	for ( size_t i = 0; i < Front.size( ); i++ ) {
		int c = Convolute( GetX( i ), GetY( i ) );
		Back[i] = Front[i] ? SurviveRule[c] : BirthRule[c];
	}
	std::swap( Front, Back );
}

void Grid::Randomize( ) {
	for ( size_t i = 0; i < Front.size( ); i++ ) {
		Front[i] = rand( ) % 2;
	}
}

void Grid::Randomize( float percent = 0.5f ) {
	for ( size_t i = 0; i < Front.size( ); i++ ) {
		Front[i] = static_cast <float> ( rand( ) ) / static_cast <float> ( RAND_MAX ) < percent;
	}
}

void Grid::Clear( ) {
	for ( size_t i = 0; i < Front.size( ); i++ ) {
		Front[i] = false;
	}
}

void Grid::Fill( ) {
	for ( size_t i = 0; i < Front.size( ); i++ ) {
		Front[i] = true;
	}
}

inline int Grid::GetIdx( int x, int y ) {
	return y * Width + x;
}

inline int Grid::GetX( int i ) {
	return i % Width;
}


inline int Grid::GetY( int i ) {
	return i / Width;
}


Cell Grid::Get( int x, int y ) {
	if ( InGrid( x, y ) || EdgeBehavior == Wrap ) {
		// If the cell is within the grid or wrapping is enabled, return the cell value
		return Front[GetIdx( MOD_POSITIVE( x, Width ), MOD_POSITIVE( y, Height ) )];
	} else {
		// If the cell is outside the grid and wrapping is disabled, return the default cell value (AlwaysOn)
		return EdgeBehavior == AlwaysOn ? 1 : 0; // Assuming 1 represents "Alive" and 0 represents "Dead"
	}
}


Cell Grid::GetBack( int x, int y ) {
	return ( InGrid( x, y ) || EdgeBehavior == Wrap ) ? Back[GetIdx( MOD_POSITIVE( x, Width ), MOD_POSITIVE( y, Height ) )] : EdgeBehavior == AlwaysOn;
}

void Grid::Set( int x, int y, Cell value ) {
	if ( InGrid( x, y ) || EdgeBehavior == Wrap )
		Front[GetIdx( MOD_POSITIVE( x, Width ), MOD_POSITIVE( y, Height ) )] = value;
}

void Grid::SetBack( int x, int y, Cell value ) {
	if ( InGrid( x, y ) || EdgeBehavior == Wrap )
		Back[GetIdx( MOD_POSITIVE( x, Width ), MOD_POSITIVE( y, Height ) )] = value;
}