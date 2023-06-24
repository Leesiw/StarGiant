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



-- 보스의 초기 상태
state = BossState.ATTACK

-- 보스의 AI 업데이트 함수
function updateBossAI()
    if state == BossState.SLEEP then
        -- 보스가 잠자는 동작을 수행합니다.
        sleep()
    elseif state == BossState.IDLE then
        -- 보스가 유휴 상태일 때의 동작을 수행합니다.
        idle()
    elseif state == BossState.ATTACK then
        -- 보스가 공격할 때의 동작을 수행합니다.
        attack()
    end

    -- 보스의 다음 상태를 결정합니다.
    nextState()
end

-- 보스의 잠자는 동작
function sleep()
    -- 보스가 잠자는 동안의 동작을 정의합니다.
    print("보스가 잠자는 중입니다.")
    -- TODO: 보스의 잠자는 동작을 구현합니다.
end

-- 보스의 유휴 상태 동작
function idle()
    -- 보스의 유휴 상태 동작을 정의합니다.
    print("보스가 idle 상태입니다.")
    -- TODO: 보스의 유휴 상태 동작을 구현합니다.
end

-- 보스의 공격 동작
function attack()
    -- 보스의 공격 동작을 정의합니다.
    print("보스가 공격합니다.")
    -- TODO: 보스의 공격 동작을 구현합니다.
end

-- 보스의 다음 상태 결정
function nextState()
    -- TODO: 보스의 다음 상태를 결정하는 로직을 구현합니다.
    -- 예를 들어, 특정 조건에 따라 다음 상태를 변경할 수 있습니다.
    -- 다음 상태를 state 변수에 할당합니다.

    -- 예시: Lua에서 보스의 상태를 변경
    if someCondition then
        state = BossState.IDLE
    elseif someOtherCondition then
        state = BossState.ATTACK
    end
end

return {
    updateBossAI = updateBossAI
}
