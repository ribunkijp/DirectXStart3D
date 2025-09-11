###  CMake

```c++
Remove-Item -Recurse -Force build
    
cmake --preset x64-Debug

cmake --build  --preset x64-Debug
```

### XMVECTOR

```c++
A + B				XMVectorAdd(A, B)
A - B				XMVectorSubtract(A, B)
A * B (逐分量相乘)	 XMVectorMultiply(A, B)
A * (标量s)		   XMVectorScale(A, s)
A / B (逐分量相除)	 XMVectorDivide(A, B)

```

