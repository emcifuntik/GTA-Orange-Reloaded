//==============================================================================
//
// File: CVector3.h
// Project: Shared
// Author(s): Multi Theft Auto Team
//
//==============================================================================

#pragma once
#include <cmath>

#include <xmmintrin.h>
inline void vmul_sse(const float *a, const float b, float *r)
{
	_mm_storeu_ps(r, _mm_mul_ps(_mm_loadu_ps(a), _mm_set1_ps(b)));
}
inline void vdiv_sse(const float *a, const float b, float *r)
{
	_mm_storeu_ps(r, _mm_div_ps(_mm_loadu_ps(a), _mm_set1_ps(b)));
}
inline void vadd_sse(const float *a, const float *b, float *r)
{
	_mm_storeu_ps(r, _mm_add_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}
inline void vsub_sse(const float *a, const float *b, float *r)
{
	_mm_storeu_ps(r, _mm_sub_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}
inline void vmodmul_sse(const float *a, const float *b, float *r)
{
	_mm_storeu_ps(r, _mm_mul_ps(_mm_loadu_ps(a), _mm_loadu_ps(b)));
}
inline void vclamp_sse(const float *a, float *min, float *max, float *r)
{
	_mm_storeu_ps(r, _mm_max_ps(_mm_loadu_ps(min), _mm_min_ps(_mm_loadu_ps(max), _mm_loadu_ps(a))));
}

class CVector3
{
public:
	float fX;
	float fY;
	float fZ;

	CVector3()
	{
		fX = fY = fZ = 0.0f;
	}

	CVector3(float _fX, float _fY, float _fZ)
	{
		fX = _fX; fY = _fY; fZ = _fZ;
	}

	bool IsEmpty() const
	{
		return (fX == 0 && fY == 0 && fZ == 0);
	}

	float Length() const
	{
		return sqrt((fX * fX) + (fY * fY) + (fZ * fZ));
	}

	void Normalize()
	{
		float length = Length();
		fX /= length;
		fY /= length;
		fZ /= length;
	}

	std::string ToString()
	{
		std::stringstream ss;
		ss << "X = " << fX << ", Y = " << fY << ", Z = " << fZ;
		return ss.str();
	}

	static CVector3 Add(CVector3 left, CVector3 right)
	{
		vadd_sse((float*)&left, (float*)&right, (float*)&left);
		return left;
	}
	static CVector3 Subtract(CVector3 left, CVector3 right)
	{
		vadd_sse((float*)&left, (float*)&right, (float*)&left);
		return left;
	}
	static CVector3 Modulate(CVector3 left, CVector3 right)
	{
		vadd_sse((float*)&left, (float*)&right, (float*)&left);
		return left;
	}
	static CVector3 Multiply(CVector3 value, float scale)
	{
		vdiv_sse((float*)&value, scale, (float*)&value);
		return value;
	}
	static CVector3 Divide(CVector3 value, float scale)
	{
		vdiv_sse((float*)&value, scale, (float*)&value);
		return value;
	}

	CVector3 operator+ (const CVector3& vecRight) const
	{

		return CVector3(fX + vecRight.fX, fY + vecRight.fY, fZ + vecRight.fZ);
	}

	CVector3 operator+ (float fRight) const
	{
		return CVector3(fX + fRight, fY + fRight, fZ + fRight);
	}

	CVector3 operator- (const CVector3& vecRight) const
	{
		return CVector3(fX - vecRight.fX, fY - vecRight.fY, fZ - vecRight.fZ);
	}

	CVector3 operator- (float fRight) const
	{
		return CVector3(fX - fRight, fY - fRight, fZ - fRight);
	}

	CVector3 operator* (const CVector3& vecRight) const
	{
		return CVector3(fX * vecRight.fX, fY * vecRight.fY, fZ * vecRight.fZ);
	}

	CVector3 operator* (float fRight) const
	{
		return CVector3(fX * fRight, fY * fRight, fZ * fRight);
	}

	CVector3 operator/ (const CVector3& vecRight) const
	{
		return CVector3(fX / vecRight.fX, fY / vecRight.fY, fZ / vecRight.fZ);
	}

	CVector3 operator/ (float fRight) const
	{
		return CVector3(fX / fRight, fY / fRight, fZ / fRight);
	}

	CVector3 operator - () const
	{
		return CVector3(-fX, -fY, -fZ);
	}

	void operator += (float fRight)
	{
		fX += fRight;
		fY += fRight;
		fZ += fRight;
	}

	void operator -= (float fRight)
	{
		fX -= fRight;
		fY -= fRight;
		fZ -= fRight;
	}

	void operator *= (float fRight)
	{
		fX *= fRight;
		fY *= fRight;
		fZ *= fRight;
	}

	void operator /= (float fRight)
	{
		fX /= fRight;
		fY /= fRight;
		fZ /= fRight;
	}
};
