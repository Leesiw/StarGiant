//------------------------------------------------------- ----------------------
// File: Frustrum.h
//-----------------------------------------------------------------------------

#pragma once

#include "Camera.h"
#include "stdafx.h"


class Frustrum
{
public:
	Frustrum(CFrustrumCamera* camera);
	~Frustrum(void);

	void update(void);

	CollisionDetection checkAABBCollision(AABB& box);

	CFrustrumCamera* camera;
	Vertex viewingVolume[8];
	Plan viewingPlans[6];
	string viewingPlanNames[6];
private:

	bool isPointInFront(Vertex& point, ViewingPlan planIndice);
	CollisionDetection planeAABBCollision(AABB& box, int  viewingPlan);
};
