/**********************************************************************************
* Player.h
*
*
*
* LI WENHUI
* 2025/09/10
* *********************************************************************************/

#ifndef PLAYER_H
#define PLAYER_H

class Player {
public:
	Player();
	~Player();

	bool Load(
		ID3D11Device* device,
		ID3D11DeviceContext* context
	);



private:
	void InitVertexData(ID3D11Device* device, ID3D11DeviceContext* context);

private:


};







#endif
