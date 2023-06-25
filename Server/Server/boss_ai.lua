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

CLAW_COOL_TIME = 4
FLAME_COOL_TIME = 4

-- 보스의 초기 상태

state = BossState.IDLE
motion = BossState.IDLE
MaxHp = 100

boss_x = 1000
boss_y = 1000
boss_z = 1000


curHp = MaxHp


-- 공격 상태와 쿨타임 변수 초기화
attackState = false
attackCooldown = 0
clawCooldown = 0
flameCooldown = 0

-- 프레임 시간 관련 변수 초기화
frameTime = 0
frameInterval = 0


-- 보스의 AI 업데이트 함수
function updateBossAI(hp, bpx, bpy, bpz, frameTime)
    curHp = hp + 10
    --print(bpx, bpy, bpz)

    if state == BossState.SLEEP then
        -- 보스가 SLEEP 동작을 수행합니다.
        sleep()
    elseif state == BossState.APPEAR then
        -- 보스가 나타날 때의 동작을 수행합니다.
        appear(bpx, bpy, bpz)
    elseif state == BossState.IDLE then
        idle(frameTime)
    elseif state == BossState.DIE then
        -- 보스의 체력이 0 이하일 때 죽음 상태 처리
        if curHp <= 0 then
            die()
        end
    elseif state == BossState.ATTACK then
        -- 보스가 ATTACK 때의 동작을 수행합니다.
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


-- 보스의 idle 상태 동작
function idle(frameTime)
    print("idle 상태")
    print(frameTime)
    if frameTime >= attackCooldown then
        local attackType = math.random(1, 3)
        if attackType == BossState.BASIC_ATTACK then
            basicAttack()
            attackCooldown = frameTime + 4
            attackState = true
        elseif attackType == BossState.FLAME_ATTACK then
            if curHp <= MaxHp / 2 and frameTime >= flameCooldown then
                flameAttack()
                flameCooldown = frameTime + FLAME_COOL_TIME
                attackCooldown = frameTime + 4
                attackState = true
            end
        elseif attackType == BossState.CLAW_ATTACK then
            if frameTime >= clawCooldown then
                clawAttack()
                clawCooldown = frameTime + CLAW_COOL_TIME
                attackCooldown = frameTime + 4
                attackState = true
            end
        end
    end

    if attackState then
        attackCooldown = frameTime + 4
    end
end


-- 보스의 appear 상태 동작
function appear(bpx, bpy, bpz)
    -- 보스의 appear 상태 동작을 정의합니다.
    print("보스가 appear 상태입니다.")
    -- TODO: 보스의 appear 상태 동작을 구현합니다.
    -- 플레이어의 위치로 세팅
    boss_x = bpx + 1000
    boss_y = bpy + 1000
    boss_z = bpz + 1000
end


-- 보스의 공격 동작
function attack(frameTime)
    print("보스가 공격합니다.")
    -- TODO: 보스의 공격 동작을 구현합니다.

    if frameTime >= attackCooldown then
        state = BossState.IDLE
        attackState = false
    end
end


-- BASIC_ATTACK 실행 함수
function basicAttack()
    print("보스가 BASIC_ATTACK을 실행합니다.")
    -- TODO: BASIC_ATTACK 실행 코드 작성
end

-- FLAME_ATTACK 실행 함수
function flameAttack()
    print("보스가 FLAME_ATTACK을 실행합니다.")
    -- TODO: FLAME_ATTACK 실행 코드 작성
end

-- CLAW_ATTACK 실행 함수
function clawAttack()
    print("보스가 CLAW_ATTACK을 실행합니다.")
    -- TODO: CLAW_ATTACK 실행 코드 작성
end

-- 죽음 상태 처리 함수
function die()
    print("보스가 죽었습니다.")
    -- TODO: 죽음 상태 처리 코드 작성
end


function nextState()
    print("다음으로")

    if state == BossState.IDLE then
        if curHp <= 0 then
            state = BossState.DIE
            print("다이")
        elseif attackState then
            state = BossState.ATTACK
            print("공격")
        end
        -- 다른 조건에 따른 상태 변경 로직을 추가하세요
    elseif state == BossState.ATTACK then
        if someCondition then
            state = BossState.IDLE
            print("IDLE")
        end
        -- 다른 조건에 따른 상태 변경 로직을 추가하세요
    end
end

-- 보스가 특정 위치를 바라보도록 하는 함수
function LookAtPosition()
    return true;
end

return {
    updateBossAI = updateBossAI
}
