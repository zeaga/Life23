#pragma once
#include <vector>

enum WrapSetting {
	AlwaysOff,
	AlwaysOn,
	Wrap
};

typedef unsigned char Cell;

class Grid {
public:
	Grid( int width, int height );
	WrapSetting EdgeBehavior = Wrap;
	bool Neighborhood[8];
	char BirthRule[9];
	char SurviveRule[9];
	int GetWidth( );
	int GetHeight( );
	void Tick( );
	void Randomize( );
	void Randomize( float percent );
	void Fill( );
	void Clear( );
	Cell Get( int x, int y );
	void Set( int x, int y, Cell value );
	void Resize( int width, int height );
private:
	std::vector<Cell> Front;
	std::vector<Cell> Back;
	inline bool InGrid( int x, int y );
	int Convolute( int x, int y );
	int GetIdx( int x, int y );
	int GetX( int i );
	int GetY( int i );
	Cell GetBack( int x, int y );
	void SetBack( int x, int y, Cell value );
	int Width;
	int Height;
};