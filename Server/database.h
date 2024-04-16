#pragma once
#include "stdafx.h"
#include <sqlext.h>

struct PLAYER_INFO
{
	std::wstring user_id;
	std::wstring user_password;
	
	INT user_level;
	INT possion;
	INT grenade;
};

struct PLAYER_TABLE
{
	SQLWCHAR id[20];
	SQLWCHAR password[50];
	SQLINTEGER user_level, possion, grenade;

	SQLLEN cb_password = 0, cb_id = 0, cb_level = 0, cb_possion = 0, cb_grenade = 0;
};

class DataBase
{
public:
	DataBase();
	~DataBase();

	void Createaccount(const char* id, const char* password);
	void Login(const char* id, const char* password);

private:
	

	SQLHENV m_henv;
	SQLHDBC m_hdbc;
	SQLHSTMT m_hstmt;
};