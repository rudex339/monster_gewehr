#pragma once
#include "stdafx.h"
#include "CAPObject.h"

DWORD WINAPI Calculate(LPVOID arg);
DWORD WINAPI ProcessClient(LPVOID arg);

std::array<Player, MAX_CLIENT> players;