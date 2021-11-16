#pragma once
#include <iostream>
#include <fstream>
#include "KeyHandler.h"

void KeyHandler::openKey(std::ifstream* KeyFile) { //open file on class KeyFile
    KeyFile->open("key.txt", std::ios::in);

    fileOpenErr = 0;

    if (KeyFile->fail() == 1) {
        fileOpenErr = 1;
    }
}

void KeyHandler::closeKey(std::ifstream* KeyFile) { //open file on class KeyFile
    for (int i = 0; i <= keysetNum; i++) { //생성된 배열 모두 해제
        delete[] keysetMemList[i];
    }

    if (KeyFile->is_open()) { //열렸으면 닫기
        KeyFile->close();
    }
}

int KeyHandler::getNumOfKeys() {
    return keysetNum + 1;
}

uint8_t* KeyHandler::getKeyAddr(int n) {
    if (n > keysetNum) {
        return NULL;
    }
    else {
        return keysetMemList[n];
    }
}

void KeyHandler::searchKey(std::ifstream* KeyFile) {                    //open file on class KeyFile
    if (fileOpenErr == 0) {
        char inCharBuf[2] = { '0', '0' };
        uint8_t intBufH = 0;
        uint8_t intBufL = 0;
        int writePointer = 0;

        std::cout << "Searching keys..." << "\n\n";

        KeyFile->seekg(0, std::ios::beg);                               //파일 포인터 처음으로 이동
        filePointer = 0;

        seekHexPrefix(KeyFile);                                         //키 파일 검색하기

        while (fileSearchEnd == 0 && keysetNum < 63) {                  //파일 끝이 아니거나 키 숫자가 64개 이하면 while돌기
            if (hexValFound == 1) {                                     //만약 0x를 발견했으면
                keysetNum++;
                std::cout << "Found key " << keysetNum << std::endl;
                keysetMemList[keysetNum] = new uint8_t[128];            //unit8 128바이트 공간 할당 및 시작주소 넣기

                inCharBuf[0] = { '0' };                                 //입력 버퍼 초기화         
                inCharBuf[1] = { '0' };
                writePointer = 0;                                       //쓰기 포인터 초기화

                while (fileSearchEnd == 0 && endOfKeyset == 0 && writePointer < 128) {        //파일 끝이거나 키셋 끝일때까지 while돌기
                    if (hexValFound == 1) {                             //0x를 찾았다면 
                        KeyFile->get(inCharBuf[0]);                     //상위 4비트 로드
                        KeyFile->get(inCharBuf[1]);                     //하위 4비트 로드

                        intBufH =
                            ((inCharBuf[0] >= 'A') ?                    //상위 4비트 char->uint8 변환
                                (inCharBuf[0] >= 'a') ?
                                    (inCharBuf[0] - 'a' + 10) : 
                                    (inCharBuf[0] - 'A' + 10) :
                                (inCharBuf[0] - '0')) << 4;
                        intBufL =
                            (inCharBuf[1] >= 'A') ?                     //하위 4비트 char->uint8 변환
                                (inCharBuf[1] >= 'a') ?
                                    (inCharBuf[1] - 'a' + 10) : 
                                    (inCharBuf[1] - 'A' + 10) :
                                (inCharBuf[1] - '0');

                        *(*(keysetMemList + keysetNum) + writePointer) = intBufH + intBufL; //쓰기
                        writePointer++;

                    }

                    seekHexPrefix(KeyFile);
                }

                if (writePointer < 128) {
                    std::cout << "ERR: Keyset " << keysetNum << " is shorter than expected" << "\n\n";

                    delete[] keysetMemList[keysetNum];                          //새로 생성된 배열 폐기
                    keysetNum--;
                }
                else {
                    if (endOfKeyset == 1) {
                        std::cout << "Keyset " << keysetNum << " is copied successfully" << "\n\n";
                    }
                    else if (hexValFound == 1) {
                        std::cout << "ERR: Keyset " << keysetNum << " is longer than expected" << "\n\n";

                        delete[] keysetMemList[keysetNum];                      //새로 생성된 배열 폐기
                        keysetNum--;

                        while (fileSearchEnd == 0 && endOfKeyset == 0) {
                            seekHexPrefix(KeyFile);                             //세미콜론 나타나거나 파일 끝날때까지 서치
                        }
                    }
                    else {

                    }
                }
            }
            seekHexPrefix(KeyFile);
        }

        if (keysetNum == 63) {
            std::cout << "Can't store more keyset" << std::endl;
        }

        if (fileSearchEnd == 1) {
            std::cout << "End of the file" << std::endl;
        }
    }
    else {
        std::cout << "File open error" << std::endl;
    }

}

void KeyHandler::saveKey(std::ofstream* FlashFile, int keysetNum, int M1, int A12, int A8, int A4, int A0, int D7, int D6, int D5, int D4, int D3, int D2, int D1, int D0) {
    /*
      counter   Z80    flash
        D12     /M1     A9
        D11     A12     A0
        D10     A8      A7
        D9      A4      A12
        D8      A0      A1
        D7      D7      A4
        D6      D6      A3
        D5      D5      A5
        D4      D4      A8
        D3      D3      A6
        D2      D2      A2
        D1      D1      A10
        D0      D0      A11
    */

    //source 0x00 - 0x7F target 0x0000 - 0x1FFF

    if (FlashFile->is_open() == 0) {   //없으면 생성
        generateFile(FlashFile);
    }

    char flashKey[8192] = { 0 };

    for (unsigned int COUNTER = 0; COUNTER < 0x2000; COUNTER++) {
        unsigned int KEYSET_ADDRESS = 0;
        bool KEYSET_XOR = 0;
        char CPU_DATA = 0;

        char KEY_BUFFER = 0;

        unsigned int FLASH_ADDRESS = 0;
        char FLASH_DATA = 0;


        KEYSET_ADDRESS =    (((COUNTER >> 3) & 1) +        //D3
                            (((COUNTER >> 5) & 1) << 1) +  //D5
                            (((COUNTER >> 12) & 1) << 2) + ///M1
                            (((COUNTER >> 8) & 1) << 3) +  //A0
                            (((COUNTER >> 9) & 1) << 4) +  //A4
                            (((COUNTER >> 10) & 1) << 5) + //A8
                            (((COUNTER >> 11) & 1) << 6)); //A12
        KEYSET_XOR =        ((COUNTER >> 7) & 1);          //D7

        CPU_DATA =          ((COUNTER & 1) +               //D0
                            (((COUNTER >> 1) & 1) << 1) +  //D1
                            (((COUNTER >> 2) & 1) << 2) +  //D2
                            (((COUNTER >> 4) & 1) << 4) +  //D4
                            (((COUNTER >> 6) & 1) << 6));  //D6


        if (KEYSET_XOR == 1) {
            KEYSET_ADDRESS = KEYSET_ADDRESS ^ 3;                //XOR비트 켜졌으면 하위 2비트 반전시키기
        }

        KEY_BUFFER = *(*(keysetMemList + keysetNum) + KEYSET_ADDRESS);     //MAME 키 데이터 버퍼로 가져오기

        FLASH_DATA =    CPU_DATA +
                        (((((KEY_BUFFER >> 3) & 1) ^ KEYSET_XOR) << 3) + //D3
                        ((((KEY_BUFFER >> 5) & 1) ^ KEYSET_XOR) << 5) +  //D5
                        ((((KEY_BUFFER >> 7) & 1) ^ KEYSET_XOR) << 7));  //D7

        FLASH_ADDRESS = ((((COUNTER >> 0) & 1) << D0) +  //D0
                        (((COUNTER >> 1) & 1) << D1) +   //D1
                        (((COUNTER >> 2) & 1) << D2) +   //D2
                        (((COUNTER >> 3) & 1) << D3) +   //D3
                        (((COUNTER >> 4) & 1) << D4) +   //D4
                        (((COUNTER >> 5) & 1) << D5) +   //D5
                        (((COUNTER >> 6) & 1) << D6) +   //D6
                        (((COUNTER >> 7) & 1) << D7) +   //D7
                        (((COUNTER >> 8) & 1) << A0) +   //A0
                        (((COUNTER >> 9) & 1) << A4) +   //A4
                        (((COUNTER >> 10) & 1) << A8) +  //A5
                        (((COUNTER >> 11) & 1) << A12) + //A12
                        (((COUNTER >> 12) & 1) << M1));  //M1

        flashKey[FLASH_ADDRESS] = FLASH_DATA;
    }

    FlashFile->seekp((long long)keysetNum * 8192, std::ios::beg);       //포인터 세팅
    FlashFile->write((char*)flashKey, sizeof(flashKey));

}

void KeyHandler::seekHexPrefix(std::ifstream* KeyFile) {            //seek "0x" or ";"
    char inBuf[2] = { '0', '0' };

    while ((inBuf[0] != '0' || inBuf[1] != 'x') && inBuf[1] != ';' && KeyFile->eof() == 0) { //4 input AND
        inBuf[0] = inBuf[1];
        KeyFile->get(inBuf[1]);
    }

    if (KeyFile->eof() == 0) {                          //파일 끝이 아니면
        if (inBuf[1] == ';') {                          //만약 세미콜론 발견 시
            endOfKeyset = 1;
            hexValFound = 0;
            fileSearchEnd = 0;
        }
        else {                                          //0x를 찾음
            endOfKeyset = 0;
            hexValFound = 1;
            fileSearchEnd = 0;
        }
    }
    else {                                              //파일 끝이면 그냥 끝
        endOfKeyset = 0;
        hexValFound = 0;
        fileSearchEnd = 1;
    }

    filePointer = KeyFile->tellg();                     //파일 포인터 업데이트
}

void KeyHandler::generateFile(std::ofstream* FlashFile) {                     //open file on class KeyFile
    char zero = 0x0;
    FlashFile->open("FlashKey.bin", std::ios::out | std::ios::binary);

    for (int i = 0; i < 0x40000; i++) {
        FlashFile->write(&zero, sizeof(zero));
    }
}