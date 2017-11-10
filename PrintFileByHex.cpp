#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define LINE_MAX_BYTES 32

int ReadInt(const char* pData,int &pos,int &value)
{
	if (pData == NULL)
	{
		return 0;
	}

	value = *((int*)(pData + pos));
	int size_len = (int)sizeof(int);	
	pos += size_len;

	return size_len; 
}

int ReadUInt(const char* pData, int &pos, unsigned int &value)
{
	if (pData == NULL)
	{
		return 0;
	}

	value = *((unsigned int*)(pData + pos));
	int size_len = (int)sizeof(unsigned int);	
	pos += size_len;

	return size_len; 
	
}

int ReadUShort(const char* pData, int &pos, unsigned short &value)
{

	if (pData == NULL)
	{
		return 0;
	}

	value = *((unsigned short*)(pData + pos));
	int size_len = (int)sizeof(unsigned short);	
	pos += size_len;

	return size_len; 
}


int ReadChar(const char* pData, int &pos,char &value)
{
	if (pData == NULL)
	{
		return 0;
	}

	value = *(pData + pos);
	int size_len = (int)sizeof(char);
	pos += size_len;	

	return size_len;
}

union StrLen{
	char vals[4];
	int iLen;
};

int ReadStr(const char* pData, int &pos, char* value, size_t num)
{
	if (pData == NULL)
	{
		return 0;
	}

	StrLen uLen;
	memset(&uLen,0,sizeof(uLen));
	for (int i = 0; i < (int)(sizeof(uLen.vals)/sizeof(uLen.vals[0])); i++)
	{
		ReadChar(pData,pos,uLen.vals[i]);
		
		if (uLen.vals[i] & (1 << 7))
		{
			uLen.vals[i] = uLen.vals[i] & ~(1 << 7);
		}else{
			break;
		}
	}
	
	int i = 0;
	for (;i < uLen.iLen;i++)
	{
		if (i >= (int)num - 1)
		{
			break;
		}	
		ReadChar(pData,pos,*(value + i));	
	} 

	*(value + i) = 0;

	int size_len = i * sizeof(char);
	//pos += size_len;

	return size_len;
}

int SimpleParseVideo(const char* pData)
{
	if (pData == NULL)
	{
		return -1;
	}
	
	int pos = 0;
	int iRet = 0;

	//brief info follow
	unsigned short brief_len;
 	iRet = ReadUShort(pData,pos,brief_len);
	std::cout<<"brief_len: "<<brief_len<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;
	 
	int version;
	iRet = ReadInt(pData,pos,version);
	std::cout<<"version: "<<version<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;
	
	char battleType;
	iRet = ReadChar(pData,pos,battleType);
	std::cout<<"battleType: "<<(int)battleType<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl; 

	char winFlag;
	iRet = ReadChar(pData,pos,winFlag);
	std::cout<<"winflag: "<<(int)winFlag<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;
	
	char loseReason;
	iRet = ReadChar(pData,pos,loseReason);
	std::cout<<"loseReason: "<<(int)loseReason<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;
	
	int usedTime;
	iRet = ReadInt(pData,pos,usedTime);
	std::cout<<"usedTime: "<<usedTime<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	//left role info
	std::cout<<"\n"<<std::endl;

	char leftName[255] = {0};
	iRet = ReadStr(pData,pos,leftName,sizeof(leftName));
	std::cout<<"leftName: "<<leftName<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int leftLevel;
	iRet = ReadInt(pData,pos,leftLevel);
	std::cout<<"leftLevel: "<<leftLevel<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int leftUseIcon;
	iRet = ReadInt(pData,pos,leftUseIcon);
	std::cout<<"leftUseIcon: "<<leftUseIcon<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int leftWJCount;
	iRet = ReadInt(pData,pos,leftWJCount);
	std::cout<<"leftWJCount: "<<leftWJCount<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int wjAttr[6];//wjID,wjLv,wjPos,wjStage,wjStarID,wjSBStage;		
	
	std::cout<<"left wujiang: wjID, wjLv, wjPos, wjStage, wjStarID, wjSBStage"<<std::endl;		
	for ( int i = 0; i < leftWJCount; i++)
	{
		for (int j = 0; j < (int)(sizeof(wjAttr)/sizeof(wjAttr[0])); j++)			
		{
			ReadInt(pData,pos,wjAttr[j]);
			std::cout<<wjAttr[j]<<",	";
		}
		std::cout<<std::endl;
	}		
	
	//right role info
	std::cout<<"\n"<<std::endl;

	char rightName[255] = {0};
	iRet = ReadStr(pData,pos,rightName,sizeof(rightName));
	std::cout<<"rightName: "<<rightName<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int rightLevel;
	iRet = ReadInt(pData,pos,rightLevel);
	std::cout<<"rightLevel: "<<rightLevel<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int rightUseIcon;
	iRet = ReadInt(pData,pos,rightUseIcon);
	std::cout<<"rightUseIcon: "<<rightUseIcon<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int rightWJCount;
	iRet = ReadInt(pData,pos,rightWJCount);
	std::cout<<"rightWJCount: "<<rightWJCount<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	int rightWJAttr[6];//wjID,wjLv,wjPos,wjStage,wjStarID,wjSBStage;		
	
	std::cout<<"right wujiang: wjID, wjLv, wjPos, wjStage, wjStarID, wjSBStage"<<std::endl;		
	for ( int i = 0; i < rightWJCount; i++)
	{
		for (int j = 0; j < (int)(sizeof(rightWJAttr)/sizeof(rightWJAttr[0])); j++)			
		{
			ReadInt(pData,pos,rightWJAttr[j]);
			std::cout<<rightWJAttr[j]<<",	";
		}
		std::cout<<std::endl;
	}		
	

	//isWin flag
	std::cout<<"\n"<<std::endl;

	char winFlag2;
	pos = sizeof(unsigned short) + brief_len;
	iRet = ReadChar(pData,pos,winFlag2);
	std::cout<<"winFlag2: "<<(int)winFlag2<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;

	//detail info follow
	std::cout<<"\n"<<std::endl;

	unsigned int detail_len; 
	iRet = ReadUInt(pData,pos,detail_len);
	std::cout<<"detail_len: "<<detail_len<<"	size "<<iRet<<"	curr_pos "<<pos<<std::endl;
	
	
	return 0;
}

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
	
	std::cout<<"origin data("<<iFileSize<<"):"<<pTmpBuff<<std::endl;

	//try parse
	std::cout<<"\n"<<std::endl;
	SimpleParseVideo(pData);

	delete pData;
	pData = NULL;
	delete pTmpBuff;
	pTmpBuff = NULL;
	fclose(pFile);

	return 0;
}
