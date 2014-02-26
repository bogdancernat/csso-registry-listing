// csso-tema1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <string>
#include <malloc.h>
#include <iostream>
#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

#define TOTALBYTES    8192
#define BYTEINCREMENT 4096

LPWSTR getValueType(DWORD type){
  LPWSTR type_name = L"";
  switch (type){
  case REG_BINARY:
    type_name = L"REG_BINARY";
    break;
  case REG_DWORD:
    type_name = L"REG_DWORD";
    break;
  case REG_DWORD_BIG_ENDIAN:
    type_name = L"REG_DWORD_BIG_ENDIAN";
    break;
  case REG_EXPAND_SZ:
    type_name = L"REG_EXPAND_SZ";
    break;
  case REG_LINK:
    type_name = L"REG_LINK";
    break;
  case REG_MULTI_SZ:
    type_name = L"REG_MULTI_SZ";
    break;
  case REG_NONE:
    type_name = L"REG_NONE";
    break;
  case REG_QWORD:
    type_name = L"REG_QWORD";
    break;
  case REG_SZ:
    type_name = L"REG_SZ";
    break;
  }
  return type_name;
}
void loopSubKeys(HKEY key, LPCWSTR current_path){
  // current_path already exists
  DWORD nrSubKeys;
  DWORD longestSubKey;
  DWORD nrValues;
  DWORD longestValue;

  TCHAR valueName[MAX_VALUE_NAME];
  DWORD valueNameSize;

  DWORD v_res;
  DWORD v_type;
  DWORD v_cbData;
  DWORD BufferSize = TOTALBYTES;
  HANDLE hv_file;

  BOOL write_flag = FALSE;
  PPERF_DATA_BLOCK v_data = (PPERF_DATA_BLOCK)malloc(BufferSize);

  DWORD subKeySize;
  DWORD sk_res;
  TCHAR skName[MAX_VALUE_NAME];

  RegQueryInfoKey(key, NULL, NULL, NULL,
    &nrSubKeys,
    &longestSubKey, NULL,
    &nrValues,
    &longestValue, NULL, NULL, NULL);
  // create files for each value this key has
  if (nrValues){
    for (DWORD i = 0, retCode = ERROR_SUCCESS; i < nrValues; i++){
      valueNameSize = MAX_VALUE_NAME;
      valueName[0] = '\0';
      retCode = RegEnumValue(key, i,
        valueName,
        &valueNameSize,
        NULL, NULL, NULL, NULL);

      if (retCode == ERROR_SUCCESS){
        std::wstring filePath = current_path + std::wstring(valueName);

        hv_file = CreateFile(filePath.c_str(),
          FILE_APPEND_DATA,
          FILE_SHARE_READ,
          NULL,
          CREATE_ALWAYS,
          FILE_ATTRIBUTE_NORMAL,
          NULL);

        // get value data
        v_res = RegQueryValueEx(key, valueName, NULL, &v_type, (LPBYTE)v_data, &v_cbData);
        while (v_res == ERROR_MORE_DATA){
          // Get a buffer that is big enough.

          BufferSize += BYTEINCREMENT;
          v_data = (PPERF_DATA_BLOCK)realloc(v_data, BufferSize);
          v_cbData = BufferSize;
          v_res = RegQueryValueEx(key,
            valueName,
            NULL,
            NULL,
            (LPBYTE)v_data,
            &v_cbData);
        }
        if (v_res == ERROR_SUCCESS){
          // write to file
          LPWSTR type_name = getValueType(v_type);
          std::string a = "abcd";

          write_flag = WriteFile(
            hv_file,
            type_name,
            std::wstring(type_name).size()*2,
            NULL,
            NULL
            );
          write_flag = WriteFile(
            hv_file,
            "\n",
            2,
            NULL,
            NULL
            );
          write_flag = WriteFile(
            hv_file,
            v_data,
            v_cbData,
            NULL,
            NULL
            );
          CloseHandle(hv_file);
        }
      }
    }
  } // end create files for each value this key has
  // create folders for each subkey this key has and call this function again

  if (nrSubKeys){
    for (DWORD i = 0; i < nrSubKeys; i++){
      subKeySize = MAX_KEY_LENGTH;
      sk_res = RegEnumKeyEx(key, i,
        skName,
        &subKeySize,
        NULL,
        NULL,
        NULL,
        NULL);
      if (sk_res == ERROR_SUCCESS){
        std::wstring path_subKey = std::wstring(current_path) + L"\\" + std::wstring(skName);

        std::wcout << path_subKey << '\n';
        if (CreateDirectory(path_subKey.c_str(), NULL)
          || ERROR_ALREADY_EXISTS == GetLastError()){
          HKEY subKey;
          RegOpenKeyEx(key, skName, 0, KEY_READ, &subKey);
          loopSubKeys(subKey, path_subKey.c_str());
        }
        else {
          printf("Oups! That path is invalid");
        }
      }
    }
  }
}

int _tmain(int argc, _TCHAR* argv[]){
  HKEY rootKey;
  LONG res;
  LPCWSTR startkey = L"SOFTWARE";
  LPCWSTR DIR_ROOT = L"c:\\csso-output\\";

  std::wstring path = DIR_ROOT + std::wstring(startkey);

  std::wcout << path << '\n';
  res = RegOpenKeyEx(HKEY_CURRENT_USER, startkey, 0, KEY_READ, &rootKey);

  if (res == ERROR_SUCCESS){
    if (CreateDirectory(path.c_str(), NULL)
      || ERROR_ALREADY_EXISTS == GetLastError()){
      loopSubKeys(rootKey, path.c_str());
    }
    else {
      printf("Oups! That path is invalid");
    }
  }
  RegCloseKey(rootKey);
  return 0;
}


