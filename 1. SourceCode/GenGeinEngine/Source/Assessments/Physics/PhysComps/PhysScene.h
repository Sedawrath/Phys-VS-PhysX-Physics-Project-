#pragma once
#include <vector>
#include <glm/glm.hpp>

class PhysActor;

class PhysScene
{
public:

	~PhysScene();
	PhysScene();
	PhysScene(const glm::vec3 a_gravity);

	void Reset();
	void Update(const double a_dt);
	void DebugScene();

	void CheckForCollision(PhysActor* a_actorA, PhysActor* a_actorB);
	void AddActor(PhysActor* a_actor);
	void RemoveActor(PhysActor* a_actor);
	void RemoveAllActors();

	void Render();

	void CheckForCollisions();
	
	// Plane Collision Checks
	static bool PlaneToPlane(PhysActor* a_A, PhysActor* a_B);
	static bool PlaneToSphere(PhysActor* a_A, PhysActor* a_B);
	static bool PlaneToAABB(PhysActor* a_A, PhysActor* a_B);
	static bool PlaneToJoint(PhysActor* a_A, PhysActor* a_B);

	// Sphere Collision Checks
	static bool SphereToPlane(PhysActor* a_A, PhysActor* a_B);
	static bool SphereToSphere(PhysActor* a_A, PhysActor* a_B);
	static bool SphereToAABB(PhysActor* a_A, PhysActor* a_B);
	static bool SphereToJoint(PhysActor* a_A, PhysActor* a_B);

	// AABB Collision Checks
	static bool AABBToPlane(PhysActor* a_A, PhysActor* a_B);
	static bool AABBToSphere(PhysActor* a_A, PhysActor* a_B);
	static bool AABBToAABB(PhysActor* a_A, PhysActor* a_B);
	static bool AABBToJoint(PhysActor* a_A, PhysActor* a_B);

	// Ray Collision Checks
	static bool JointToPlane(PhysActor* a_A, PhysActor* a_B);
	static bool JointToSphere(PhysActor* a_A, PhysActor* a_B);
	static bool JointToAABB(PhysActor* a_A,PhysActor* a_B);
	static bool JointToJoint(PhysActor* a_A, PhysActor* a_B);

private:
	glm::vec3 m_gravity;
	std::vector<PhysActor*> m_actors;
	
	// Inst object references:
	FBXModel* m_pPlaneModel;
	FBXModel* m_pSphereModel;
	FBXModel* m_pBoxModel;
};