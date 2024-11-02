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


STEPPER::STEPPER()
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

void STEPPER::initialize() {
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
        throw runtime_error ("CreateFile failed with error %d.\n");
        return;
    }
    SecureZeroMemory(&dcb, sizeof(DCB));
    dcb.DCBlength = sizeof(DCB);

    fSuccess = GetCommState(hSerial, &dcb);
    if (!fSuccess)
    {
        throw runtime_error("GetCommState error");
        return;
    }

    dcb.BaudRate = CBR_115200;     //  baud rate
    dcb.ByteSize = 8;             //  data size, xmit and rcv
    dcb.Parity = NOPARITY;      //  parity bit
    dcb.StopBits = ONESTOPBIT;    //  stop bit

    fSuccess = SetCommState(hSerial, &dcb);

    if (!fSuccess)
    {
        throw runtime_error("SetCommState error\n");
        return;
    }
    memset(inputData, 0, sizeof(inputData));
    SetupComm(hSerial, 1024, 1024);
    char outputData[1024] = {};
    COMSTAT comStat;
    ClearCommError(hSerial, &dwError, &comStat);
}

void STEPPER::read(){
    bool check = true;
    do {
        if (!ReadFile(hSerial, inputData, dwBuffer, &dwRead, &overRead) && GetLastError() == ERROR_IO_PENDING)
        {
            WaitForSingleObject(overRead.hEvent, 2000);
        }
        else check = false;
    } while (check);
    return;
}

void STEPPER::writeCmd(LPCVOID outputData) { //todo
    bool check = true;
    do {
        if (!WriteFile(hSerial, outputData, sizeof(outputData), &dwWritten, &overWrite) && (GetLastError() == ERROR_IO_PENDING))
        {
            if (WaitForSingleObject(overWrite.hEvent, 1000))
            {
                dwWritten = 0;
            }
            else
            {
                GetOverlappedResult(hSerial, &overWrite, &dwWritten, FALSE);
                overWrite.Offset += dwWritten;
            }
            //            printf("WriteFile error\n");
        }
    } while (check);
}
