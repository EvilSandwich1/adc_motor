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
//OVERLAPPED overRead, overWrite;
std::ofstream file_data;
//std::ofstream log_file;

using namespace std;

StpCoord current_coord;

const wchar_t* FindComPortByVidPid() {
    static wchar_t result[16] = { 0 }; // Буфер для результата (L"COMX")
    const wchar_t* usbPath = L"SYSTEM\\CurrentControlSet\\Enum\\USB";

    // Формируем маску VID&PID
    wchar_t vidPid[32] = L"VID_1A86&PID_7523";

    HKEY hUsbKey;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, usbPath, 0, KEY_READ, &hUsbKey) != ERROR_SUCCESS) {
        return nullptr;
    }

    wchar_t deviceKeyName[256];
    DWORD index = 0;
    DWORD nameSize = 256;

    // Ищем устройство с нужным VID&PID
    while (RegEnumKeyEx(hUsbKey, index++, deviceKeyName, &nameSize,
        NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
        if (wcsstr(deviceKeyName, vidPid) != nullptr) {
            // Нашли устройство, ищем его параметры
            std::wstring devicePath = std::wstring(usbPath) + L"\\" + deviceKeyName;

            HKEY hDeviceKey;
            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, devicePath.c_str(), 0, KEY_READ, &hDeviceKey) == ERROR_SUCCESS) {
                wchar_t subKeyName[256];
                DWORD subIndex = 0;
                DWORD subNameSize = 256;

                // Проверяем подразделы устройства
                while (RegEnumKeyEx(hDeviceKey, subIndex++, subKeyName, &subNameSize,
                    NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    std::wstring paramsPath = devicePath + L"\\" + subKeyName + L"\\Device Parameters";

                    HKEY hParamsKey;
                    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, paramsPath.c_str(), 0, KEY_READ, &hParamsKey) == ERROR_SUCCESS) {
                        wchar_t portName[256];
                        DWORD portSize = sizeof(portName);

                        // Получаем имя порта
                        if (RegQueryValueEx(hParamsKey, L"PortName", NULL, NULL,
                            (LPBYTE)portName, &portSize) == ERROR_SUCCESS) {
                            // Проверяем, что это COM-порт
                            if (wcsncmp(portName, L"COM", 3) == 0) {
                                swprintf_s(result, L"COM%ls", portName + 3); // Форматируем результат
                                RegCloseKey(hParamsKey);
                                RegCloseKey(hDeviceKey);
                                RegCloseKey(hUsbKey);
                                return result;
                            }
                        }
                        RegCloseKey(hParamsKey);
                    }
                    subNameSize = 256;
                }
                RegCloseKey(hDeviceKey);
            }
        }
        nameSize = 256;
    }

    RegCloseKey(hUsbKey);
    return nullptr;
}

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
        sPortName = FindComPortByVidPid();
        hSerial = CreateFile(sPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        //memset(&overRead, 0, sizeof(OVERLAPPED));
        //memset(&overWrite, 0, sizeof(OVERLAPPED));

        //overRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        //overWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

        COMMTIMEOUTS ComTimeOut;
        ComTimeOut.ReadIntervalTimeout = MAXDWORD;
        ComTimeOut.ReadTotalTimeoutConstant = 0;
        ComTimeOut.ReadTotalTimeoutMultiplier = 0;
        ComTimeOut.WriteTotalTimeoutMultiplier = 0;
        ComTimeOut.WriteTotalTimeoutConstant = 0;
        SetCommTimeouts(hSerial, &ComTimeOut);

        if (hSerial == INVALID_HANDLE_VALUE){
            throw runtime_error("CreateFile failed with error %d.\n");
            return false;
        }
        SecureZeroMemory(&dcb, sizeof(DCB));
        dcb.DCBlength = sizeof(DCB);

        fSuccess = GetCommState(hSerial, &dcb);
        if (!fSuccess){
            throw runtime_error("GetCommState error");
            return false;
        }

        dcb.BaudRate = CBR_115200;     //  baud rate
        dcb.ByteSize = 8;             //  data size, xmit and rcv
        dcb.Parity = NOPARITY;      //  parity bit
        dcb.StopBits = ONESTOPBIT;    //  stop bit

        fSuccess = SetCommState(hSerial, &dcb);

        if (!fSuccess){
            throw runtime_error("SetCommState error\n");
            return false;
        }
        memset(inputData, 0, sizeof(inputData));
        SetupComm(hSerial, 128, 1024);
        COMSTAT comStat;
        ClearCommError(hSerial, &dwError, &comStat);

        write_cmd(const_cast<char*>("\r\n\r\n"));
        return true;
    }
    catch (const std::exception& e){
        string excpt_data = e.what();
        MessageBox(nullptr, wstring(begin(excpt_data), end(excpt_data)).c_str(), L"Error", MB_ICONERROR | MB_OK);
    }
}

char* stepper::read(){
    memset(inputData, 0, 1024);
    if (!ReadFile(hSerial, inputData, dwBuffer, &dwRead, NULL))
    {
        //WaitForSingleObject(overRead.hEvent, INFINITE);
        //GetOverlappedResult(hSerial, &overRead, &dwRead, FALSE);
    }
    return inputData;
}

void stepper::write_cmd(char* outputData) { 
    if (!WriteFile(hSerial, outputData, sizeof(outputData), &dwWritten, NULL))
    {
        //WaitForSingleObject(overWrite.hEvent, INFINITE);
        //etOverlappedResult(hSerial, &overWrite, &dwWritten, FALSE);
    }
}

void stepper::just(float speed) {
    char cmd[32];
    float cur_x = current_coord.x;

    if (cur_x > 127.0 || cur_x < 5) return;
    sprintf_s(cmd, "G1 X-5.0 F%f", speed);
    write_cmd(cmd);

    while (true) {
        if (current_coord.x == cur_x - 5.0) {
            sprintf_s(cmd, "G1 X5.0 F%f", speed);
            write_cmd(cmd);
            break;
        }
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
    if (output_s.empty() || !output_s.contains("WPos") || output_s[0] != '<' || !output_s.contains("."))
        return current_coord;

    if (output_s.find("WPos") + 5 > output_s.size())
        return current_coord;

    for (int i = output_s.find("WPos") + 5; i < output_s.find(","); i++) {
        if (i >= output_s.size())
            break;
        tmp += output_s[i];
    }
    if (tmp.contains(",")) {
        tmp.erase(tmp.find(","));
    }
    current_coord.x = stof(tmp);
    tmp.clear();
    output_s.erase(0, output_s.find(",") + 1);

    if (!output_s.contains(","))
        return current_coord;

    for (int i = 0; i < output_s.find(","); i++) { 
        if (i >= output_s.size())
            break;
        tmp += output_s[i];
    }
    if (tmp.contains(",")) {
        tmp.erase(tmp.find(","));
    }
    current_coord.y = stof(tmp);
    tmp.clear();
    output_s.erase(0, output_s.find(",") + 1);

    if (!output_s.contains("."))
        return current_coord;

    if (output_s.contains("|")) {
        for (int i = 0; i < output_s.find("|"); i++) {
            if (i >= output_s.size())
                break;
            tmp += output_s[i];
        }
        if (tmp.contains("|")) {
            tmp.erase(tmp.find("|"));
        }
    }
    else {
        for (int i = 0; i < output_s.size() - 1; i++) {
            if (i >= output_s.size())
                break;
            tmp += output_s[i];
        }
    }
    current_coord.z = stof(tmp);
    tmp.clear();

    return current_coord;
}

bool stepper::home() {
    char cmd[128];
    char tmp[128];
    strcpy_s(cmd, "$H\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    tmp[0] = *read();

    Sleep(500);

    strcpy_s(cmd, "$H Z\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    tmp[0] = *read();

    Sleep(500);

    strcpy_s(cmd, "G91\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);
    tmp[0] = *read();

    return true;
}

bool stepper::move(float val, std::string coord) {
    char cmd[128];

    if (coord == "x") {
        if (current_coord.x + val >= 0.0f) {
            return false;
        }
        sprintf_s(cmd, "G0X%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    if (coord == "y") {
        if (current_coord.y + val >= 0.0f) {
            return false;
        }
        sprintf_s(cmd, "G0Y%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    if (coord == "z") {
        if (current_coord.z + val >= 1.0f) {
            return false;
        }
        sprintf_s(cmd, "G0Z%f\n", val);
        write_cmd(cmd);
        read();
        return true;
    }

    else {
        return false;
    }
}

void stepper::close() {
   /* if (overRead.hEvent != NULL)
        CloseHandle(overRead.hEvent);
    if (overWrite.hEvent != NULL)
        CloseHandle(overWrite.hEvent);*/
    if (hSerial != 0) {
        if (!CloseHandle(hSerial))
            throw runtime_error("CloseHandle error\n");
    }
    return;
}

bool stepper::algorithm(AlgCoord coord, ADC adc, int delay) {

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
                Sleep(delay);
                //data = adc.data_avg(adc.data_proc()); // СИНХРОННАЯ ЕБУЛДЫГА
                data = adc.frame();
                sprintf_s(data_s, "X: %f, Y: %f, Z: %f, Data: %f\n", current_coord.x, current_coord.y, current_coord.z, data);
                file_data << data_s;
                k++;
                Sleep(100);
            } 
        } while (j <= ((coord.end_y - coord.begin_y) / coord.step_y));
    } while (i <= ((coord.end_x - coord.begin_x) / coord.step_x));
    
    strcpy_s(cmd, "G91\n");
    Sleep(100);
    write_cmd(cmd);
    Sleep(100);

    file_data.close();
    return true;
}

bool stepper::algorithm_smart(AlgCoord coord, ADC adc, int delay) {
    char cmd[32];
    char data_s[1024];
    float data;

    strcpy_s(cmd, "G90\n");
    this_thread::sleep_for(100ms);
    write_cmd(cmd);
    this_thread::sleep_for(100ms);
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

    bool y_inv = false;
    bool z_inv = false;

    if (coord.step_x == 0)
        coord.step_x = 1.0f;
    if (coord.step_y == 0)
        coord.step_y = 1.0f;
    if (coord.step_z == 0)
        coord.step_z = 1.0f;

    do {
        val_x = coord.begin_x + coord.step_x * i;
        move(val_x, "x");
        while (true) {
            if (current_coord.x == val_x)
                break;
        }
        i++;
        this_thread::sleep_for(100ms);
        j = 0;
        if (current_coord.y == coord.begin_y) {
            y_inv = false;
        }
        if (current_coord.y == coord.end_y) {
            y_inv = true;
        }
        do {
            if (!y_inv) {
                val_y = coord.begin_y + coord.step_y * j;
            }
            if (y_inv) {
                val_y = coord.end_y - coord.step_y * j;
            }
            move(val_y, "y");
            while (true) {
                if (current_coord.y == val_y)
                    break;
            }
            j++;
            this_thread::sleep_for(100ms);
            k = 0;

            if (current_coord.z == coord.begin_z) {
                z_inv = false;
            }
            if (current_coord.z == coord.end_z) {
                z_inv = true;
            }
            while (k <= ((coord.end_z - coord.begin_z) / coord.step_z)) {
                if (!z_inv) {
                    val_z = coord.begin_z + coord.step_z * k;
                }
                if (z_inv) {
                    val_z = coord.end_z - coord.step_z * k;
                }
                move(val_z, "z");
                while (true) {
                    if (current_coord.z == val_z)
                        break;
                }
                this_thread::sleep_for(std::chrono::milliseconds(delay));
                //data = adc.data_avg(adc.data_proc()); // СИНХРОННАЯ ЕБУЛДЫГА
                data = adc.frame();
                this_thread::sleep_for(100ms);
                sprintf_s(data_s, "X: %f, Y: %f, Z: %f, Data: %f\n", current_coord.x, current_coord.y, current_coord.z, data);
                file_data << data_s;
                k++;
                this_thread::sleep_for(100ms);
            }
        } while (j <= ((coord.end_y - coord.begin_y) / coord.step_y));
    } while (i <= ((coord.end_x - coord.begin_x) / coord.step_x));

    strcpy_s(cmd, "G91\n");
    this_thread::sleep_for(100ms);
    write_cmd(cmd);
    this_thread::sleep_for(100ms);

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
}
