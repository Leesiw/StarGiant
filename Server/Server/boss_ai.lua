-- boss_ai.lua

BossState = {
    SLEEP = 0,
    IDLE = 1,
    SIT_IDLE = 2,
    SCREAM = 3,
    GET_HIT = 4,
    WALK = 5,
    RUN = 6,
    BASIC_ATTACK = 7,
    CLAW_ATTACK = 8,
    FLAME_ATTACK = 9,
    DEFEND = 10,
    TAKE_OFF = 11,
    FLY_FLOAT = 12,
    FLY_FLAME_ATTACK = 13,
    FLY_FORWARD = 14,
    FLY_GLIDE = 15,
    LAND = 16,
    DIE = 17,
    ATTACK = 18,
    CHASE = 19,
    APPEAR = 20,
    DEL = 21,
    COUNT = 22
}





-- ������ �ʱ� ����

state = BossState.APPEAR
motion = BossState.IDLE
MaxHp = 100

boss_x = 1000
boss_y = 1000
boss_z = 1000


curHp = MaxHp

function getPlayerPos(px, py, pz)
    local position = { px, py, pz }
	return position;
end



-- ������ AI ������Ʈ �Լ�
function updateBossAI(hp, bpx, bpy, bpz)
    curHp = hp + 10
    print(bpx,bpy,bpz)

    if state == BossState.SLEEP then
        -- ������ SLEEP ������ �����մϴ�.
        sleep()

    elseif state == BossState.APPEAR then
        -- ������ ��Ÿ�� ���� ������ �����մϴ�.
        appear(bpx, bpy, bpz)

    elseif state == BossState.IDLE then
        -- ������ IDLE ���� ������ �����մϴ�.
        idle(bpx, bpy, bpz)

    elseif state == BossState.ATTACK then
        -- ������ ATTACK ���� ������ �����մϴ�.
        attack()
    end

    -- ������ ���� ���¸� �����մϴ�.
    nextState()
end

-- ������ ���ڴ� ����
function sleep()
    -- ������ ���ڴ� ������ ������ �����մϴ�.
    print("������ ���ڴ� ���Դϴ�.")
    -- TODO: ������ ���ڴ� ������ �����մϴ�.
end

-- ������ ���� ���� ����
function idle(bpx, bpy, bpz)
    -- ������ ���� ���� ������ �����մϴ�.
    print("������ idle �����Դϴ�.")
    -- TODO: ������ ���� ���� ������ �����մϴ�.


end

-- ������ appear ���� ����
function appear(bpx, bpy, bpz)
    -- ������ appear ���� ������ �����մϴ�.
    print("������ appear �����Դϴ�.")
    -- TODO: ������ appear ���� ������ �����մϴ�.
    -- �÷��̾��� ��ġ
    boss_x = bpx
    boss_y = bpy
    boss_z = bpz

end

-- ������ ���� ����
function attack()
    -- ������ ���� ������ �����մϴ�.
    print("������ �����մϴ�.")
    -- TODO: ������ ���� ������ �����մϴ�.
end

-- ������ ���� ���� ����
function nextState()
    -- TODO: ������ ���� ���¸� �����ϴ� ������ �����մϴ�.
    -- ���� ���, Ư�� ���ǿ� ���� ���� ���¸� ������ �� �ֽ��ϴ�.
    -- ���� ���¸� state ������ �Ҵ��մϴ�.

    -- ����: Lua���� ������ ���¸� ����
    if someCondition then
        state = BossState.IDLE
    elseif someOtherCondition then
        state = BossState.ATTACK
    end
end


-- ������ Ư�� ��ġ�� �ٶ󺸵��� �ϴ� �Լ�
function LookAtPosition()
    return true;
end

return {
    updateBossAI = updateBossAI
}
