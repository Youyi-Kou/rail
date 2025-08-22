#include "stdafx.h"
#include "SHA.h"

std::string GetFileSHA1(std::string strHashFile)
{
	char szCmd[5000] = { 0 };
	char result[1024] = { 0 };
	sprintf(szCmd, "certutil -hashfile %s SHA1", strHashFile.c_str());

	FILE* file = NULL;
	if ((file = _popen(szCmd, "r")) != NULL)
	{
		if (fgets(result, 1024, file) != NULL)
		{
			if (fgets(result, 1024, file) != NULL)
			{
				_pclose(file);
			}
		}
	} 	
	std::string str = std::string(result, strlen(result) - 1);
	return str;
}

std::string GetFileSHA256(std::string strHashFile)
{
	char szCmd[5000] = { 0 };
	char result[1024] = { 0 };
	sprintf(szCmd, "certutil -hashfile %s SHA256", strHashFile.c_str());

	FILE* file = NULL;
	if ((file = _popen(szCmd, "r")) != NULL)
	{
		if (fgets(result, 1024, file) != NULL)
		{
			if (fgets(result, 1024, file) != NULL)
			{
				_pclose(file);
			}
		}
	}
	std::string str = std::string(result, strlen(result) - 1);
	return str;
}