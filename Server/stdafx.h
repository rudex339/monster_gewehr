#pragma once
// tcp/ip�� �����
#define _WINSOCK_DEPRECATED_NO_WARNINGS // ���� ���� API ��� �� ��� ����
#define _CRT_SECURE_NO_WARNINGS

#include <winsock2.h> // ����2 ���� ���
#include <ws2tcpip.h> // ����2 Ȯ�� ���

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...

#pragma comment(lib, "ws2_32") // ws2_32.lib ��ũ

// directx 12 ���
#pragma comment(lib, "d3d12.lib")
#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXColors.h>
#include <DirectXCollision.h>


#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <array>
#include <unordered_map>
#include <locale.h>
#include <format>
#include "protocol.h"

using namespace DirectX;

namespace Vector3
{
	inline XMFLOAT3 Add(const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&f1) + XMLoadFloat3(&f2));
		return result;
	}

	inline XMFLOAT3 Sub(const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&f1) - XMLoadFloat3(&f2));
		return result;
	}

	inline XMFLOAT3 Mul(const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMLoadFloat3(&f1) * XMLoadFloat3(&f2));
		return result;
	}

	inline float Dot(const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Dot(XMLoadFloat3(&f1), XMLoadFloat3(&f2)));
		return result.x;
	}

	inline XMFLOAT3 Cross(const XMFLOAT3& f1, const XMFLOAT3& f2)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Cross(XMLoadFloat3(&f1), XMLoadFloat3(&f2)));
		return result;
	}

	inline XMFLOAT3 Normalize(const XMFLOAT3& f)
	{
		XMFLOAT3 result;
		XMStoreFloat3(&result, XMVector3Normalize(XMLoadFloat3(&f)));
		return result;
	}


}

void err_quit(const char* msg);
void err_display(const char* msg);
void err_display(int errnum);