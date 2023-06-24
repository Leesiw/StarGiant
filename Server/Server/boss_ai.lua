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
state = BossState.ATTACK

-- ������ AI ������Ʈ �Լ�
function updateBossAI()
    if state == BossState.SLEEP then
        -- ������ ���ڴ� ������ �����մϴ�.
        sleep()
    elseif state == BossState.IDLE then
        -- ������ ���� ������ ���� ������ �����մϴ�.
        idle()
    elseif state == BossState.ATTACK then
        -- ������ ������ ���� ������ �����մϴ�.
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
function idle()
    -- ������ ���� ���� ������ �����մϴ�.
    print("������ idle �����Դϴ�.")
    -- TODO: ������ ���� ���� ������ �����մϴ�.
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

return {
    updateBossAI = updateBossAI
}
