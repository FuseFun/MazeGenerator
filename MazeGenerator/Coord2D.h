#pragma once

struct FCoord2D
{
	int X;
	int Y;

	FCoord2D();
	FCoord2D(int InX, int InY);

	const int& operator()(int PointIndex) const;
	int& operator()(int PointIndex);
	bool operator==(const FCoord2D& Other) const;
	bool operator!=(const FCoord2D& Other) const;
	FCoord2D& operator*=(int Scale);
	FCoord2D& operator/=(int Divisor);
	FCoord2D& operator+=(const FCoord2D& Other);
	FCoord2D& operator-=(const FCoord2D& Other);
	FCoord2D& operator/=(const FCoord2D& Other);
	FCoord2D& operator=(const FCoord2D& Other);
	FCoord2D operator*(int Scale) const;
	FCoord2D operator/(int Divisor) const;
	FCoord2D operator+(const FCoord2D& Other) const;
	FCoord2D operator-(const FCoord2D& Other) const;
	FCoord2D operator/(const FCoord2D& Other) const;
};


inline FCoord2D::FCoord2D() {}
inline FCoord2D::FCoord2D(int InX, int InY) : X(InX), Y(InY) {}

inline const int& FCoord2D::operator()(int PointIndex) const
{
	return (&X)[PointIndex];
}

inline int& FCoord2D::operator()(int PointIndex)
{
	return (&X)[PointIndex];
}

inline bool FCoord2D::operator==(const FCoord2D& Other) const
{
	return X == Other.X && Y == Other.Y;
}

inline bool FCoord2D::operator!=(const FCoord2D& Other) const
{
	return (X != Other.X) || (Y != Other.Y);
}

inline FCoord2D& FCoord2D::operator*=(int Scale)
{
	X *= Scale;
	Y *= Scale;
	return *this;
}

inline FCoord2D& FCoord2D::operator/=(int Divisor)
{
	X /= Divisor;
	Y /= Divisor;
	return *this;
}

inline FCoord2D& FCoord2D::operator+=(const FCoord2D& Other)
{
	X += Other.X;
	Y += Other.Y;
	return *this;
}

inline FCoord2D& FCoord2D::operator-=(const FCoord2D& Other)
{
	X -= Other.X;
	Y -= Other.Y;
	return *this;
}

inline FCoord2D& FCoord2D::operator/=(const FCoord2D& Other)
{
	X /= Other.X;
	Y /= Other.Y;
	return *this;
}

inline FCoord2D& FCoord2D::operator=(const FCoord2D& Other)
{
	X = Other.X;
	Y = Other.Y;
	return *this;
}

inline FCoord2D FCoord2D::operator*(int Scale) const
{
	return FCoord2D(*this) *= Scale;
}

inline FCoord2D FCoord2D::operator/(int Divisor) const
{
	return FCoord2D(*this) /= Divisor;
}

inline FCoord2D FCoord2D::operator+(const FCoord2D& Other) const
{
	return FCoord2D(*this) += Other;
}

inline FCoord2D FCoord2D::operator-(const FCoord2D& Other) const
{
	return FCoord2D(*this) -= Other;
}

inline FCoord2D FCoord2D::operator/(const FCoord2D& Other) const
{
	return FCoord2D(*this) /= Other;
}
