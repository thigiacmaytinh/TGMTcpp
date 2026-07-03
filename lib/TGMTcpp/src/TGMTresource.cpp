#include "TGMTresource.h"
#include <windows.h>
#include "resource.h"
#include <fstream>
#include <iostream>

HMODULE GCM()
{
    HMODULE hModule = NULL;
        
    GetModuleHandleEx(
        GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
        (LPCTSTR)GCM,
        &hModule); // Current module handle
    return hModule;
}

void TGMTresource::WriteResourceToFile(int resourceID, const std::string& outputPath) {   

    // Locate the resource
    HRSRC hRes = FindResource(GCM(), MAKEINTRESOURCE(resourceID), MAKEINTRESOURCE(DATA));
    if (!hRes) {
        std::cerr << "Failed to find resource. Error code: " << GetLastError() << "\n";
        return;
    }

    // Load the resource into memory
    HGLOBAL hResData = LoadResource(GCM(), hRes);
    if (!hResData) {
        std::cerr << "Failed to load resource.\n";
        return;
    }

    // Get the size of the resource
    DWORD dataSize = SizeofResource(GCM(), hRes);


    void* pData = LockResource(hResData);

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(static_cast<const char*>(pData), dataSize);
    outFile.close();

}
