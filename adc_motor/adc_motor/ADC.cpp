#include "ADC.h"

using namespace std;

void* fdata = NULL, * fdata1 = NULL;
HANDLE hFile, hMap, hFile1, hMap1 = NULL;


void* datas;
ULONG* sync;

LONG complete;

HANDLE hThread = NULL;
ULONG Tid;

//ADC* pADC;

ULONG IrqStep = 1024; //777-777%7; // половинка буфера кратная числу каналов или не обязательно кратная
ULONG FIFO = 1024;    // fifo
ULONG pages = 128;    // количество страниц IrqStep в кольцевом буфере PC
ULONG multi = 2;      // - количество половинок кольцевого буфера, которое мы хотим собрать и записать на диск
ULONG pointsize;      // pI->GetParameter(L_POINT_SIZE, &ps) возвращает размер отсчета в байтах (обычно 2, но 791 4 байта)

long fsize = multi * (pages / 2) * IrqStep;

ASYNC_PAR pp;
ULONG step = 0;
float datafloat;

DWORD tm = 10000000;


typedef IDaqLDevice* (*CREATEFUNCPTR)(ULONG Slot);

const int BUF_LENGTH = 1;

CREATEFUNCPTR CreateInstance;

ADC_PAR adcPar;
DAC_PAR dacPar;
PLATA_DESCR_U2 pd;
SLOT_PAR sl;
ULONG slot;
USHORT d;
ULONG param;
ULONG status;
int k;
IDaqLDevice* pI;
int y;
float dataBuffer[8192];
bool ready_swap = false;

int buffer_pointer;

vector<float> buffer;
vector<float> buffer_long;
vector<float> pffft_buffer;
//vector<float> pffft_buffer_ord;

ULONG WINAPI ServiceThread(PVOID /*Context*/)
{
    ULONG halfbuffer = IrqStep * pages / 2;              // Собираем половинками кольцевого буфера
    ULONG s{};
    InterlockedExchange(&s, *sync);
    ULONG fl2, fl1 = fl2 = (s <= halfbuffer) ? 0 : 1;  // Настроили флаги
    void* tmp, * tmp1;

    for (int i = 0; i < multi; i++)                         // Цикл по необходимомму количеству половинок
    {
        while (fl2 == fl1)
        {
            if (InterlockedCompareExchange(&complete, 3, 3)) return 0;
            InterlockedExchange(&s, *sync);
            fl2 = (s <= halfbuffer) ? 0 : 1; // Ждем заполнения половинки буфера
        }

        tmp1 = ((char*)datas + (halfbuffer * i) * pointsize);                   // Настраиваем указатель в кольцевом буфере
        InterlockedExchange(&s, *sync);
        fl1 = (s <= halfbuffer) ? 0 : 1;                 // Обновляем флаг
        Sleep(0);                                     // если собираем медленно то можно и спать больше
    }
    return 0;                                         // Вышли
}


ADC::ADC()
{
    try 
    {
        //create_controls();
        //this->init_e440();
        //this->stream_setup();
        //this->frame();
    }
    catch (const std::exception& e)
    {
        string excpt_data = e.what();
        MessageBox(nullptr, wstring(begin(excpt_data), end(excpt_data)).c_str(), L"Ошибка", MB_ICONERROR | MB_OK);
        ExitProcess(EXIT_FAILURE);
    }
}


HINSTANCE ADC::CallCreateInstance(LPCWSTR name)
{
    HINSTANCE hComponent = LoadLibrary(name);
    if (hComponent == NULL) { throw runtime_error("LoadLib failed"); }

    CreateInstance = (CREATEFUNCPTR)::GetProcAddress(hComponent, "CreateInstance");
    if (CreateInstance == NULL) { throw runtime_error("CreateInstance failed"); }
    return hComponent;
}

string char_to_str(const char* test)
{
    string str = "";
    int size_arr = 64;
    for (int x = 0; x < size_arr; x++) 
    {
        str += test[x];
    }
    return str;
}

bool ADC::init_e440()
{
#ifdef WIN64
    CallCreateInstance(L"lcomp64.dll");
#else
    CallCreateInstance(L"lcomp.dll");
#endif

#define M_FAIL(x,s) do { throw runtime_error(x);  } while(0)
#define M_OK(x,e)   do {  } while(0)

    LUnknown* pIUnknown = CreateInstance(0);
    if (pIUnknown == NULL) { M_FAIL(char_to_str("CreateInstance"), nullptr); return false; }

    HRESULT hr = pIUnknown->QueryInterface(IID_ILDEV, (void**)&pI);
    if (!SUCCEEDED(hr)) { M_FAIL(char_to_str("QueryInterface"), nullptr); return false; }

    status = pIUnknown->Release();
    M_OK("Release IUnknown", endl);

    HANDLE hVxd = pI->OpenLDevice(); // открываем устройство
    if (hVxd == INVALID_HANDLE_VALUE) { M_FAIL(char_to_str("OpenLDevice"), hVxd); this->disconnect(); return false; }
    else M_OK("OpenLDevice", endl);

    status = pI->GetSlotParam(&sl);
    if (status != L_SUCCESS) { M_FAIL(char_to_str("GetSlotParam"), status); return false; }
    else M_OK("GetSlotParam", endl);

    status = pI->LoadBios("e440"); // загружаем биос в модуль
    if ((status != L_SUCCESS) && (status != L_NOTSUPPORTED)) { M_FAIL(char_to_str("LoadBios"), status); return false; }
    else M_OK("LoadBios", endl);

    status = pI->PlataTest(); // тестируем успешность загрузки и работоспособность биос
    if (status != L_SUCCESS) { M_FAIL(char_to_str("PlataTest"), status); return false; }
    else M_OK("PlataTest", endl);

    status = pI->ReadPlataDescr(&pd); // считываем данные о конфигурации платы/модуля. 
    // ОБЯЗАТЕЛЬНО ДЕЛАТЬ! (иначе расчеты параметров сбора данных невозможны тк нужна информация о названии модуля и частоте кварца )
    if (status != L_SUCCESS) { M_FAIL(char_to_str("ReadPlataDescr"), status); return false; }
    else M_OK("ReadPlataDescr", endl);
    pp.s_Type = L_ASYNC_ADC_INP;
    pp.Chn[0] = 0b00100000; // 0 канал дифф. подключение (в общем случае лог. номер канала)
    status = pI->EnableCorrection();
    pp.dRate = 400.0;
    //display();
    return true;
}
/*void ADC::display() {
    wstringstream wss;
    wss << frame();
    //SendMessage(this->m_hWnd, WM_PAINT, NULL, NULL);
}*/
float* ADC::frame(){
//float ADC::frame() {
    /*datafloat = pp.Data[0];
    datafloat /= 800.0;*/
    for (int i = 0; i < 8192; i++) {
        status = pI->IoAsync(&pp);
        dataBuffer[i] = pp.Data[0] / 800.0f;
    }
    return dataBuffer;
//  return datafloat;
}

bool ADC::stream_setup() {
    status = pI->RequestBufferStream(&tm, L_STREAM_ADC);
    if (status != L_SUCCESS) { M_FAIL("RequestBufferStream(ADC)", status); return false; }
    else M_OK("RequestBufferStream(ADC)", endl);

    adcPar.t1.s_Type = L_ADC_PARAM;
    adcPar.t1.AutoInit = 1;
    adcPar.t1.dRate = 1.024;
    adcPar.t1.dKadr = 0;
    adcPar.t1.dScale = 0;
    adcPar.t1.SynchroType = 3;//0
    adcPar.t1.SynchroSensitivity = 1;
    adcPar.t1.SynchroMode = 0;
    adcPar.t1.AdChannel = 0b00100000;
    adcPar.t1.AdPorog = 0;
    adcPar.t1.NCh = 1;
    adcPar.t1.Chn[0] = 0b00100000;
    //adcPar.t1.Chn[1] = 0x1;
    //adcPar.t1.Chn[2] = 0x2;
    //adcPar.t1.Chn[3] = 0x3;
    adcPar.t1.FIFO = 512;
    adcPar.t1.IrqStep = 512;
    adcPar.t1.Pages = 2;
    adcPar.t1.IrqEna = 1;
    adcPar.t1.AdcEna = 1;

    status = pI->FillDAQparameters(&adcPar.t1);
    if (status != L_SUCCESS) { M_FAIL("FillDAQparameters(ADC)", status); return false; }
    else M_OK("FillDAQparameters(ADC)", endl);

    status = pI->SetParametersStream(&adcPar.t1, &tm, (void**)&datas, (void**)&sync, L_STREAM_ADC);
    if (status != L_SUCCESS) { M_FAIL("SetParametersStream(ADC)", status); return false; }
    else M_OK("SetParametersStream(ADC)", endl);

    IrqStep = adcPar.t1.IrqStep; // обновили глобальные переменные котрые потом используются в ServiceThread
    pages = adcPar.t1.Pages;

    pI->GetParameter(L_POINT_SIZE, &pointsize);
    if (status != L_SUCCESS) { M_FAIL("GetParameter", status); return false; }
    else M_OK("GetParameter", endl);

    status = pI->InitStartLDevice(); // Инициализируем внутренние переменные драйвера
    if (status != L_SUCCESS) { M_FAIL("InitStartLDevice(ADC)", status); return false; }
    else M_OK("InitStartLDevice(ADC)", endl);

    hThread = CreateThread(0, 0x2000, ServiceThread, 0, 0, &Tid); // Создаем и запускаем поток сбора данных

    status = pI->EnableCorrection();

    status = pI->StartLDevice(); // Запускаем сбор в драйвере
    if (status != L_SUCCESS) { M_FAIL("StartLDevice(ADC)", status); return false; }
    else M_OK("StartLDevice(ADC)", endl);

    return true;
}

void* ADC::stream() {
    while (true)
    {
        ULONG s;
        InterlockedExchange(&s, *sync);
        //cout << ".......... " << setw(8) << s << "\r";
        if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, 0)) { complete = 1; break; }
        Sleep(10);
    }

    if (!complete)
    {
        InterlockedBitTestAndSet(&complete, 0); //complete=1
        WaitForSingleObject(hThread, INFINITE);
    }
    return datas;
}

void ADC::pffft(int npoints, int window_id) {
    data_proc();
    static float scaleFactor;
    static float maxVolt = 2.5f;
    pffft_buffer.clear();
    vector<float> pffft_input_buffer;
    vector<float> window;
    if (buffer_long.size() < npoints)
        return;
    float max = *max_element(&buffer_long[0], &buffer_long[npoints - 1]);
    scaleFactor = maxVolt / max;
    switch (window_id) {
        case 1:
            for (int i = 0; i < npoints; i++) {
                window.push_back(0.54 - 0.46 * cos(2 * M_PI * i / (npoints - 1)));
            }
            break;
        case 2:
            for (int i = 0; i < npoints; i++) {
                window.push_back(1
                    - 1.93 * cos(2 * M_PI * i / (npoints - 1))
                    + 1.29 * cos(4 * M_PI * i / (npoints - 1))
                    - 0.388 * cos(6 * M_PI * i / (npoints - 1))
                    + 0.032 * cos(8 * M_PI * i / (npoints - 1)));
            }
            break;
        default:
            for (int i = 0; i < npoints; i++) {
                window.push_back(1);
            }
            break;
    }
    for (int i = 0; i < npoints; i++) {
        pffft_buffer.push_back(0);
        pffft_input_buffer.push_back(buffer_long[i]*scaleFactor*window[i]);
    }
    PFFFT_Setup* pffft_setup = pffft_new_setup(npoints, PFFFT_REAL);
    pffft_transform_ordered(pffft_setup, &pffft_input_buffer[0], &pffft_buffer[0], NULL, PFFFT_FORWARD);
    pffft_destroy_setup(pffft_setup);
}

 void ADC::disconnect() {
     status = pI->StopLDevice(); // Остановили сбор
     if (status != L_SUCCESS) { M_FAIL("StopLDevice(ADC)", status); return; }
     else M_OK("StopLDevice(ADC)", endl);

     status = pI->CloseLDevice();
     if (status != L_SUCCESS) { M_FAIL(char_to_str("CloseLDevice"), status); /*goto end;*/ }
     else M_OK("CloseLDevice", endl);

     status = pI->Release();
     M_OK("Release IDaqLDevice", endl);
 }

vector<float>& ADC::data_proc() {
     //stream();
     buffer.clear();
     while (true)
     {
         ULONG s;
         InterlockedExchange(&s, *sync);
         cout << ".......... " << setw(8) << s << "\r";
         if (WAIT_OBJECT_0 == WaitForSingleObject(hThread, 0)) { complete = 1; break; }
         Sleep(10);
     }

     if (!complete)
     {
         InterlockedBitTestAndSet(&complete, 0); //complete=1
         WaitForSingleObject(hThread, INFINITE);
     }
     ready_swap = false;
     buffer.clear();
     fsize = multi * (pages / 2) * IrqStep;
     if (*sync == IrqStep) {
         for (int i = *sync; i < *sync * 2; i++) {
             buffer.push_back(((PSHORT)datas)[i] / 800.0);
         }
         for (int i = 0; i < *sync ; i++) {
             buffer.push_back(((PSHORT)datas)[i] / 800.0);
         }
     }
     if (*sync == IrqStep * 2) {
         for (int i = 0; i < *sync; i++) {
             buffer.push_back(((PSHORT)datas)[i] / 800.0);
         }
     }
     /*for (int i = 0; i < *sync; i++) {
         buffer.push_back(((PSHORT)datas)[i] / 800.0);
     }*/
     if (buffer_pointer == BUF_LENGTH) {
         buffer_pointer = 0;
         buffer_long.clear();
         ready_swap = true;
     }
     buffer_long.insert(buffer_long.end(), buffer.begin(), buffer.end());
     buffer_pointer++;

     return buffer;
 }

 float ADC::data_avg(vector<float>& dataStream) {
     float avg = std::reduce(dataStream.begin(), dataStream.end()) / dataStream.size();
     return avg;
 }

ADC::~ADC() {}