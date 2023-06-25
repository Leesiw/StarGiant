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

-- ������ �ʱ� ����

state = BossState.IDLE
motion = BossState.IDLE
MaxHp = 100

boss_x = 1000
boss_y = 1000
boss_z = 1000


curHp = MaxHp


-- ���� ���¿� ��Ÿ�� ���� �ʱ�ȭ
attackState = false
attackCooldown = 0
clawCooldown = 0
flameCooldown = 0

-- ������ �ð� ���� ���� �ʱ�ȭ
frameTime = 0
frameInterval = 0


-- ������ AI ������Ʈ �Լ�
function updateBossAI(hp, bpx, bpy, bpz, frameTime)
    curHp = hp + 10
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


-- ������ idle ���� ����
function idle(frameTime)
    print("idle ����")
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


-- ������ appear ���� ����
function appear(bpx, bpy, bpz)
    -- ������ appear ���� ������ �����մϴ�.
    print("������ appear �����Դϴ�.")
    -- TODO: ������ appear ���� ������ �����մϴ�.
    -- �÷��̾��� ��ġ�� ����
    boss_x = bpx + 1000
    boss_y = bpy + 1000
    boss_z = bpz + 1000
end


-- ������ ���� ����
function attack(frameTime)
    print("������ �����մϴ�.")
    -- TODO: ������ ���� ������ �����մϴ�.

    if frameTime >= attackCooldown then
        state = BossState.IDLE
        attackState = false
    end
end


-- BASIC_ATTACK ���� �Լ�
function basicAttack()
    print("������ BASIC_ATTACK�� �����մϴ�.")
    -- TODO: BASIC_ATTACK ���� �ڵ� �ۼ�
end

-- FLAME_ATTACK ���� �Լ�
function flameAttack()
    print("������ FLAME_ATTACK�� �����մϴ�.")
    -- TODO: FLAME_ATTACK ���� �ڵ� �ۼ�
end

-- CLAW_ATTACK ���� �Լ�
function clawAttack()
    print("������ CLAW_ATTACK�� �����մϴ�.")
    -- TODO: CLAW_ATTACK ���� �ڵ� �ۼ�
end

-- ���� ���� ó�� �Լ�
function die()
    print("������ �׾����ϴ�.")
    -- TODO: ���� ���� ó�� �ڵ� �ۼ�
end


function nextState()
    print("��������")

    if state == BossState.IDLE then
        if curHp <= 0 then
            state = BossState.DIE
            print("����")
        elseif attackState then
            state = BossState.ATTACK
            print("����")
        end
        -- �ٸ� ���ǿ� ���� ���� ���� ������ �߰��ϼ���
    elseif state == BossState.ATTACK then
        if someCondition then
            state = BossState.IDLE
            print("IDLE")
        end
        -- �ٸ� ���ǿ� ���� ���� ���� ������ �߰��ϼ���
    end
end

-- ������ Ư�� ��ġ�� �ٶ󺸵��� �ϴ� �Լ�
function LookAtPosition()
    return true;
end

return {
    updateBossAI = updateBossAI
}
