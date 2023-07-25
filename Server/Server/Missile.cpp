#include "Missile.h"
#include "Session.h"

CMissile::CMissile()
{
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	isActive = false;

	// 바운딩 박스 모델 수정 시 수정 필요.
	boundingbox = BoundingOrientedBox(XMFLOAT3(-2.38419e-07f, 0.640688f, -2.98023e-07f), XMFLOAT3(1.44325f, 6.89483f, 1.44325f), XMFLOAT4(0.f, 0.f, 0.f, 1.f));
}

void CMissile::Animate(float fTimeElapsed, CGameObject* target)
{
	m_fTrackingTimeRemaining -= fTimeElapsed;

	if (m_fTrackingTimeRemaining > 0.f)
	{	
		LookAtPosition(fTimeElapsed, target->GetPosition());
	}

	float fDistance = 80.f * fTimeElapsed;
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(GetLook(), fDistance, true);
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);

	if (m_fTrackingTimeRemaining < -4.f) { Reset(); }
}

void CMissile::LookAtPosition(float fTimeElapsed, const XMFLOAT3& pos)
{
	XMFLOAT3 new_pos = pos;
	XMMATRIX inv_mat = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World));	// 역행렬

	new_pos = Vector3::TransformCoord(new_pos, inv_mat); // 타겟의 위치를 적 자체의 좌표계로 변환
	if (Vector3::Length(new_pos) > 0.0001f) {
		new_pos = Vector3::Normalize(new_pos);
	}

	float pitch = XMConvertToDegrees(asin(-new_pos.y));
	float yaw = XMConvertToDegrees(atan2(new_pos.x, new_pos.z));

	float rotate_angle = fTimeElapsed * 100.f;

	XMFLOAT3 p_y_r{ pitch, yaw, 0.f };
	if (Vector3::Length(p_y_r) > rotate_angle) {
		p_y_r = Vector3::Normalize(p_y_r);
		Rotate(p_y_r.x * rotate_angle, p_y_r.y * rotate_angle, 0.f);
	}
	else {
		Rotate(pitch, yaw, 0.f);
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
	m_xmf4x4ToParent = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();

	isActive = false;
}
