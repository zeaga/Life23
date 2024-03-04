#include "Grid.h"

Grid::Grid( int width, int height ) {
	for ( size_t i = 0; i < 8; i++ )
		this->Neighborhood[i] = true;
	this->BirthRule[3] = 1;
	this->SurviveRule[3] = 1;
	this->SurviveRule[2] = 1;
	this->Width = width;
	this->Height = height;
	this->Front.resize( static_cast<std::vector<Cell, std::allocator<Cell>>::size_type>( width ) * height );
	this->Back.resize( this->Front.size( ) );
}

int Grid::GetWidth( ) {
	return this->Width;
}

int Grid::GetHeight( ) {
	return this->Height;
}

void Grid::Resize( int newWidth, int newHeight ) {
	int cloneWidth = std::min( this->Width, newWidth );
	int cloneHeight = std::min( this->Height, newHeight );
	std::vector<Cell> clone = std::vector<Cell>( cloneWidth * cloneHeight );
	for ( size_t y = 0; y < cloneHeight; y++ ) {
		for ( size_t x = 0; x < cloneWidth; x++ ) {
			clone[y * cloneWidth + x] = this->Get( x, y );
		}
	}
	this->Width = newWidth;
	this->Height = newHeight;
	this->Front.resize( static_cast<std::vector<Cell, std::allocator<Cell>>::size_type>( newWidth ) * newHeight, 0 );
	this->Back.resize( this->Front.size( ), 0 );
	this->Clear( );
	for ( size_t y = 0; y < cloneHeight; y++ ) {
		for ( size_t x = 0; x < cloneWidth; x++ ) {
			Set( x, y, clone[y * cloneWidth + x] );
		}
	}
}

inline bool Grid::InGrid( int x, int y ) {
	return ( x >= 0 ) && ( y >= 0 ) && ( x < this->Width ) && ( y < this->Height );
}

int Grid::Convolute( int x, int y ) {
	int sum = 0;
	const int x_lookup[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	const int y_lookup[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	for ( size_t i = 0; i < 8; i++ ) {
		int dx = x_lookup[i];
		int dy = y_lookup[i];
		int xx = x + dx;
		int yy = y + dy;
		if ( this->Neighborhood[i] && this->Get( xx, yy ) )
			sum++;
	}
	return sum;
}

void Grid::Tick( ) {
	for ( size_t i = 0; i < this->Front.size( ); i++ ) {
		int c = this->Convolute( this->GetX( i ), this->GetY( i ) );
		this->Back[i] = this->Front[i] ? this->SurviveRule[c] : BirthRule[c];
	}
	std::swap( this->Front, this->Back );
}

void Grid::Randomize( ) {
	for ( size_t i = 0; i < this->Front.size( ); i++ ) {
		this->Front[i] = rand( ) % 2;
	}
}

void Grid::Randomize( float percent = 0.5f ) {
	for ( size_t i = 0; i < this->Front.size( ); i++ ) {
		this->Front[i] = static_cast <float> ( rand( ) ) / static_cast <float> ( RAND_MAX ) < percent;
	}
}

void Grid::Clear( ) {
	for ( size_t i = 0; i < this->Front.size( ); i++ ) {
		this->Front[i] = false;
	}
}

void Grid::Fill( ) {
	for ( size_t i = 0; i < this->Front.size( ); i++ ) {
		this->Front[i] = true;
	}
}

inline int Grid::GetIdx( int x, int y ) {
	return y * this->Width + x;
}

inline int Grid::GetX( int i ) {
	return i % this->Width;
}


inline int Grid::GetY( int i ) {
	return i / this->Width;
}

inline int mod( int a, int b ) {
	return ( ( a % b ) + b ) % b;
}


Cell Grid::Get( int x, int y ) {
	return ( this->InGrid( x, y ) || this->EdgeBehavior == Wrap ) ? this->Front[GetIdx( mod( x, this->Width ), mod( y, this->Height ) )] : this->EdgeBehavior == AlwaysOn;
}


Cell Grid::GetBack( int x, int y ) {
	return ( this->InGrid( x, y ) || this->EdgeBehavior == Wrap ) ? this->Back[GetIdx( mod( x, this->Width ), mod( y, this->Height ) )] : this->EdgeBehavior == AlwaysOn;
}

void Grid::Set( int x, int y, Cell value ) {
	if ( this->InGrid( x, y ) || this->EdgeBehavior == Wrap )
		this->Front[this->GetIdx( mod( x, this->Width ), mod( y, this->Height ) )] = value;
}

void Grid::SetBack( int x, int y, Cell value ) {
	if ( this->InGrid( x, y ) || this->EdgeBehavior == Wrap )
		this->Back[this->GetIdx( mod( x, this->Width ), mod( y, this->Height ) )] = value;
}