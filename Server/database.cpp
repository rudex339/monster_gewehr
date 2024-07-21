#include "database.h"

void show_error(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];
	if (RetCode == SQL_INVALID_HANDLE) {
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS) {
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5)) {
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
	}
}

DataBase::DataBase() : m_hdbc{}, m_henv{}, m_hstmt{}
{
	setlocale(LC_ALL, "korean");
	std::wcout.imbue(std::locale("korean"));

	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);

		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, m_henv, &m_hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(m_hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);

				// Connect to data source  
				retcode = SQLConnect(m_hdbc, (SQLWCHAR*)L"Monster_gewehr", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);				
				if (!(SQL_SUCCESS == retcode || SQL_SUCCESS_WITH_INFO == retcode)) {
					std::cout << "실패함" << std::endl;
				}
				std::cout << "데이터베이스 연결 칸료" << std::endl;
			}
			else {
				std::cout << "실패함" << std::endl;
			}
		}
		else {
			std::cout << "실패함" << std::endl;
		}
	
	}
	else {
		std::cout << "실패함" << std::endl;
	}
}

DataBase::~DataBase()
{
	SQLDisconnect(m_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, m_hdbc);
	SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
}

bool DataBase::Createaccount(Player* player)
{
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);

	std::wstring c_id{ player->GetName().c_str(), player->GetName().c_str() + player->GetName().length()};
	std::wstring c_password{ player->GetPassword().c_str(), player->GetPassword().c_str() + player->GetPassword().length()};

	std::wcout << c_id << " " << c_password << std::endl;

	/*std::wstring query = std::format(L"INSERT INTO user_data (user_id, user_password, user_level, possion, grenade) VALUES ('{0}', '{1}', 1, 10, 10)",
		c_id, c_password);*/

	std::wstring query = std::format(L"CALL register_user ('{0}', '{1}')",
		c_id, c_password);

	retcode = SQLExecDirect(m_hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (!(SQL_SUCCESS == retcode || SQL_SUCCESS_WITH_INFO == retcode)) {
		std::cout << "실패함" << std::endl;
		show_error(m_hstmt, SQL_HANDLE_STMT, retcode);
		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
		return false;
	}

	SQLCancel(m_hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
	return true;
}

bool DataBase::Login(Player* player)
{
	SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);

	std::wstring c_id;
	std::wstring c_password;
	std::string u_id = player->GetName();
	std::string u_password = player->GetPassword();

	c_id.assign(u_id.begin(), u_id.end());
	c_password.assign(u_password.begin(), u_password.end());


	std::wstring query = std::format(L"CALL try_login ('{0}', '{1}')",
		c_id, c_password);

	retcode = SQLExecDirect(m_hstmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (!(SQL_SUCCESS == retcode || SQL_SUCCESS_WITH_INFO == retcode)) {
		std::cout << "실패함" << std::endl;
		show_error(m_hstmt, SQL_HANDLE_STMT, retcode);
		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
		return false;
	}

	PLAYER_TABLE player_table{};
	PLAYER_INFO player_data{};

	retcode = SQLBindCol(m_hstmt, 1, SQL_C_WCHAR, &player_table.id, 20, &player_table.cb_id);
	retcode = SQLBindCol(m_hstmt, 2, SQL_C_WCHAR, &player_table.password, 20, &player_table.cb_password);
	retcode = SQLBindCol(m_hstmt, 3, SQL_C_SLONG, &player_table.money, 4, &player_table.cb_money);
	retcode = SQLBindCol(m_hstmt, 4, SQL_C_SLONG, &player_table.rifle, 4, &player_table.cb_rifle);
	retcode = SQLBindCol(m_hstmt, 5, SQL_C_SLONG, &player_table.shotgun, 4, &player_table.cb_shotgun);
	retcode = SQLBindCol(m_hstmt, 6, SQL_C_SLONG, &player_table.sniper, 4, &player_table.cb_sniper);
	retcode = SQLBindCol(m_hstmt, 7, SQL_C_SLONG, &player_table.l_armor, 4, &player_table.cb_l_armor);
	retcode = SQLBindCol(m_hstmt, 8, SQL_C_SLONG, &player_table.h_armor, 4, &player_table.cb_h_armor);
	retcode = SQLBindCol(m_hstmt, 9, SQL_C_SLONG, &player_table.grenade, 4, &player_table.cb_grenade);
	retcode = SQLBindCol(m_hstmt, 10, SQL_C_SLONG, &player_table.flashbang, 4, &player_table.cb_flashbang);
	retcode = SQLBindCol(m_hstmt, 11, SQL_C_SLONG, &player_table.bandage, 4, &player_table.cb_bandage);
	retcode = SQLBindCol(m_hstmt, 12, SQL_C_SLONG, &player_table.fat, 4, &player_table.cb_fat);
	retcode = SQLBindCol(m_hstmt, 13, SQL_C_SLONG, &player_table.reject, 4, &player_table.cb_reject);


	while (true) {
		retcode = SQLFetch(m_hstmt);
		show_error(m_hstmt, SQL_HANDLE_STMT, retcode);

		if (SQL_SUCCESS == retcode || SQL_SUCCESS_WITH_INFO == retcode) {
			player_data.user_id = player_table.id;
			player_data.user_password = player_table.password;
			
			std::wstring temp_id{ L"null" };
			std::wstring temp_password{ L"null" };
			if (temp_id == player_data.user_id) {
				SQLCancel(m_hstmt);
				SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
				std::cout << "id / password가 일치하지 않습니다." << std::endl;
				return false;
			}
			
		}
		else {			
			break;
		}

		SQLCancel(m_hstmt);
		SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
		return true;
	}
	std::cout << "FAIL" << std::endl;
	SQLCancel(m_hstmt);
	SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);
	return false;
}
