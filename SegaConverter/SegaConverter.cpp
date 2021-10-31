#include <iostream>
#include <fstream>
#include "KeyHandler.h"


int main() {
    std::ifstream MameKey;                                  //SegaKey 클래스 생성 
    std::ofstream FlashKey; // ("FlashKey.bin", std::ios::out | std::ios::binary);
    KeyHandler SegaKey;                                     //ReadKey 클래스 생성

    SegaKey.openKey(&MameKey);
    SegaKey.searchKey(&MameKey);
    SegaKey.saveKey(&FlashKey, 0, 12, 11, 10, 9, 8 , 7, 6, 5, 4, 3, 2, 1, 0);
    SegaKey.saveKey(&FlashKey, 1, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
    SegaKey.closeKey(&MameKey);

    return 0;
}



