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




-- ������ �ʱ� ����
state = BossState.SLEEP
motion = BossState.SLEEP
MaxHp = 100

boss_x = 5000   --��ġ
boss_y = 5000
boss_z = 5000


curHp = MaxHp


--true false
onappear = true
onIdle = false


-- ���� ���¿� ��Ÿ�� ���� �ʱ�ȭ
attackState = false
attackCooldown = 4
clawCooldown = 0
flameCooldown = 0

-- ������ �ð� ���� ���� �ʱ�ȭ
frameTime = 0
frameInterval = 0


-- ������ AI ������Ʈ �Լ�
function updateBossAI(hp, bpx, bpy, bpz, elapsedTime)
    curHp = hp
    frameTime = frameTime + elapsedTime
    clawCooldown = clawCooldown - elapsedTime
    flameCooldown = flameCooldown - elapsedTime

    --print(bpx, bpy, bpz)

    if state == BossState.SLEEP then
        -- ������ SLEEP ������ �����մϴ�.
        sleep()
    elseif state == BossState.APPEAR then
        -- ������ ��Ÿ�� ���� ������ �����մϴ�.
        appear(bpx, bpy, bpz)
    elseif state == BossState.IDLE then
        idle(frameTime)
    elseif state == BossState.DIE then
        -- ������ ü���� 0 ������ �� ���� ���� ó��
        if curHp <= 0 then
            die()
        end
    elseif state == BossState.ATTACK then
        -- ������ ATTACK ���� ������ �����մϴ�.
        attack(frameTime)
    end
    -- ������ ���� ���¸� �����մϴ�.
    nextState( bpx, bpy, bpz)
end

-- ������ ���ڴ� ����
function sleep()
    -- ������ ���ڴ� ������ ������ �����մϴ�.
    --print("������ sleep ���Դϴ�.")
    -- TODO: ������ ���ڴ� ������ �����մϴ�.

end


-- ������ idle ���� ����
function idle(frameTime)
    --print("idle ����")
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

-- ������ appear ���� ����
function appear(bpx, bpy, bpz)
    -- ������ appear ���� ������ �����մϴ�.
   -- print("������ appear �����Դϴ�.")
    -- TODO: ������ appear ���� ������ �����մϴ�.
    -- �÷��̾��� ��ġ�� ����
    boss_x = bpx + 2300
    boss_y = bpy + 0
    boss_z = bpz + 0
end


-- ������ ���� ����
function attack(frameTime)
    --print("������ �����մϴ�.")
    -- TODO: ������ ���� ������ �����մϴ�.
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
    --    print("IDLE�� �� �غ�")
        attackState = false
    end
end

-- BASIC_ATTACK ���� �Լ�
function basicAttack()
  --  print("������ BASIC_ATTACK�� �����մϴ�.")
    -- TODO: BASIC_ATTACK ���� �ڵ� �ۼ�
    motion = BossState.BASIC_ATTACK
--    print(motion)
    frameTime = 0
end

-- FLAME_ATTACK ���� �Լ�
function flameAttack()
 --   print("������ FLAME_ATTACK�� �����մϴ�.")
    -- TODO: FLAME_ATTACK ���� �ڵ� �ۼ�
    motion = BossState.FLAME_ATTACK
 --   print(motion)
    frameTime = 0
end

-- CLAW_ATTACK ���� �Լ�
function clawAttack()
  --  print("������ CLAW_ATTACK�� �����մϴ�.")
    -- TODO: CLAW_ATTACK ���� �ڵ� �ۼ�
    motion = BossState.CLAW_ATTACK
  --  print(motion)
    frameTime = 0
end

-- ���� ���� ó�� �Լ�
function die()
   -- print("������ die ������")
    -- TODO: ���� ���� ó�� �ڵ� �ۼ�
end


function nextState(bpx, bpy, bpz)
   -- print("��������")
    local distance = CalculateDistance(boss_x, boss_y, boss_z, bpx, bpy, bpz)

    if state == BossState.IDLE then
        if curHp <= 0 then
            state = BossState.DIE
            motion = BossState.DIE
     --       print("����")
        elseif attackState and state ~= BossState.ATTACK then
            state = BossState.ATTACK
            frameTime = 0
            attackCooldown = 4
    --        print("����, ������Ÿ�� 0���� �ʱ�ȭ")
        end
        


    elseif state == BossState.SLEEP then
  --      print("SLEEP�� �ٲߴϴ�")
        state = BossState.SLEEP
        motion = BossState.SLEEP

        -- appear��
        if onappear == true then
            state = BossState.APPEAR
   --         print("APPEAR�� �ٲߴϴ�")
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
     --       print("IDLE�� �����Ѵ�")
            frameTime = 0
        end
     
    end
end

-- idle�� �ٲ�
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
