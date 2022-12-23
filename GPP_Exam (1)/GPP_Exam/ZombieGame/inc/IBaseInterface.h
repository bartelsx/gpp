#pragma once
class IBaseInterface
{
public:
	IBaseInterface();
	virtual ~IBaseInterface();

	//RENDERER
	virtual void Draw_Polygon(const Elite::Vector2* points, int count, const Elite::Vector3& color, float depth) = 0;
	void Draw_Polygon(const Elite::Vector2* points, int count, const Elite::Vector3& color);
	virtual void Draw_SolidPolygon(const Elite::Vector2* points, int count, const Elite::Vector3& color, float depth, bool triangulate = false) = 0;
	void Draw_SolidPolygon(const Elite::Vector2* points, int count, const Elite::Vector3& color);
	virtual void Draw_Circle(const Elite::Vector2& center, float radius, const Elite::Vector3& color, float depth) = 0;
	void Draw_Circle(const Elite::Vector2& center, float radius, const Elite::Vector3& color);
	virtual void Draw_SolidCircle(const Elite::Vector2& center, float32 radius, const Elite::Vector2& axis, const Elite::Vector3& color, float depth) = 0;
	void Draw_SolidCircle(const Elite::Vector2& center, float32 radius, const Elite::Vector2& axis, const Elite::Vector3& color);
	virtual void Draw_Segment(const Elite::Vector2& p1, const Elite::Vector2& p2, const Elite::Vector3& color, float depth) = 0;
	void Draw_Segment(const Elite::Vector2& p1, const Elite::Vector2& p2, const Elite::Vector3& color);
	virtual void Draw_Direction(const Elite::Vector2& p, Elite::Vector2 dir, float length, const Elite::Vector3& color, float depth = 0.9f) = 0;
	virtual void Draw_Transform(const b2Transform& xf, float depth) = 0;
	void Draw_Transform(const b2Transform& xf);
	virtual void Draw_Point(const Elite::Vector2& p, float size, const Elite::Vector3& color, float depth) = 0;
	void Draw_Point(const Elite::Vector2& p, float size, const Elite::Vector3& color);
	//void Draw_String(int x, int y, const char* string, ...) = 0;
	//virtual void Draw_String(const Elite::Vector2& pw, const char* string, ...) = 0;

	virtual float NextDepthSlice() = 0;
};

