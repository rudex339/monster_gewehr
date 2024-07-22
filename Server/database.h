#pragma once
#include "stdafx.h"
#include <sqlext.h>
#include "CAPObject.h"

struct PLAYER_INFO
{
	std::wstring user_id;
	std::wstring user_password;
};

struct PLAYER_TABLE
{
	SQLWCHAR id[20];
	SQLWCHAR password[20];

	SQLINTEGER money, rifle, shotgun, sniper, l_armor, h_armor, grenade, flashbang,
		bandage, fak, injector;

	SQLLEN cb_password = 0, cb_id = 0, cb_money = 0, cb_rifle = 0, cb_shotgun = 0, 
		cb_sniper = 0, cb_l_armor = 0, cb_h_armor = 0, cb_grenade = 0, cb_flashbang = 0, 
		cb_bandage = 0, cb_fat = 0, cb_reject = 0;
};

class DataBase
{
public:
	DataBase();
	~DataBase();

	bool Createaccount(Player* player);
	bool Login(Player* player);

private:
	

	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;
};