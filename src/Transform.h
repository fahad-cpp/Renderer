#pragma once
#include "Vector.h"
struct Transform {
	Vector position = { 0,0,0 };
	float scale = 0.f;
	Vector rotation = 0;
};
enum class RotateOrder {
	RO_YXZ = 0,
	RO_XYZ
};
INTERNAL_W Vector rotate(const Vector& vec, const Vector& rotationP, RotateOrder ro = RotateOrder::RO_YXZ) {

	float sinx, siny, sinz, cosx, cosy, cosz;

	if (rotationP == Vector{ 0,0,0 }) {
		return vec;
	}
	Vector rotation = { float(rotationP.x * (PI * 2)) / 360.f,float(rotationP.y * (PI * 2)) / 360.f,float(rotationP.z * (PI * 2)) / 360.f };

	sinx = sin(rotation.x);
	siny = sin(rotation.y);
	sinz = sin(rotation.z);

	cosx = cos(rotation.x);
	cosy = cos(rotation.y);
	cosz = cos(rotation.z);

	Vector result = {};
	if (ro == RotateOrder::RO_YXZ) {
		//Yaw
		Vector yrotated;
		yrotated.y = vec.y;
		yrotated.x = vec.x * cosy + vec.z * (-siny);
		yrotated.z = vec.x * siny + vec.z * cosy;

		//Pitch
		Vector xrotated;
		xrotated.x = yrotated.x;
		xrotated.y = yrotated.y * cosx + yrotated.z * (-sinx);
		xrotated.z = yrotated.y * sinx + yrotated.z * cosx;

		//Roll
		Vector zrotated;
		zrotated.z = xrotated.z;
		zrotated.x = xrotated.x * cosz + xrotated.y * (-sinz);
		zrotated.y = xrotated.x * sinz + xrotated.y * cosz;

		result = zrotated;
	}
	else if (ro == RotateOrder::RO_XYZ) {
		Vector xrotated;
		xrotated.x = vec.x;
		xrotated.y = vec.y * cosx + vec.z * (-sinx);
		xrotated.z = vec.y * sinx + vec.z * cosx;

		Vector yrotated;
		yrotated.y = xrotated.y;
		yrotated.x = xrotated.x * cosy + xrotated.z * (-siny);
		yrotated.z = xrotated.x * siny + xrotated.z * cosy;

		Vector zrotated;
		zrotated.z = yrotated.z;
		zrotated.x = yrotated.x * cosz + yrotated.y * (-sinz);
		zrotated.y = yrotated.x * sinz + yrotated.y * (cosz);

		result = zrotated;
	}
	return result;
}
inline Vector transformVertex(Vector vec, const Transform& tf, RotateOrder ro = RotateOrder::RO_YXZ) {
	return ((rotate(vec, tf.rotation, ro) * tf.scale) + tf.position);
}
