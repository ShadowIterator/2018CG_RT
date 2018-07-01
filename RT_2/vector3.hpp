#ifndef Vector_hpp
#define Vector_hpp

#include "Global.h"

template<class T>
class Vector3
{
public:
	T x;
	T y;
	T z;

public:
	Vector3() :x(0), y(0), z(0)
	{
	}
	Vector3(const T& tx, const T& ty, const T& tz) :x(tx), y(ty), z(tz)
	{
	}
	Vector3(const Vector3& tv) :x(tv.x), y(tv.y), z(tv.z)
	{
	}
	void modify(const T& tx, const T& ty, const T& tz)
	{
		x = tx; y = ty; z = tz;
	}
	void modify(const Vector3& tv)
	{
		x = tv.x; y = tv.y; z = tv.z;
	}
	T len() const
	{
		return std::sqrt(x * x + y * y + z * z);
	}
	T len2() const
	{
		return (x * x + y * y + z * z);
	}
	Vector3 operator + (const Vector3& tv) const
	{
		return Vector3(x + tv.x, y + tv.y, z + tv.z);
	}
	Vector3 operator - (const Vector3& tv) const
	{
		return Vector3(x - tv.x, y - tv.y, z - tv.z);
	}
	Vector3 operator * (const T& a) const
	{
		return Vector3(a * x, a * y, a * z);
	}
	T operator % (const Vector3& tv) const	//dot
	{
		return  x * tv.x + y * tv.y + z * tv.z;
	}
	Vector3 operator * (const Vector3& tv) const	//cross
	{
		return Vector3(y * tv.z - z * tv.y, z * tv.x - x * tv.z, x * tv.y - y * tv.x);
	}
	Vector3 operator / (const T& a) const
	{
		return Vector3(x / a, y / a, z / a);
	}
	Vector3 normalize() const
	{
		T L = len();
		return Vector3(x / L, y / L, z / L);
	}
	Vector3 reLength(const T& k) const
	{
		T L = len();
		return Vector3(x * k / L, y * k / L, z * k / L);
	}
	void _normalize()
	{
		T L = len();
		x /= L; y /= L; z /= L;
	}
	void _reLength(const T& k)
	{
		T L = len();
		x = x * k / L;
		y = y * k / L;
		z = z * k / L;
	}

	friend std::ostream& operator << (std::ostream& out, const Vector3& tv)
	{
		out << "( " << tv.x << " , " << tv.y << " , " << tv.z << " )";
		return out;
	}

	friend std::istream& operator >> (std::istream& in, Vector3& tv)
	{
		in >> tv.x >> tv.y >> tv.z;
		return in;
	}

	friend Vector3& operator += (Vector3& A, const Vector3& B)
	{
		A = A + B;
		return A;
	}
	friend Vector3& operator -= (Vector3& A, const Vector3& B)
	{
		A = A - B;
		return A;
	}
	friend Vector3& operator *= (Vector3& A, const T& k)
	{
		A = A * k;
		return A;
	}
	friend Vector3& operator /= (Vector3&, const T& k)
	{
		A = A / k;
		return A;
	}
	friend Vector3& operator *= (Vector3& A, const Vector3& B)
	{
		A = A * B;
		return A;
	}
	friend Vector3 operator - (const Vector3& A)
	{
		return Vector3(-A.x, -A.y, -A.z);
	}
	T Distance2(const Vector3& A)
	{
		return dist2(*this, A);
	}
	T Distance(const Vector3& A)
	{
		return dist(*this, A);
	}
	T& GetCoord(int axis)
	{
		if (axis == 0) return x;
		if (axis == 1) return y;
		if (axis == 2) return z;
	}

	void AssRandomVector()
	{
		do {
			x = 2 * ran() - 1;
			y = 2 * ran() - 1;
			z = 2 * ran() - 1;
		} while (x * x + y * y + z * z > 1 || x * x + y * y + z * z < EPS);
		_normalize();
	}
	Vector3 GetAnVerticalVector()
	{
		Vector3 ret = *this * Vector3(0, 0, 1);
		if (ret.IsZeroVector()) ret = Vector3(1, 0, 0);
		else ret._normalize();
		return ret;
	}
	bool IsZeroVector()
	{
		return DBcmp(x, 0) == 0 && DBcmp(y, 0) == 0 && DBcmp(z, 0) == 0;
	}
	void Input(std::stringstream& fin)
	{
		fin >> x >> y >> z;
	}
	Vector3 Reflect(Vector3 N)
	{
		return *this - N * (2 * ((*this) % N));
	}
	Vector3 Refract(Vector3 N, T n)
	{
		Vector3 V = normalize();
		T cosI = -(N % V), cosT2 = 1 - (n * n) * (1 - cosI * cosI);
		if (cosT2 > EPS) return V * n + N * (n * cosI - sqrt(cosT2));
		return V.Reflect(N);
	}
	Vector3 Diffuse()
	{
		Vector3 Vert = GetAnVerticalVector();
		T theta = acos(sqrt(ran()));
		T phi = ran() * 2 * PI;
		return Rotate(Vert, theta).Rotate(*this, phi);
	}
	Vector3 Rotate(Vector3 axis, T theta)
	{
		Vector3 ret;
		double cost = cos(theta);
		double sint = sin(theta);
		ret.x += x * (axis.x * axis.x + (1 - axis.x * axis.x) * cost);
		ret.x += y * (axis.x * axis.y * (1 - cost) - axis.z * sint);
		ret.x += z * (axis.x * axis.z * (1 - cost) + axis.y * sint);
		ret.y += x * (axis.y * axis.x * (1 - cost) + axis.z * sint);
		ret.y += y * (axis.y * axis.y + (1 - axis.y * axis.y) * cost);
		ret.y += z * (axis.y * axis.z * (1 - cost) - axis.x * sint);
		ret.z += x * (axis.z * axis.x * (1 - cost) - axis.y * sint);
		ret.z += y * (axis.z * axis.y * (1 - cost) + axis.x * sint);
		ret.z += z * (axis.z * axis.z + (1 - axis.z * axis.z) * cost);
		return ret;
	}



};
	
typedef Vector3<double> Point3lf;
typedef Vector3<double> Vector3lf;

template<typename T>
T dist2(const Vector3<T>& v1, const Vector3<T>& v2)
{
	T dx = v1.x - v2.x;
	T dy = v1.y - v2.y;
	T dz = v1.z - v2.z;
	return (dx * dx + dy * dy + dz * dz);
}

template<typename T>
T dist(const Vector3<T>& v1, const Vector3<T>& v2)
{
	T dx = v1.x - v2.x;
	T dy = v1.y - v2.y;
	T dz = v1.z - v2.z;
	return std::sqrt(dx * dx + dy * dy + dz * dz);
}


#endif // !Vector_hpp
