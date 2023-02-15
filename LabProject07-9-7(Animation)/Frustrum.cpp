//-----------------------------------------------------------------------------
// File: Frustrum.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Frustrum.h"

Frustrum::Frustrum(CFrustrumCamera* camera)
{
	this->camera = camera;
	//Frustrum을 이루는 면들 
	viewingPlanNames[BOTTOM] = "BOTTOM";
	viewingPlanNames[TOP] = "TOP";
	viewingPlanNames[LEFT] = "LEFT";
	viewingPlanNames[RIGHT] = "RIGHT";
	viewingPlanNames[FNEAR] = "NEAR";
	viewingPlanNames[FFAR] = "FAR";
}

Frustrum::~Frustrum(void)
{
}

void Frustrum::update(void)
{
	float tan_tmp = tan(camera->fovy * ANG2RAD / 2); //카메라 시야범위 Tan값

	float hNear = tan_tmp * camera->zNear;
	float wNear = hNear * camera->aspect;

	float hFar = tan_tmp * camera->zFar;
	float wFar = hFar * camera->aspect;

	//변환
	Vertex directionZ = Vertex(camera->GetLookAtPosition().x, camera->GetLookAtPosition().y, camera->GetLookAtPosition().z) - Vertex(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
	directionZ.normalize();

	Vertex directionX = directionZ.crossProduct(Vertex(camera->GetUpVector().x, camera->GetUpVector().y, camera->GetUpVector().z));
	directionX.normalize();

	Vertex directionY = directionX.crossProduct(directionZ);
	directionY.normalize();



	//Near Center, Far Center Vertex
	Vertex nc = Vertex(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z) + (directionZ * camera->zNear);
	Vertex fc = Vertex(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z) + (directionZ * camera->zFar);


	// Update every 8 viewing frustrum
	viewingVolume[NTR] = nc + (directionX * wNear) + (directionY * hNear);
	viewingVolume[NBR] = nc + (directionX * wNear) - (directionY * hNear);
	viewingVolume[NTL] = nc - (directionX * wNear) + (directionY * hNear);
	viewingVolume[NBL] = nc - (directionX * wNear) - (directionY * hNear);
	viewingVolume[FTR] = fc + (directionX * wFar) + (directionY * hFar);
	viewingVolume[FBR] = fc + (directionX * wFar) - (directionY * hFar);
	viewingVolume[FTL] = fc - (directionX * wFar) + (directionY * hFar);
	viewingVolume[FBL] = fc - (directionX * wFar) - (directionY * hFar);


	// Extract plans
	Vertex leftToRight = viewingVolume[NBR] - viewingVolume[NBL];
	Vertex bottomToTop = viewingVolume[NTR] - viewingVolume[NBR];
	viewingPlans[FNEAR] = Plan(bottomToTop, leftToRight, viewingVolume[NBR]);
	viewingPlans[FFAR] = Plan(leftToRight, bottomToTop, viewingVolume[FBR]);

	Vertex leftNearToFar = viewingVolume[FBL] - viewingVolume[NBL];
	viewingPlans[BOTTOM] = Plan(leftToRight, leftNearToFar, viewingVolume[NBL]);
	viewingPlans[LEFT] = Plan(leftNearToFar, bottomToTop, viewingVolume[NBL]);

	Vertex rightNearToFar = viewingVolume[FBR] - viewingVolume[NBR];
	viewingPlans[RIGHT] = Plan(bottomToTop, rightNearToFar, viewingVolume[NBR]);

	Vertex upperLeftNearToFar = viewingVolume[FTL] - viewingVolume[NTL];
	viewingPlans[TOP] = Plan(upperLeftNearToFar, leftToRight, viewingVolume[NTL]);

}

CollisionDetection Frustrum::checkAABBCollision(AABB& box)
{
	CollisionDetection result;
	bool intersecting = false;
	for (int i = 0; i < 6; i++)
	{
		result = planeAABBCollision(box, i);
		if (result == COLL_OUT) return COLL_OUT;
		if (result == COLL_INTERSECT)
			intersecting = true;
	}
	if (intersecting)
		return COLL_INTERSECT;
	return COLL_IN;

}

bool Frustrum::isPointInFront(Vertex& point, ViewingPlan planIndice)
{
	Plan* plan = &viewingPlans[planIndice];
	float distanceFromPlan = plan->distanceFromPoint(point);
	return distanceFromPlan >= 0;
}

CollisionDetection Frustrum::planeAABBCollision(AABB& box, int viewingPlan)
{
	float distance = 0;
	bool foundNegative = false;
	bool foundPositive = false;
	for (int i = 0; i < 8; i++)
	{
		distance = viewingPlans[viewingPlan].distanceFromPoint(box.boundaries[i]);
		if (distance < 0)
			foundNegative = true;
		else
			foundPositive = true;
	}

	if (foundNegative && foundPositive)
		return COLL_INTERSECT;

	if (foundPositive)
		return COLL_IN;
	else
		return COLL_OUT;
}
