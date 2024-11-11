#include "stepper.h"

HANDLE hSerial;
LPCWSTR sPortName = L"COM4";
DCB dcb;
BOOL fSuccess;
DWORD dwBuffer = 1024;
DWORD dwWritten;
DWORD dwError;
char inputData[1024] = { 0 };
DWORD dwRead;
HANDLE hEvent;
unsigned long int data = 0;
bool check = true;
OVERLAPPED overRead, overWrite;

using namespace std;

StpCoord current_coord;


stepper::stepper()
{
    try {

    }
    catch (const std::exception& e)
    {
        string excpt_data = e.what();
        MessageBox(nullptr, wstring(begin(excpt_data), end(excpt_data)).c_str(), L"Error", MB_ICONERROR | MB_OK);
        ExitProcess(EXIT_FAILURE);
    }
}

bool stepper::initialize() {
    try {
        hSerial = CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL);
        memset(&overRead, 0, sizeof(OVERLAPPED));
        memset(&overWrite, 0, sizeof(OVERLAPPED));

        overRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        overWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        COMMTIMEOUTS ComTimeOut;
        ComTimeOut.ReadIntervalTimeout = MAXDWORD;
        ComTimeOut.ReadTotalTimeoutConstant = 0;
        ComTimeOut.ReadTotalTimeoutMultiplier = 0;
        ComTimeOut.WriteTotalTimeoutMultiplier = 0;
        ComTimeOut.WriteTotalTimeoutConstant = 5000;
        SetCommTimeouts(hSerial, &ComTimeOut);

        if (hSerial == INVALID_HANDLE_VALUE)
        {
            //  Handle the error.
            throw runtime_error("CreateFile failed with error %d.\n");
            return false;
        }
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);

        fSuccess = GetCommState(hSerial, &dcb);
        if (!fSuccess)
        {
            throw runtime_error("GetCommState error");
            return false;
        }

        dcb.BaudRate = CBR_115200;     //  baud rate
        dcb.ByteSize = 8;             //  data size, xmit and rcv
        dcb.Parity = NOPARITY;      //  parity bit
        dcb.StopBits = ONESTOPBIT;    //  stop bit

        fSuccess = SetCommState(hSerial, &dcb);

        if (!fSuccess)
        {
            throw runtime_error("SetCommState error\n");
            return false;
        }
        memset(inputData, 0, sizeof(inputData));
        SetupComm(hSerial, 1024, 1024);
        COMSTAT comStat;
        ClearCommError(hSerial, &dwError, &comStat);
        return true;
    }
    catch (const std::exception& e)
    {
        string excpt_data = e.what();
        MessageBox(nullptr, wstring(begin(excpt_data), end(excpt_data)).c_str(), L"Error", MB_ICONERROR | MB_OK);
    }
}

char* stepper::read(){
    bool check = true;
    memset(inputData, 0, 1024);
    do {
        if (!ReadFile(hSerial, inputData, dwBuffer, &dwRead, &overRead) && GetLastError() == ERROR_IO_PENDING)
        {
            WaitForSingleObject(overRead.hEvent, 2000);
        }
        else check = false;
    } while (check);
    return inputData;
}

void stepper::write_cmd(char* outputData) { 
    bool check = true;
    do {
    check = false;
        if (!WriteFile(hSerial, outputData, sizeof(outputData), &dwWritten, &overWrite) && (GetLastError() == ERROR_IO_PENDING))
        {
            if (WaitForSingleObject(overWrite.hEvent, 1000))
            {
                dwWritten = 0;
                check = true;
            }
            else
            {
                GetOverlappedResult(hSerial, &overWrite, &dwWritten, FALSE);
                overWrite.Offset += dwWritten;
            }
        }
    } while (check);
}

StpCoord stepper::get_current_coord() {
    char cmd[4];
    strcpy_s(cmd, "?\n");
    write_cmd(cmd);
    char output[1024] = { 0 };
    strcpy_s(output, read());
    std::string output_s = output;
    std::string tmp;
    for (int i = output_s.find("WPos") + 5; i < output_s.find("WPos") + 10; i++) {
        tmp += output_s[i];
    }
    current_coord.x = stof(tmp);
    tmp.clear();
    output_s.erase(0, output_s.find("WPos") + 5);

    for (int i = output_s.find(",") + 1; i < output_s.find(",") + 5; i++) { 
        tmp += output_s[i];
    }
    current_coord.y = stof(tmp);
    tmp.clear();
    output_s.erase(0, output_s.find(",") + 5);

    for (int i = output_s.find(",") + 1; i < output_s.find(",") + 5; i++) { 
        tmp += output_s[i];
    }
    current_coord.z = stof(tmp);
    tmp.clear();
    output_s.erase(0, output_s.find(",") + 5);

    return current_coord;
}

void stepper::close() {
    if (overRead.hEvent != NULL)
        CloseHandle(overRead.hEvent);
    if (overWrite.hEvent != NULL)
        CloseHandle(overWrite.hEvent);
    if (!CloseHandle(hSerial))
        throw runtime_error("CloseHandle error\n");
    return;
}

stepper::~stepper() {
}
