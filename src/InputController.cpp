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

    wchar_t buffer[128];
    swprintf_s(buffer, L"m_mouseDelta.x=%ld, m_mouseDelta.y=%ld\n", m_mouseDelta.x, m_mouseDelta.y);
    OutputDebugStringW(buffer);
}

POINT InputController::GetMouseDelta()
{
    return m_mouseDelta;
}

void InputController::EndFrame()
{
    m_mouseDelta = { 0, 0 };
}