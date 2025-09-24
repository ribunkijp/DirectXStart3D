###  CMake

```c++
Remove-Item -Recurse -Force build
    
cmake --preset x64-Debug

cmake --build  --preset x64-Debug
```

### vector	

```c++
A + B				XMVectorAdd(A, B)
A - B				XMVectorSubtract(A, B)
A * B (逐分量相乘)	 XMVectorMultiply(A, B)
A * (标量s)		   XMVectorScale(A, s)
A / B (逐分量相除)	 XMVectorDivide(A, B)
    
DirectX::XMVector3LengthSq(v)   // x^2 + y^2 + z^2
DirectX::XMVector3Length(v)   // √(x^2 + y^2 + z^2)   // sqrt スクエアルート
    
DirectX::XMVector3Normalize // vector 正規化
    
DirectX::XMMATRIX rx = DirectX::XMMatrixRotationX(pitch); // 创建绕x轴的旋转矩阵 参数类型float radians
DirectX::XMMATRIX ry = DirectX::XMMatrixRotationY(yaw); // 创建绕Y轴的旋转矩阵
DirectX::XMMATRIX rz = DirectX::XMMatrixRotationZ(roll); // 创建绕z轴的旋转矩阵
DirectX::XMMATRIX r = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll)//绕X/Y/Z的组合旋转矩阵
    
DirectX::XMStoreFloat3(&DirectX::XMFLOAT3, DirectX::XMVECTOR) // XMVECTOR -> XMFLOAT3
DirectX::XMLoadFloat3(&DirectX::XMFLOAT3) // XMFLOAT3 -> XMVECTOR
    
DirectX::XMVectorZero();// 零向量 (0.0f, 0.0f, 0.0f, 0.0f)


DirectX::XMVectorLerp(currentVel, targetVel, smoothingFactor); //線形補間 / リニア補間 
//currentVel = (targetVel - currentVel) * smoothingFactor;

DirectX::XMQuaternionSlerp(currentVel, targetVel, smoothingFactor); //球面线性插值 在两个旋转之间平滑过渡
//currentVel = (targetVel - currentVel) * smoothingFactor;

DirectX::XMVECTOR prevS, prevR, prevT;
DirectX::XMMatrixDecompose(&prevS, &prevR, &prevT, matrix);//分解第4个参数(4x4矩阵)，然后把值写入前三个参数(scale, rotation, transform)的地址

DirectX::XMVECTOR scaleVec = XMVectorSet(2.0f, 1.0f, 0.5f, 0.0f)
DirectX::XMMatrixScalingFromVector(scaleVec) // 参数为XMVECTOR，输出一个4x4缩放矩阵
    
DirectX::XMVECTOR r = DirectX::XMQuaternionRotationAxis(XMVectorSet(0,1,0,0), DirectX::XMConvertToRadians(90.0f)); // 参数为单位话旋转轴，弧度制，返回旋转4元数(x, y, z, w)
DirectX::XMMatrixRotationQuaternion(r);// 输入一个单位化的四元数，输出4x4旋转矩阵

DirectX::XMMatrixTranslationFromVector(t)// 输出4x4平移矩阵

DirectX::XMConvertToRadians(90.0f)//输入角度值，返回弧度值



```

