-- god_ai.lua

GodState = {
    IDLE1 = 0,
    IDLE2 = 1,
    MELEE1 = 2,
    MELEE2 = 3,
    SHOT = 4,
    HIT1 = 5,
    DEATH = 6

}

SHOT_COOL_TIME = 5
ATTACK_COOL_TIME = 4


-- 보스의 초기 상태
state = GodState.IDLE2
motion = GodState.IDLE2
MaxHp = 100

god_x = -15000   --초기 위치
god_y = -15000
god_z = -15000


curHp = MaxHp

--true false
onappear = true

-- 공격 상태와 쿨타임 변수 초기화
attackState = false
attackCooldown = ATTACK_COOL_TIME

-- 프레임 시간 관련 변수 초기화
frameTime = 0


-- 보스의 AI 업데이트 함수
function updateGodAI(hp, bpx, bpy, bpz, elapsedTime)
    curHp = hp
    frameTime = frameTime + elapsedTime
    attackCooldown = attackCooldown - elapsedTime

    if state == GodState.IDLE2 then        -- 보스가 나타날 때
        print("IDLE2")
        if onappear then
            appear(bpx, bpy, bpz)
            onappear = false
        end
    elseif state == GodState.IDLE1 then    -- 기본상태
        print("IDLE1")
        idle(frameTime)
    elseif state == GodState.DIE then      -- 보스의 체력이 0 이하일 때 죽음 상태 처리
        if curHp <= 0 then
            die()
        end
    elseif state == GodState.SHOT then     -- 보스가 SHOT 때
        print("SHOT")
        attack(frameTime)
    end
    -- 보스의 다음 상태를 결정합니다.
    nextState( bpx, bpy, bpz)
end



-- 보스의 idle 상태 동작
function idle(frameTime)
    --print("idle 상태")
   -- print(frameTime)
   if not attackState and frameTime >= attackCooldown then
        if curHp <= (MaxHp / 2) then
            ATTACK_COOL_TIME = 2
        else
            ATTACK_COOL_TIME = 4
        end

        shotAttack()
        attackCooldown = frameTime + 4
        attackState = true
   end
end

-- 보스의 appear 상태 동작
function appear(bpx, bpy, bpz) -- 우선 플레이어의 위치 + 2300 했음
    god_x = bpx + 2300
    god_y = bpy + 0
    god_z = bpz + 0
end


-- 보스의 공격 동작
function attack(frameTime)
    motion = GodState.SHOT
    attackCooldown = 4
    if frameTime >= attackCooldown then
        attackState = false
    end
end

-- SHOT 실행 함수
function shotAttack()
    motion = GodState.SHOT
    frameTime = 0
end



-- 죽음 상태 처리 함수
function die()
end


function nextState(bpx, bpy, bpz)
    local distance = CalculateDistance(god_x, god_y, god_z, bpx, bpy, bpz)

    if state == GodState.IDLE1 then
        if curHp <= 0 then
            state = GodState.DEATH
            motion = GodState.DEATH
        elseif attackState and state ~= GodState.SHOT then
            state = GodState.SHOT
            frameTime = 0
            attackCooldown = ATTACK_COOL_TIME
        end
        
    elseif state == GodState.IDLE2 then
        state = GodState.IDLE2
        motion = GodState.IDLE2
        if onappear == false then
            state = GodState.IDLE1
        end
 
    elseif state == GodState.SHOT then
        if attackState == false then
            state = GodState.IDLE
            motion = GodState.IDLE 
            frameTime = 0
        end
     
    end
end

-- idle로 바꿈
function setonidle(a)
    onIdle = a;
end

function SubtractVectors(v1, v2)
    return { v1[1] - v2[1], v1[2] - v2[2], v1[3] - v2[3] }
end

function LengthVector(v)
    return math.sqrt(v[1]^2 + v[2]^2 + v[3]^2)
end

function CalculateDistance(x1, y1, z1, x2, y2, z2)
    local dx = x2 - x1
    local dy = y2 - y1
    local dz = z2 - z1
    return math.sqrt(dx^2 + dy^2 + dz^2)
end

return {
    updateGodAI = updateGodAI
}
