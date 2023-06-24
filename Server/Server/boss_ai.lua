-- ���� ���� ����
function BossStateAppear()
    print("BossStateAppear")
    SetPosition(0.0 + 10, 250.0, 640.0) -- ��ġ ����
    SendPosition() -- ��ġ ����
    SetState(BossState.SLEEP) -- ���� ����
    stateStartTime = GetCurrentTime() -- ���� ���� �ð�
end

-- ���� ���ڴ� ����
function BossStateSleep()
    print("BossStateSleep")
    CurMotion = BossAnimation.SLEEP
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    SendPosition()

    if Dist < 1500.0 then -- �÷��̾ ������ ���� ��
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- ���� ��� ����
function BossStateIdle()
    CurMotion = BossAnimation.IDLE
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    LookAtPosition(fTimeElapsed, TargetPos)
    SendPosition()

    if Dist > 2500.0 then -- �÷��̾�� �Ÿ��� �־����� ��
        SetState(BossState.CHASE)
        stateStartTime = GetCurrentTime()
    end

    if GetCurrentTime() - stateStartTime >= attactCoolTime then -- ���� ��Ÿ���� ������ ��
        SetState(BossState.ATTACK)
        randAttact = urdAttack(dree)
        stateStartTime = GetCurrentTime()
        lastAttackTime = GetCurrentTime()
    elseif bossHP / MAXBossHP <= 0.1 and a == 1 then -- ü���� ���� ���� ������ �� ��ũ�� ���·� ��ȯ
        SetState(BossState.SCREAM)
        stateStartTime = GetCurrentTime()
        a = 2
    elseif bossHP / MAXBossHP <= 0.5 and a == 0 then -- ü���� ���� ���� ������ �� �ǰ� ���·� ��ȯ
        SetState(BossState.GET_HIT)
        stateStartTime = GetCurrentTime()
        a = 1
    elseif bossHP <= 0 then -- ���� ü���� 0 ������ �� ��� ���·� ��ȯ
        SetState(BossState.DIE)
        stateStartTime = GetCurrentTime()
    end
end

-- ���� ���� ����
function BossStateAttack()
    LookAtPosition(fTimeElapsed, TargetPos)
    SendPosition()

    if randAttact > 0.5 then -- ������ ���� ���� (�⺻ ����)
        CurMotion = BossAnimation.BASIC_ATTACK

        if CurState ~= PastState then
            SendAnimation()
            if player:GetHP() > 0 then
                player:GetAttack(2)
            end
        end
        PastState = BossState.ATTACK

        local p = {
            size = sizeof(SC_BULLET_HIT_PACKET),
            type = SC_BULLET_HIT,
            data = {
                id = -1,
                hp = player:GetHP()
            }
        }
        scene_manager.Send(scene_num, p)

        if GetCurrentTime() - lastAttackTime >= 1.8 then
            lastAttackTime = GetCurrentTime()
            SetState(BossState.IDLE)
        end
    elseif randAttact < 0.2 and a == 0 then -- ������ ���� ���� (ȭ�� ����)
        CurMotion = BossAnimation.FLAME_ATTACK
        if CurState ~= PastState then
            SendAnimation()
            if player:GetHP() > 0 then
                player:GetAttack(10)
            end
            MeteoAttack(fTimeElapsed, TargetPos)
            a = 1
        end
        PastState = BossState.ATTACK

        if GetCurrentTime() - lastAttackTime >= 3.5 then
            lastAttackTime = GetCurrentTime()
            SetState(BossState.IDLE)

            local p = {
                size = sizeof(SC_BULLET_HIT_PACKET),
                type = SC_BULLET_HIT,
                data = {
                    id = -1,
                    hp = player:GetHP()
                }
            }
            scene_manager.Send(scene_num, p)
        end
    else -- ������ ���� ���� (���� ����)
        CurMotion = BossAnimation.CLAW_ATTACK
        if CurState ~= PastState then
            SendAnimation()
            if player:GetHP() > 0 then
                player:GetAttack(5)
            end
        end
        PastState = BossState.ATTACK

        local p = {
            size = sizeof(SC_BULLET_HIT_PACKET),
            type = SC_BULLET_HIT,
            data = {
                id = -1,
                hp = player:GetHP()
            }
        }
        scene_manager.Send(scene_num, p)

        if GetCurrentTime() - lastAttackTime >= 2.2 then
            lastAttackTime = GetCurrentTime()
            SetState(BossState.IDLE)
        end
    end
end

-- ���� ��ũ�� ����
function BossStateScream()
    CurMotion = BossAnimation.SCREAM
    if CurMotion ~= PastMotion then
        SendAnimation()
    end

    if GetCurrentTime() - stateStartTime >= 3 then
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- ���� �ǰ� ����
function BossStateGetHit()
    CurMotion = BossAnimation.GET_HIT
    if CurMotion ~= PastMotion then
        SendAnimation()
    end

    if GetCurrentTime() - stateStartTime >= 1 then
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- ���� ���� ����
function BossStateChase()
    CurMotion = BossAnimation.FLY_FORWARD
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    PastState = BossState.FLY_FORWARD

    MoveBoss(fTimeElapsed, TargetPos, Dist)
    SendPosition()

    if Dist <= 2000.0 then -- �÷��̾�� ���� �Ÿ� �̳��� ��������� ��
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- ���� ��� ����
function BossStateDie()
    CurMotion = BossAnimation.DIE
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
end

-- ���� AI �Լ�
function BossAi(fTimeElapsed, CurState, player, bossHP)
    local TargetPos = player:GetPosition() -- �÷��̾� ��ġ
    local BossPos = GetPosition() -- ���� ��ġ
    local SubTarget = Vector3.Subtract(TargetPos, BossPos) -- Ÿ�ٰ� ���� ������ ����
    local Dist = Vector3.Length(SubTarget) -- �Ÿ� ���

    local a = 0
    print("BossAi")

    if CurState == BossState.APPEAR then
        BossStateAppear()
    elseif CurState == BossState.SLEEP then
        BossStateSleep()
    elseif CurState == BossState.IDLE then
        BossStateIdle()
    elseif CurState == BossState.ATTACK then
        BossStateAttack()
    elseif CurState == BossState.SCREAM then
        BossStateScream()
    elseif CurState == BossState.GET_HIT then
        BossStateGetHit()
    elseif CurState == BossState.CHASE then
        BossStateChase()
    elseif CurState == BossState.DIE then
        BossStateDie()
    end

    if CurState ~= BossState.ATTACK and CurState ~= BossState.CHASE then
        PastState = CurState
    end

    if a == 1 then
        MoveMeteo(fTimeElapsed)
    end
end
