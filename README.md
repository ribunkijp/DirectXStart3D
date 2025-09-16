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

    



```

