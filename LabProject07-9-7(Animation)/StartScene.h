#pragma once
#include "Shader.h"
#include "Player.h"


class CStartScene
{
public:
    CStartScene() {};
    ~CStartScene() {};


    bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
    bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

};