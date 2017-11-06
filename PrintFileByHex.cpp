#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define LINE_MAX_BYTES 32

int main(int args, char** argv)
{
	if (args < 2)
	{
		fprintf(stderr,"need specify a file\n");
		exit(1);
	}	

	FILE* pFile = fopen(argv[1],"rb");
	if (pFile == NULL)
	{
		return -1;
	}
	
	int iRet = fseek(pFile,0,SEEK_END);
	if (iRet)
	{
		fclose(pFile);
		return -2;
	}
	
	iRet = ftell(pFile);
	if (iRet < 0)
	{
		fclose(pFile);
		return -3;
	}

	int iFileSize = iRet;

	iRet = fseek(pFile,0,SEEK_SET);
	if (iRet)
	{
		fclose(pFile);
		return -4;
	}

	if (iFileSize == 0)
	{
		fclose(pFile);
		return 0;
	}

	char* pData = new char[iFileSize + 1];
	
	memset(pData,0,iFileSize +1);
	
	iRet = fread(pData,sizeof(char),iFileSize,pFile);
	if (iRet != iFileSize)
	{
		fprintf(stderr,"fread size expect(%d),real(%d)\n",iFileSize,iRet);
		fclose(pFile);
		return -5;
	}	

	int iHexDataLen = iFileSize*4 + (int)(iFileSize / LINE_MAX_BYTES) * 1;
	int iPos = 0;
	char* pTmpBuff = new char[iHexDataLen + 1];
	memset(pTmpBuff,0,iHexDataLen + 1);
	for (int i = 0; i < iFileSize && iPos < iHexDataLen; i++)
	{
		int iRet = snprintf(pTmpBuff+iPos,iHexDataLen - iPos,"\\x%02x",((*((unsigned int*)(pData + i))) & 0xFF));	
		if (iRet <= 0)
		{
			return -6;
		}

		iPos += iRet;
		
		if (((i+1) % LINE_MAX_BYTES) == 0 && iPos < iHexDataLen)
		{
			*(pTmpBuff + iPos) = '\n';		
			iPos += 1;
		}
	}
	
	std::cout<<"data("<<iFileSize<<"):"<<pTmpBuff<<std::endl;

	delete pData;
	delete pTmpBuff;
	fclose(pFile);

	return 0;
}
