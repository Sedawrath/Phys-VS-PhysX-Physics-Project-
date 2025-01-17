#pragma once
#include <PxSimulationEventCallback.h>

using namespace physx;
class CollisionCallback : public PxSimulationEventCallback
{
public:
	CollisionCallback();

	virtual void onContact(const PxContactPairHeader& a_pairHeader,
		const PxContactPair* a_pairs, PxU32 a_nbPairs);

	virtual void onTrigger(PxTriggerPair* a_pairs, PxU32 a_nbPairs);
	virtual void onConstraintBreak(PxConstraintInfo*, PxU32);
	virtual void onWake(PxActor**, PxU32);
	virtual void onSleep(PxActor**, PxU32);

	const bool GetTriggered() { return m_triggered; };
	PxRigidActor* GetTriggerBody() { return m_triggerBody; };
private:
	PxRigidActor* m_triggerBody;
	bool m_triggered;
};

