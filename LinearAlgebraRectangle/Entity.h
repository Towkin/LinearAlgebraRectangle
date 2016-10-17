#pragma once

#include "Eigen"
#include <vector>
#include "UniqueID.h"

enum EContext {
	Global,
	Relative
};
enum ERotation {
	Radians,
	Degrees
};

// A rotation, whose value is always in the range [-M_PI...M_PI] radians, or [-180...180] degrees.
struct Rotation {
private:
	// The rotation is saved as radians.
	float Radians;
	// Simple Radian->Degree scalar.
	static const float RadToDeg;

public:
	Rotation() : Radians(0.f) {};
	Rotation(float aValue, ERotation aType = ERotation::Radians) : Rotation() { Set(aValue, aType); };

	// Returns the rotation as a float in either Radians or Degrees. 
	float Get(ERotation aType = ERotation::Radians) const { return aType == ERotation::Radians ? Radians : Radians * RadToDeg; }
	void Set(float aValue, ERotation aType = ERotation::Radians) { 
		// Compact ternary operator degrees->radians conversion, and use of fmodf to get Radians in range -M_PI...M_PI
		Radians = std::fmodf(
			(aType == ERotation::Radians ? aValue : aValue / RadToDeg) + M_PI, 
			M_PI * 2) 
		- M_PI; 
	}

	// Simple add and subtract operators.
	Rotation operator+(const Rotation& aOther) const { return Rotation(this->Radians + aOther.Radians); }
	Rotation operator-(const Rotation& aOther) const { return Rotation(this->Radians - aOther.Radians); }

	// Rotation operation, returns the rotated vector.
	Eigen::Vector2f operator*(const Eigen::Vector2f& aVector) const;
};

class Entity : UniqueID {
private:
	std::vector<Entity*> Children;
	Entity* Parent;

	Eigen::Vector2f RelativePosition;
	Rotation RelativeRotation;
	Eigen::Vector2f RelativeVelocity;
	
public:
	Entity();
	Entity(const Entity& aCopy);
	~Entity();

	size_t GetChildCount() const;
	Entity* GetChild(size_t aIndex) const;

	bool HasParent() const;
	bool IsChildTo(Entity* aParent) const;
	void AttachTo(Entity* aParent, EContext aContext = EContext::Global);
	void DetachFromParent(EContext aContext = EContext::Global);
	

	Eigen::Vector2f GetPosition(EContext aContext = EContext::Global) const;
	void SetPosition(Eigen::Vector2f aPosition, EContext aContext = EContext::Global);

	Rotation GetRotation(EContext aContext = EContext::Global) const;
	void SetRotation(Rotation aRotation, EContext aContext = EContext::Global);
};

