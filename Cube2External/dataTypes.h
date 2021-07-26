struct Vec2 {
	float X;
	float Y;
};

struct Vec3 {
	float X;
	float Y;
	float Z;
	void multiply(Vec3 vector) {
		X *= vector.X;
		Y *= vector.Y;
		Z *= vector.Z;
	};
};

struct Vec4 {
	float X;
	float Y;
	float Z;
	float W;
};

struct Matrix4D {
	float Matrix[16];
};