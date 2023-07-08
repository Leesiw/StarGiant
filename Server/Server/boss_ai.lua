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


CLAW_COOL_TIME = 5
FLAME_COOL_TIME = 10




-- 보스의 초기 상태
state = BossState.SLEEP
motion = BossState.SLEEP
MaxHp = 100

boss_x = 5000   --위치
boss_y = 5000
boss_z = 5000


curHp = MaxHp


--true false
onappear = true
onIdle = false


-- 공격 상태와 쿨타임 변수 초기화
attackState = false
attackCooldown = 4
clawCooldown = 0
flameCooldown = 0

-- 프레임 시간 관련 변수 초기화
frameTime = 0
frameInterval = 0


-- 보스의 AI 업데이트 함수
function updateBossAI(hp, bpx, bpy, bpz, elapsedTime)
    curHp = hp
    frameTime = frameTime + elapsedTime
    clawCooldown = clawCooldown - elapsedTime
    flameCooldown = flameCooldown - elapsedTime

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
        attack(frameTime)
    end
    -- 보스의 다음 상태를 결정합니다.
    nextState( bpx, bpy, bpz)
end

-- 보스의 잠자는 동작
function sleep()
    -- 보스가 잠자는 동안의 동작을 정의합니다.
    --print("보스가 sleep 중입니다.")
    -- TODO: 보스의 잠자는 동작을 구현합니다.

end


-- 보스의 idle 상태 동작
function idle(frameTime)
    --print("idle 상태")
   -- print(frameTime)
   if not attackState and frameTime >= attackCooldown then
        local attackType
        
        if curHp <= (MaxHp / 2) then
            attackType = math.random(7, 9)
        else
            attackType = math.random(7, 8)
        end

        --print("attackType - ", attackType)
        if attackType == tonumber(BossState.BASIC_ATTACK) then
            basicAttack()
            attackCooldown = frameTime + 4
            attackState = true
        elseif attackType == tonumber(BossState.CLAW_ATTACK) then
            if clawCooldown <= 0 then
                clawAttack()
                clawCooldown = frameTime + CLAW_COOL_TIME
                attackCooldown = frameTime + 4
                attackState = true
            end
        elseif attackType == tonumber(BossState.FLAME_ATTACK) then
            if flameCooldown <= 0 then
                flameAttack()
                flameCooldown = frameTime + FLAME_COOL_TIME
                attackCooldown = frameTime + 4
                attackState = true
            end
        end
   end



end

-- 보스의 appear 상태 동작
function appear(bpx, bpy, bpz)
    -- 보스의 appear 상태 동작을 정의합니다.
   -- print("보스가 appear 상태입니다.")
    -- TODO: 보스의 appear 상태 동작을 구현합니다.
    -- 플레이어의 위치로 세팅
    boss_x = bpx + 2300
    boss_y = bpy + 0
    boss_z = bpz + 0
end


-- 보스의 공격 동작
function attack(frameTime)
    --print("보스가 공격합니다.")
    -- TODO: 보스의 공격 동작을 구현합니다.
    if attackType == tonumber(BossState.BASIC_ATTACK) then
        --state = BossState.BASIC_ATTACK
        motion = BossState.BASIC_ATTACK
   --     print(motion)
    --    print("BASIC_ATTACK")
    
    elseif attackType == tonumber(BossState.CLAW_ATTACK) then
        --state = BossState.BASIC_ATTACK
        motion = BossState.BASIC_ATTACK
   --     print(motion)
     --   print("CLAW_ATTACK")

    elseif attackType == tonumber(BossState.FLAME_ATTACK) then
        --state = BossState.FLAME_ATTACK
        motion = BossState.FLAME_ATTACK
       -- print(motion)
      --  print("FLAME_ATTACK")
    end

    --print(frameTime)
    attackCooldown = 4
    if frameTime >= attackCooldown then
    --    print("IDLE로 갈 준비")
        attackState = false
    end
end

-- BASIC_ATTACK 실행 함수
function basicAttack()
  --  print("보스가 BASIC_ATTACK을 실행합니다.")
    -- TODO: BASIC_ATTACK 실행 코드 작성
    motion = BossState.BASIC_ATTACK
--    print(motion)
    frameTime = 0
end

-- FLAME_ATTACK 실행 함수
function flameAttack()
 --   print("보스가 FLAME_ATTACK을 실행합니다.")
    -- TODO: FLAME_ATTACK 실행 코드 작성
    motion = BossState.FLAME_ATTACK
 --   print(motion)
    frameTime = 0
end

-- CLAW_ATTACK 실행 함수
function clawAttack()
  --  print("보스가 CLAW_ATTACK을 실행합니다.")
    -- TODO: CLAW_ATTACK 실행 코드 작성
    motion = BossState.CLAW_ATTACK
  --  print(motion)
    frameTime = 0
end

-- 죽음 상태 처리 함수
function die()
   -- print("보스가 die 상태입")
    -- TODO: 죽음 상태 처리 코드 작성
end


function nextState(bpx, bpy, bpz)
   -- print("다음으로")
    local distance = CalculateDistance(boss_x, boss_y, boss_z, bpx, bpy, bpz)

    if state == BossState.IDLE then
        if curHp <= 0 then
            state = BossState.DIE
            motion = BossState.DIE
     --       print("다이")
        elseif attackState and state ~= BossState.ATTACK then
            state = BossState.ATTACK
            frameTime = 0
            attackCooldown = 4
    --        print("공격, 프레임타임 0으로 초기화")
        end
        


    elseif state == BossState.SLEEP then
  --      print("SLEEP로 바꿉니다")
        state = BossState.SLEEP
        motion = BossState.SLEEP

        -- appear로
        if onappear == true then
            state = BossState.APPEAR
   --         print("APPEAR로 바꿉니다")
            onappear = false
        end

        if distance <= 1500 then
    --        print(onIdle)
            onIdle = true
        else
   --         print(onIdle)
            onIdle = false
        end   

        if onIdle == true then
    --        print("onIdle")
            state = BossState.IDLE
            motion = BossState.IDLE
        end


    elseif state == BossState.APPEAR then
          state = BossState.SLEEP
          motion = BossState.SLEEP



    elseif state == BossState.ATTACK then
        if attackState == false then
            state = BossState.IDLE
            motion = BossState.IDLE
     --       print("IDLE로 변경한다")
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
    updateBossAI = updateBossAI
}
