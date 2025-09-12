/**********************************************************************************
* InputController.h
*
*
*
* LI WENHUI
* 2025/09/11
* *********************************************************************************/

#ifndef INPUTCONTROLLER_H
#define INPUTCONTROLLER_H

class InputController {
public:
	InputController();
	~InputController();

	void OnRawMouseMove(long dx, long dy);
	POINT GetMouseDelta();
	void EndFrame();
	bool IsKeyPressed(int vKey) const;


private:
	POINT m_mouseDelta = { 0, 0 };


};








#endif
