#pragma once
#include "../Math/MatrixTransform.h"
#include "../Core/Core.h"

class ASPEN EditorCamera
{
private:
	Vector2f m_Position;
	float m_Scale;

	float m_Ratio;
public:
	EditorCamera(float ratio);

	void Update();

	Matrix3x3f GetCameraMatrix() const;
	void SetRatio(float ratio) { m_Ratio = ratio; }
	float GetRatio() const { return m_Ratio; }
	float GetScale() const { return m_Scale; }
	Vector2f GetPosition() const{ return m_Position; }
};