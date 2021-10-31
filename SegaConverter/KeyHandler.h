#pragma once
#include <iostream>
#include <fstream>

class KeyHandler {
private:
    //file pointer
    long long filePointer = 0;

    //keyset total number
    int keysetNum = -1; //max 64

    //status registers
    bool fileOpenErr = 0;
    bool fileSearchEnd = 0;
    bool hexValFound = 0;
    bool endOfKeyset = 0;

    void generateFile(std::ofstream* FlashFile);
    void seekHexPrefix(std::ifstream* KeyFile);                 //seek "0x" or ";"

public:
    uint8_t* keysetMemList[64] = { NULL };                  //keyset list, each keyset is 128bytes

    void openKey(std::ifstream* KeyFile);
    void closeKey(std::ifstream* KeyFile);
    void searchKey(std::ifstream* KeyFile);                     //open file on class KeyFile

    void saveKey(std::ofstream* FlashFile, int keyNum, int M1, int A12, int A8, int A4, int A0, int D7, int D6, int D5, int D4, int D3, int D2, int D1, int D0);
    
    int getNumOfKeys();
    uint8_t* getKeyAddr(int n);    
};