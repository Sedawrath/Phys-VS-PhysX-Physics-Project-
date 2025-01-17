#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include "Engine\Objects\FBXModel.h"

#include "ParticleEmitter.h"

//constructor
ParticleEmitter::ParticleEmitter(int _maxParticles, PxVec3 _position,PxParticleSystem* _ps,float _releaseDelay, FBXModel* a_model)
{
	m_releaseDelay		= _releaseDelay;
	//maximum number of particles our emitter can handle
	m_maxParticles		= _maxParticles; 

	//array of particle structs
	m_activeParticles	= new Particle[m_maxParticles]; 
	
	m_time				= 0; //time system has been running
	m_respawnTime		= 0; //time for next respawn
	m_position			= _position;
	m_ps				=_ps; //pointer to the physX particle system
	m_particleMaxAge	= 8; //maximum time in seconds that a particle can live for
	
	m_model = a_model;

	//initialize the buffer
	for(int index=0;index<m_maxParticles;index++)
	{
		m_activeParticles[index].active = false;
		m_activeParticles[index].maxTime = 0;
	}

	m_minVelocity = PxVec3(-10.0f, 0, -10.0f);
	m_maxVelocity = PxVec3(10.0f, 0, 10.0f);
}

//destructure
ParticleEmitter::~ParticleEmitter()
{
	//remove all the active particles
	delete m_activeParticles;
}

void ParticleEmitter::SetStartVelocityRange(float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	m_minVelocity.x = minX;
	m_minVelocity.y = minY;
	m_minVelocity.z = minZ;
	m_maxVelocity.x = maxX;
	m_maxVelocity.y = maxY;
	m_maxVelocity.z = maxZ;
}

//find the next free particle, mark it as used and return it's index.  If it can't allocate a particle: returns minus one
int ParticleEmitter::GetNextFreeParticle()
{
	//find particle, this is a very inefficient way to do this.  A better way would be to keep a list of free particles so we can quickly find the first free one
	for(int index=0;index<m_maxParticles;index++)
	{
		//when we find a particle which is free
		if(!m_activeParticles[index].active)
		{
			m_activeParticles[index].active = true; //mark it as not free
			m_activeParticles[index].maxTime = m_time+m_particleMaxAge;  //record when the particle was created so we know when to remove it
			return index;
		}
	}
	return -1; //returns minus if a particle was not allocated
}


//releast a particle from the system using it's index to ID it
void ParticleEmitter::ReleaseParticle(int index)
{
	if(index >= 0 && index < m_maxParticles)
		m_activeParticles[index].active = false;
}

//returns true if a particle age is greater than it's maximum allowed age
bool ParticleEmitter::TooOld(int index)
{
	if(index >= 0 && index < m_maxParticles && m_time > m_activeParticles[index].maxTime)
		return true;
	return false;
}

//add particle to PhysX System
bool ParticleEmitter::AddPhysXParticle(int particleIndex)
{

	//set up the data
	//set up the buffers
	PxU32 myIndexBuffer[] = {particleIndex};
	PxVec3 startPos = m_position;
	PxVec3 startVel(0,0,0);
	//randomize starting velocity.
	float fT = (rand() % (RAND_MAX + 1)) / (float)RAND_MAX;
	startVel.x += m_minVelocity.x + (fT * (m_maxVelocity.x - m_minVelocity.x));
	fT = (rand() % (RAND_MAX + 1)) / (float)RAND_MAX;
	startVel.y += m_minVelocity.y + (fT * (m_maxVelocity.y - m_minVelocity.y));
	fT = (rand() % (RAND_MAX + 1)) / (float)RAND_MAX;
	startVel.z += m_minVelocity.z + (fT * (m_maxVelocity.z - m_minVelocity.z));

	//we can change starting position tos get different emitter shapes
	PxVec3 myPositionBuffer[] = {startPos};
	PxVec3 myVelocityBuffer[] =  {startVel};

	//reserve space for data
	PxParticleCreationData particleCreationData;
	particleCreationData.numParticles = 1;  //spawn one particle at a time,  this is inefficient and we could improve this by passing in the list of particles.
	particleCreationData.indexBuffer = PxStrideIterator<const PxU32>(myIndexBuffer);
	particleCreationData.positionBuffer = PxStrideIterator<const PxVec3>(myPositionBuffer);
	particleCreationData.velocityBuffer = PxStrideIterator<const PxVec3>(myVelocityBuffer);
	// create particles in *PxParticleSystem* ps
	return m_ps->createParticles(particleCreationData);
}

//updateParticle
void ParticleEmitter::Update(const double delta)
{
	//tick the emitter
	m_time += (float)delta;
	m_respawnTime+= (float)delta;
	int numberSpawn = 0;
	//if respawn time is greater than our release delay then we spawn at least one particle so work out how many to spawn
	if(m_respawnTime>m_releaseDelay)
	{
		numberSpawn = (int)(m_respawnTime/m_releaseDelay);
		m_respawnTime -= (numberSpawn * m_releaseDelay);
	}
	// spawn the required number of particles 
	for(int count = 0;count < numberSpawn;count++)
	{
		//get the next free particle
		int particleIndex = GetNextFreeParticle();
		if(particleIndex >=0)
		{
			//if we got a particle ID then spawn it
			AddPhysXParticle(particleIndex);
		}
	}
	//check to see if we need to release particles because they are either too old or have hit the particle sink
	//lock the particle buffer so we can work on it and get a pointer to read data
	PxParticleReadData* rd = m_ps->lockParticleReadData();
	// access particle data from PxParticleReadData was OK
	if (rd)
	{
		std::vector<PxU32> particlesToRemove; //we need to build a list of particles to remove so we can do it all in one go
		PxStrideIterator<const PxParticleFlags> flagsIt(rd->flagsBuffer);

		for (unsigned i = 0; i < rd->validParticleRange; ++i, ++flagsIt)
		{
			if (*flagsIt & PxParticleFlag::eVALID)
				{
					//if particle is either too old or has hit the sink then mark it for removal.  We can't remove it here because we buffer is locked
					if (*flagsIt & PxParticleFlag::eCOLLISION_WITH_DRAIN || TooOld(i))
					{
						//mark our local copy of the particle free
						ReleaseParticle(i);
						//add to our list of particles to remove
						particlesToRemove.push_back(i);
					}
				}
		}
		// return ownership of the buffers back to the SDK
		rd->unlock();
		//if we have particles to release then pass the particles to remove to PhysX so it can release them
		if(particlesToRemove.size()>0)
		{
			//create a buffer of particle indicies which we are going to remove
			PxStrideIterator<const PxU32> indexBuffer(&particlesToRemove[0]);
			//free particles from the physics system
			m_ps->releaseParticles(particlesToRemove.size(), indexBuffer);
		}
	}
}

//simple routine to render our particles
void ParticleEmitter::RenderParticles()
{
	// lock SDK buffers of *PxParticleSystem* ps for reading
	PxParticleReadData* rd = m_ps->lockParticleReadData();
	// access particle data from PxParticleReadData
	if (rd)
	{
		PxStrideIterator<const PxParticleFlags> flagsIt(rd->flagsBuffer);
		PxStrideIterator<const PxVec3> positionIt(rd->positionBuffer);

		for (unsigned int i = 0; i < rd->validParticleRange; ++i, ++flagsIt, ++positionIt)
		{
				if (*flagsIt & PxParticleFlag::eVALID)
				{
						//convert physx vector to a glm vec3
						glm::vec3 pos(positionIt->x,positionIt->y,positionIt->z);
						//use a gizmo box to visualize particle.  This would be much better done using a facing quad preferably done using the geometry shader
						
						m_model->SetLocalTransform(glm::translate(pos) * glm::scale(glm::vec3(0.1f)));
						m_model->Render();
				}
		}
		// return ownership of the buffers back to the SDK
		rd->unlock();
	}
}