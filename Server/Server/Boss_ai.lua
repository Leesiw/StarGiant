-- ������ ���� ����
local BossState = {
    APPEAR = 0,
    SLEEP = 1,
    IDLE = 2,
    ATTACK = 3,
    METEO_ATTACK = 4,
    CHASE = 5,
    WALK = 6,
    RUN = 7,
    DEFEND = 8,
    DIE = 9
}

-- ������ �ִϸ��̼� ����
local BossAnimation = {
    SLEEP = 0,
    IDLE = 1,
    BASIC_ATTACK = 2,
    CLAW_ATTACK = 3,
    FLAME_ATTACK = 4,
    GET_HIT = 5,
    SCREAM = 6,
    DIE = 7
}

-- ���� AI �Լ�
function Boss_Ai_lua(fTimeElapsed, CurState, player, bossHP)
    local TargetPos = player:GetPosition()
    local BossPos = GetPosition()
    local SubTarget = Vector3:Subtract(TargetPos, BossPos)
    local Dist = Vector3:Length(SubTarget)

    local LookAtPos = nil -- LookAtPosition ��� ���� �߰�

    local attactCoolTime = 3.0 -- ���� ��Ÿ�� ����
    local lastAttackTime = steady_clock:now() -- ������ ���� �ð� ����

    local a = 0 -- a ���� �߰�

    -- ���º� ���� ó��
    if CurState == BossState.APPEAR then
        SetPosition(0.0 + 10, 250.0, 640.0)
        SendPosition()

        SetState(BossState.SLEEP)
        stateStartTime = steady_clock:now()

    elseif CurState == BossState.SLEEP then
        CurMotion = BossAnimation.SLEEP
        if CurMotion ~= PastMotion then
            SendAnimation()
        end
        SendPosition()

        if Dist < 1500.0 then
            SetState(BossState.IDLE)
            stateStartTime = steady_clock:now()
        end

    elseif CurState == BossState.IDLE then
        CurMotion = BossAnimation.IDLE
        if CurMotion ~= PastMotion then
            SendAnimation()
        end
        LookAtPosition(fTimeElapsed, TargetPos)
        SendPosition()

        if Dist > 2500.0 then
            SetState(BossState.CHASE)
            stateStartTime = steady_clock:now()
        end

        if duration_cast:seconds(steady_clock:now() - stateStartTime):count() >= attactCoolTime then
            SetState(BossState.ATTACK)
            randAttact = urdAttack(dree)
            stateStartTime = steady_clock:now()
            lastAttackTime = steady_clock:now()
        elseif (bossHP / MAXBossHP) <= 0.5 and a == 0 then
            SetState(BossState.GET_HIT)
            stateStartTime = steady_clock:now()
            a = 1
        elseif bossHP <= 0 then
            SetState(BossState.DIE)
            stateStartTime = steady_clock:now()
        end

    elseif CurState == BossState.ATTACK then
        local aa = 0

        LookAtPosition(fTimeElapsed, TargetPos)
        SendPosition()

        if randAttact > 0.5 then
            CurMotion = BossAnimation.BASIC_ATTACK
            if CurState ~= PastState then
                SendAnimation()
                if player:GetHP() > 0 then
                    player:GetAttack(2)
                end
            end
            PastState = BossState.ATTACK

            -- SC_BULLET_HIT_PACKET ����
            local p = {}
            p.size = sizeof(SC_BULLET_HIT_PACKET)
            p.type = SC_BULLET_HIT
            p.data.id = -1
            p.data.hp = player:GetHP()
            scene_manager:Send(scene_num, p)

            if duration_cast:seconds(steady_clock:now() - lastAttackTime):count() >= 1.8 then
                lastAttackTime = steady_clock:now()
                SetState(BossState.IDLE)
            end

        elseif randAttact < 0.2 and a == 0 then
            CurMotion = BossAnimation.FLAME_ATTACK
            if CurState ~= PastState then
                SendAnimation()
                if player:GetHP() > 0 then
                    player:GetAttack(10)
                end
                MoveMeteo(fTimeElapsed)
                a = 1
            end
            PastState = BossState.ATTACK

            if duration_cast:seconds(steady_clock:now() - lastAttackTime):count() >= 3.5 then
                lastAttackTime = steady_clock:now()
                SetState(BossState.IDLE)

                -- SC_BULLET_HIT_PACKET ����
                local p = {}
                p.size = sizeof(SC_BULLET_HIT_PACKET)
                p.type = SC_BULLET_HIT
                p.data.id = -1
                p.data.hp = player:GetHP()
                scene_manager:Send(scene_num, p)
            end

        else
            CurMotion = BossAnimation.CLAW_ATTACK
            if CurState ~= PastState then
                SendAnimation()
                if player:GetHP() > 0 then
                    player:GetAttack(5)
                end
            end
            PastState = BossState.ATTACK

            -- SC_BULLET_HIT_PACKET ����
            local p = {}
            p.size = sizeof(SC_BULLET_HIT_PACKET)
            p.type = SC_BULLET_HIT
            p.data.id = -1
            p.data.hp = player:GetHP()
            scene_manager:Send(scene_num, p)

            if duration_cast:seconds(steady_clock:now() - lastAttackTime):count() >= 2.2 then
                lastAttackTime = steady_clock:now()
                SetState(BossState.IDLE)
            end
        end

    elseif CurState == BossState.CHASE then
        CurMotion = BossAnimation.FLY_FORWARD
        if CurMotion ~= PastMotion then
            SendAnimation()
        end
        PastState = BossState.FLY_FORWARD

        MoveBoss(fTimeElapsed, TargetPos, Dist)
        SendPosition()

        if Dist <= 2000.0 then
            SetState(BossState.IDLE)
            stateStartTime = steady_clock:now()
        end

    elseif CurState == BossState.DIE then
        CurMotion = BossAnimation.DIE
        if CurMotion ~= PastMotion then
            SendAnimation()
        end

    end

    -- ���� �� �ִϸ��̼� ������Ʈ
    PastState = CurState
    PastMotion = CurMotion

    -- ���� AI ��� ��ȯ
    return CurState, CurMotion, LookAtPos
end
