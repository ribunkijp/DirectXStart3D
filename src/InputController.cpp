/**********************************************************************************
* InputController.cpp
*
*
*
* LI WENHUI
* 2025/09/11
* *********************************************************************************/

#include "Pch.h"
#include "InputController.h"

InputController::InputController() {}
InputController::~InputController() {}


void InputController::OnRawMouseMove(long dx, long dy) {
    m_mouseDelta.x += dx;
    m_mouseDelta.y += dy;
}

POINT InputController::GetMouseDelta()
{
    return m_mouseDelta;
}

void InputController::EndFrame()
{
    m_mouseDelta = { 0, 0 };
}

bool InputController::IsKeyPressed(int vKey) const {
    return (GetAsyncKeyState(vKey) & 0x8000) != 0;
}