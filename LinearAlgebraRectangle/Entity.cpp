#include "Entity.h"
#include <algorithm>


const float Rotation::RadToDeg = 180.f / M_PI;

Eigen::Vector2f Rotation::operator*(const Eigen::Vector2f& aVector) const {
	Eigen::Matrix2f RotationMatrix;
	RotationMatrix << cosf(this->Get()), -sinf(this->Get()),
					  sinf(this->Get()),  cosf(this->Get());
	
	return RotationMatrix * aVector;
}

Entity::Entity() :
	UniqueID(),
	Children(),
	Parent(nullptr),
	RelativePosition(),
	RelativeRotation()
{}

Entity::Entity(const Entity& aCopy) : Entity() {
	AttachTo(aCopy.Parent);
	SetPosition(aCopy.GetPosition(EContext::Relative), EContext::Relative);
	SetRotation(aCopy.GetRotation(EContext::Relative), EContext::Relative);
	for (Entity* Child : aCopy.Children) {
		Entity* ChildCopy = new Entity(*Child);
		ChildCopy->AttachTo(this);
	}
}

Entity::~Entity() {
	DetachFromParent();
	for (Entity* Child : Children) {
		Child->DetachFromParent();
	}
}

size_t Entity::GetChildCount() const {
	return Children.size();
}

Entity* Entity::GetChild(size_t aIndex) const {
	if (aIndex >= GetChildCount()) {
		// TODO: Throw exception.
		
		return nullptr;
	}

	return Children[aIndex];
}

bool Entity::HasParent() const {
	return Parent != nullptr;
}

bool Entity::IsChildTo(Entity* aParent) const {
	const Entity* HierarchyIterator = this;

	while (HierarchyIterator != nullptr && HierarchyIterator != aParent) {
		HierarchyIterator = HierarchyIterator->Parent;
	}

	return HierarchyIterator == aParent;
}

void Entity::AttachTo(Entity* aParent, EContext aContext) {
	if (Parent == aParent || aParent->IsChildTo(this)) {
		return;
	}

	Eigen::Vector2f OldPosition = GetPosition(aContext);

	this->DetachFromParent();

	aParent->Children.push_back(this);
	this->Parent = aParent;

	SetPosition(OldPosition, aContext);
}

void Entity::DetachFromParent(EContext aContext) {
	if (!HasParent()) {
		return;
	}

	Eigen::Vector2f OldPosition = GetPosition(aContext);

	std::vector<Entity*>::iterator ChildIt = std::find(Parent->Children.begin(), Parent->Children.end(), this);
	if (ChildIt != Parent->Children.end()) {
		Parent->Children.erase(ChildIt);
	}
	Parent = nullptr;

	SetPosition(OldPosition, aContext);
}

Eigen::Vector2f Entity::GetPosition(EContext aContext) const {
	return aContext == EContext::Global && HasParent() ? 
		Parent->GetRotation(aContext) * RelativePosition + Parent->GetPosition(aContext) 
		: RelativePosition;
}
void Entity::SetPosition(Eigen::Vector2f aPosition, EContext aContext) {
	RelativePosition = aContext == EContext::Global && HasParent() ? 
		aPosition - Parent->GetPosition(aContext) 
		: aPosition;
}

Rotation Entity::GetRotation(EContext aContext) const {
	return aContext == EContext::Global && HasParent() ? 
		RelativeRotation + Parent->GetRotation(aContext) 
		: RelativeRotation;
}
void Entity::SetRotation(Rotation aRotation, EContext aContext) {
	RelativeRotation = aContext == EContext::Global && HasParent() ?
		aRotation - Parent->GetRotation()
		: aRotation;
}