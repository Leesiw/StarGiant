#include "Missile.h"

CMissile::CMissile()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	isActive = false;
}

void CMissile::Animate(float fTimeElapsed, CGameObject* target)
{
	m_fTrackingTimeRemaining -= fTimeElapsed;

	if (m_fTrackingTimeRemaining > 0.f)
	{	
		LookAtPosition(fTimeElapsed, target->GetPosition());
	}

	float fDistance = 50.f * fTimeElapsed;
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(GetLook(), fDistance, true);
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);

	m_fMovingDistance += fDistance;

	if (m_fTrackingTimeRemaining < -10.f) Reset();
}

void CMissile::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	new_pos = Vector3::Normalize(new_pos);

	float pitch = asin(-new_pos.y);
	float yaw = atan2(new_pos.x, new_pos.z);

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > 0.1f) {
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * fTimeElapsed * 90.f, p_y_r.y * fTimeElapsed * 90.f, 0.f);
	}
}

void CMissile::SetisActive(bool active)
{
	isActive = active;
}

XMFLOAT4 CMissile::GetQuaternion()
{
	XMMATRIX mat = XMLoadFloat4x4(&m_xmf4x4ToParent);
	XMVECTOR vec = XMQuaternionRotationMatrix(mat);
	XMFLOAT4 xmf4;
	XMStoreFloat4(&xmf4, vec);
	return xmf4;
}

void CMissile::SetNewMissile(const MissileInfo& info)
{
	m_fMovingDistance = 0.f;
	isActive = true;
	m_fTrackingTimeRemaining = m_fTrackingTime;

	damage = info.damage;
	XMVECTOR a = XMLoadFloat4(&info.Quaternion);
	XMMATRIX mat = XMMatrixRotationQuaternion(a);
	m_xmf4x4ToParent = Matrix4x4::Multiply(Matrix4x4::Identity(), mat);
	SetPosition(info.StartPos);
}

void CMissile::Reset()
{
	isActive = false;
}
