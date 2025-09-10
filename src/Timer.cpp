/**********************************************************************************
* Timer.cpp
*
*
*
* LI WENHUI
* 2025/09/10
* *********************************************************************************/

#include "pch.h"
#include "Timer.h"


    Timer::Timer() {
    lastTime = std::chrono::high_resolution_clock::now();
    deltaTime = 0.0f;
}

void Timer::Reset() {
    deltaTime = 0.0f;
    lastTime = std::chrono::high_resolution_clock::now();
}

void Timer::Tick() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = currentTime - lastTime;
    deltaTime = elapsed.count();
    lastTime = currentTime;
}

float Timer::GetDeltaTime() const {
    return deltaTime;
}