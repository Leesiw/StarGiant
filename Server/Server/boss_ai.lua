-- 보스 등장 상태
function BossStateAppear()
    print("BossStateAppear")
    SetPosition(0.0 + 10, 250.0, 640.0) -- 위치 설정
    SendPosition() -- 위치 전송
    SetState(BossState.SLEEP) -- 상태 설정
    stateStartTime = GetCurrentTime() -- 상태 시작 시간
end

-- 보스 잠자는 상태
function BossStateSleep()
    print("BossStateSleep")
    CurMotion = BossAnimation.SLEEP
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    SendPosition()

    if Dist < 1500.0 then -- 플레이어가 가까이 왔을 때
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- 보스 대기 상태
function BossStateIdle()
    CurMotion = BossAnimation.IDLE
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    LookAtPosition(fTimeElapsed, TargetPos)
    SendPosition()

    if Dist > 2500.0 then -- 플레이어와 거리가 멀어졌을 때
        SetState(BossState.CHASE)
        stateStartTime = GetCurrentTime()
    end

    if GetCurrentTime() - stateStartTime >= attactCoolTime then -- 공격 쿨타임이 지났을 때
        SetState(BossState.ATTACK)
        randAttact = urdAttack(dree)
        stateStartTime = GetCurrentTime()
        lastAttackTime = GetCurrentTime()
    elseif bossHP / MAXBossHP <= 0.1 and a == 1 then -- 체력이 일정 비율 이하일 때 스크림 상태로 전환
        SetState(BossState.SCREAM)
        stateStartTime = GetCurrentTime()
        a = 2
    elseif bossHP / MAXBossHP <= 0.5 and a == 0 then -- 체력이 일정 비율 이하일 때 피격 상태로 전환
        SetState(BossState.GET_HIT)
        stateStartTime = GetCurrentTime()
        a = 1
    elseif bossHP <= 0 then -- 보스 체력이 0 이하일 때 사망 상태로 전환
        SetState(BossState.DIE)
        stateStartTime = GetCurrentTime()
    end
end

-- 보스 공격 상태
function BossStateAttack()
    LookAtPosition(fTimeElapsed, TargetPos)
    SendPosition()

    if randAttact > 0.5 then -- 랜덤한 공격 선택 (기본 공격)
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
    elseif randAttact < 0.2 and a == 0 then -- 랜덤한 공격 선택 (화염 공격)
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
    else -- 랜덤한 공격 선택 (발톱 공격)
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

-- 보스 스크림 상태
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

-- 보스 피격 상태
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

-- 보스 추적 상태
function BossStateChase()
    CurMotion = BossAnimation.FLY_FORWARD
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
    PastState = BossState.FLY_FORWARD

    MoveBoss(fTimeElapsed, TargetPos, Dist)
    SendPosition()

    if Dist <= 2000.0 then -- 플레이어와 일정 거리 이내로 가까워졌을 때
        SetState(BossState.IDLE)
        stateStartTime = GetCurrentTime()
    end
end

-- 보스 사망 상태
function BossStateDie()
    CurMotion = BossAnimation.DIE
    if CurMotion ~= PastMotion then
        SendAnimation()
    end
end

-- 보스 AI 함수
function BossAi(fTimeElapsed, CurState, player, bossHP)
    local TargetPos = player:GetPosition() -- 플레이어 위치
    local BossPos = GetPosition() -- 보스 위치
    local SubTarget = Vector3.Subtract(TargetPos, BossPos) -- 타겟과 보스 사이의 벡터
    local Dist = Vector3.Length(SubTarget) -- 거리 계산

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
