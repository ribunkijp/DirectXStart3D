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

private:
	POINT m_mouseDelta;


};








#endif
