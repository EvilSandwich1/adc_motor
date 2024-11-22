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
std::ofstream file_data;

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
        ComTimeOut.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(hSerial, &ComTimeOut);

        if (hSerial == INVALID_HANDLE_VALUE)
        {
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
    memset(inputData, 0, 1024);
    if (!ReadFile(hSerial, inputData, dwBuffer, &dwRead, &overRead) && GetLastError() == ERROR_IO_PENDING)
    {
        WaitForSingleObject(overRead.hEvent, INFINITE);
        GetOverlappedResult(hSerial, &overRead, &dwRead, FALSE);
    }
    return inputData;
}

void stepper::write_cmd(char* outputData) { 
    if (!WriteFile(hSerial, outputData, sizeof(outputData), &dwWritten, &overWrite) && (GetLastError() == ERROR_IO_PENDING))
    {
        WaitForSingleObject(overWrite.hEvent, INFINITE);
        GetOverlappedResult(hSerial, &overWrite, &dwWritten, FALSE);
    }
}

StpCoord stepper::get_current_coord() {
    char cmd[4];
    strcpy_s(cmd, "?\n");
    write_cmd(cmd);
    char output[1024] = { 0 };
    strcpy_s(output, read());
    std::string output_s = output;
    std::string tmp;
    if (output_s.empty() || !output_s.contains("WPos") || output_s[0] != '<')
        return current_coord;
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

bool stepper::home() {
    char cmd[1024];
    strcpy_s(cmd, "$H\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    read();

    strcpy_s(cmd, "$H Z\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    read();

    strcpy_s(cmd, "G91\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    read();

    return true;
}

bool stepper::move(float val, std::string coord) {
    char cmd[1024];

    if (coord == "x") {
        if (current_coord.x + val >= 0.0f) {
            return false;
        }
        sprintf_s(cmd, "G0 X%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    if (coord == "y") {
        if (current_coord.y + val >= 0.0f) {
            return false;
        }
        sprintf_s(cmd, "G0 Y%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    if (coord == "z") {
        if (current_coord.z + val >= 1.0f) {
            return false;
        }
        sprintf_s(cmd, "G0 Z%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    else {
        return false;
    }
}

void stepper::close() {
    if (overRead.hEvent != NULL)
        CloseHandle(overRead.hEvent);
    if (overWrite.hEvent != NULL)
        CloseHandle(overWrite.hEvent);
    if (hSerial != 0) {
        if (!CloseHandle(hSerial))
            throw runtime_error("CloseHandle error\n");
    }
    return;
}

bool stepper::algorithm(AlgCoord coord, ADC adc) {

    char cmd[32];
    char data_s[1024];
    float data;
    strcpy_s(cmd, "G90\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    read();

    file_data.open("test_data.txt");
    if (!file_data)
        return false;

    int i = 0;
    int j = 0;
    int k = 0;

    float val_x = 0;
    float val_y = 0;
    float val_z = 0;
    if (coord.step_x == 0)
        coord.step_x = 1;
    if (coord.step_y == 0)
        coord.step_y = 1;
    if (coord.step_z == 0)
        coord.step_z = 1;

    do {
        val_x = coord.begin_x + coord.step_x * i;
        move(val_x, "x");
        while (true) {
            if (current_coord.x == val_x)
                break;
        }
        i++;
        Sleep(100);
        j = 0;
        do {
            val_y = coord.begin_y + coord.step_y * j;
            move(val_y, "y");
            while (true) {
                if (current_coord.y == val_y)
                    break;
            }
            Sleep(100);
            j++;
            k = 0;
            while (k <= ((coord.end_z - coord.begin_z) / coord.step_z)) {
                val_z = coord.begin_z + coord.step_z * k;
                move(val_z, "z");
                while (true) {
                    if (current_coord.z == val_z)
                        break;
                }
                data = adc.data_avg(adc.data_proc());
                sprintf_s(data_s, "X: %f, Y: %f, Z: %f, Data: %f\n", current_coord.x, current_coord.y, current_coord.z, data);
                file_data << data_s;
                k++;
                Sleep(100);
            } 
        } while (j <= ((coord.end_y - coord.begin_y) / coord.step_y));
    } while (i <= ((coord.end_x - coord.begin_x) / coord.step_x));
    
    file_data.close();
    return true;
}

void stepper::test(AlgCoord coord) {
    char cmd[32];
    strcpy_s(cmd, "G90\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    read();

    int i = 0;
    int j = 0;
    int k = 0;

    float val_x = 0;
    float val_y = 0;
    float val_z = 0;
    if (coord.step_x == 0)
        coord.step_x = 1;
    if (coord.step_y == 0)
        coord.step_y = 1;
    if (coord.step_z == 0)
        coord.step_z = 1;
}
stepper::~stepper() {
    close();
}
