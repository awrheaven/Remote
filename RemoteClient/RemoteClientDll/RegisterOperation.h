#pragma once
class RegisterOperation
{
public:
	RegisterOperation(char bToken);
	~RegisterOperation();
	VOID SetPath(char* szPath);
	char* FindPath();
	char* FindKey();

public:
	HKEY m_Key;
	char m_KeyPath[MAX_PATH];
};

