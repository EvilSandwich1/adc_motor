#ifdef __cplusplus
   #define DllExport(type) extern "C" /*__declspec( dllexport )*/ type
#else 
   #define DllExport(type) type
#endif

#ifdef WIN64
#define L_ULONG ULONG64
#define PL_ULONG PULONG64
#else
#define L_ULONG ULONG
#define PL_ULONG PULONG
#endif

// ^�᪮�����஢��� �� �������樨 DLL

#define CASE_DAC_PAR_0 0
#define CASE_DAC_PAR_1 1
#define CASE_ADC_PAR_0 2
#define CASE_ADC_PAR_1 3

DllExport(L_ULONG) LoadAPIDLL(char *dllname);
// ����㦠�� lcomp.dll
DllExport(ULONG) FreeAPIDLL(PL_ULONG hDll);
// ���㦠�� lcomp.dll
DllExport(LPVOID) CallCreateInstance(PL_ULONG hDll, ULONG slot, PULONG Err); // return handle for new iface
// ����砥� 㪠��⥫� �� ����䥩� ��� ��᫥���饣� �맮�� �㭪権

// �� �㭪樨 �������� �訡�� ��� � � �ਣ����쭮� ������⥪�
// ���� ��ࠬ��� �� 㪠��⥫� �� ����䥩�
// �� ��⠫�� ��� � �ਣ����쭮� ������⥪� �� �᪫�祭��� ��ࠬ��஢ �������
// ��ࠬ����-�������� ������஢��� �� ������ �����... ���।�⢮� Union
// ᮮ⢥�ᢥ��� ���� � union �롨ࠥ��� �������⥫�� ��ࠬ��஬ �᫮� 0 1 2 3
// �� ����ᠭ�� �ਬ�஢ ��� �ࠢ��쭮� �������樨 ��। �ᥬ� include ���� ��।�����
// #define LABVIEW_FW

DllExport(ULONG)  inbyte ( PL_ULONG hIfc, ULONG offset, PUCHAR data, ULONG len, ULONG key);
DllExport(ULONG)  inword ( PL_ULONG hIfc, ULONG offset, PUSHORT data, ULONG len, ULONG key);
DllExport(ULONG)  indword( PL_ULONG hIfc, ULONG offset, PULONG data, ULONG len, ULONG key);

DllExport(ULONG)  outbyte ( PL_ULONG hIfc, ULONG offset, PUCHAR data, ULONG len, ULONG key);
DllExport(ULONG)  outword ( PL_ULONG hIfc, ULONG offset, PUSHORT data, ULONG len, ULONG key);
DllExport(ULONG)  outdword( PL_ULONG hIfc, ULONG offset, PULONG data, ULONG len, ULONG key);

   // Working with MEM ports
DllExport(ULONG)  inmbyte ( PL_ULONG hIfc, ULONG offset, PUCHAR data, ULONG len, ULONG key);
DllExport(ULONG)  inmword ( PL_ULONG hIfc, ULONG offset, PUSHORT data, ULONG len, ULONG key);
DllExport(ULONG)  inmdword( PL_ULONG hIfc, ULONG offset, PULONG data, ULONG len, ULONG key);

DllExport(ULONG)  outmbyte ( PL_ULONG hIfc, ULONG offset, PUCHAR data, ULONG len, ULONG key);
DllExport(ULONG)  outmword ( PL_ULONG hIfc, ULONG offset, PUSHORT data, ULONG len, ULONG key);
DllExport(ULONG)  outmdword( PL_ULONG hIfc, ULONG offset, PULONG data, ULONG len, ULONG key);

DllExport(ULONG)  GetWord_DM(PL_ULONG hIfc, USHORT Addr, PUSHORT Data);
DllExport(ULONG)  PutWord_DM(PL_ULONG hIfc, USHORT Addr, USHORT Data);
DllExport(ULONG)  PutWord_PM(PL_ULONG hIfc, USHORT Addr, ULONG Data);
DllExport(ULONG)  GetWord_PM(PL_ULONG hIfc, USHORT Addr, PULONG Data);

DllExport(ULONG)  GetArray_DM(PL_ULONG hIfc, USHORT Addr, ULONG Count, PUSHORT Data);
DllExport(ULONG)  PutArray_DM(PL_ULONG hIfc, USHORT Addr, ULONG Count, PUSHORT Data);
DllExport(ULONG)  PutArray_PM(PL_ULONG hIfc, USHORT Addr, ULONG Count, PULONG Data);
DllExport(ULONG)  GetArray_PM(PL_ULONG hIfc, USHORT Addr, ULONG Count, PULONG Data);

DllExport(ULONG)  SendCommand(PL_ULONG hIfc, USHORT Cmd);

DllExport(ULONG)  PlataTest(PL_ULONG hIfc);

DllExport(ULONG)  GetSlotParam(PL_ULONG hIfc, PSLOT_PAR slPar);

DllExport(L_ULONG) OpenLDevice(PL_ULONG hIfc);
DllExport(ULONG)  CloseLDevice(PL_ULONG hIfc);

///
DllExport(ULONG)  SetParametersStream(PL_ULONG hIfc, LPVOID sp, ULONG sp_type, ULONG *UsedSize, void** Data, void** Sync, ULONG StreamId);

DllExport(ULONG)  RequestBufferStream(PL_ULONG hIfc, ULONG *Size, ULONG StreamId); //in words
DllExport(ULONG)  FillDAQparameters(PL_ULONG hIfc, LPVOID sp, ULONG sp_type);
///

DllExport(ULONG)  InitStartLDevice(PL_ULONG hIfc);
DllExport(ULONG)  StartLDevice(PL_ULONG hIfc);
DllExport(ULONG)  StopLDevice(PL_ULONG hIfc);

DllExport(ULONG)  LoadBios(PL_ULONG hIfc, char *FileName);


DllExport(ULONG)  IoAsync(PL_ULONG hIfc, PWASYNC_PAR sp);  // collect all async io operations

DllExport(ULONG)  ReadPlataDescr(PL_ULONG hIfc, LPVOID pd);
DllExport(ULONG)  WritePlataDescr(PL_ULONG hIfc, LPVOID pd, USHORT Ena);
DllExport(ULONG)  ReadFlashWord(PL_ULONG hIfc, USHORT FlashAddress, PUSHORT Data);
DllExport(ULONG)  WriteFlashWord(PL_ULONG hIfc, USHORT FlashAddress, USHORT Data);
DllExport(ULONG)  EnableFlashWrite(PL_ULONG hIfc, USHORT Flag);

DllExport(ULONG)  EnableCorrection(PL_ULONG hIfc, USHORT Ena);

DllExport(ULONG)  GetParameter(PL_ULONG hIfc, ULONG name, PULONG param);
DllExport(ULONG)  SetParameter(PL_ULONG hIfc, ULONG name, PULONG param);


DllExport(ULONG)  SetLDeviceEvent(PL_ULONG hIfc, L_ULONG hEvent, ULONG EventId);


// For LabView

DllExport(ULONG) GetSyncData(PL_ULONG hIfc, L_ULONG SyncPtr, ULONG Offset, ULONG *Sync);
// �⠥� �� ���� ����뢠����� 32 ���� �᫮� SyncPtr ���祭�� � ��६����� Sync

DllExport(ULONG) GetDataFromBuffer(PL_ULONG hIfc, L_ULONG DataPtr, LPVOID DataArray, ULONG size, ULONG mask);
// ������� ����� �� ���� �������筮

DllExport(ULONG) PutDataToBuffer(PL_ULONG hIfc, L_ULONG DataPtr, LPVOID DataArray, ULONG size);
// ������� ����� � ���� �������筮

DllExport(LPVOID) Get_LDEV2_Interface(PL_ULONG hIfc, PULONG Err); // return handle for new iface

DllExport(ULONG)  InitStartLDeviceEx(PL_ULONG hIfc, ULONG StreamId);

DllExport(ULONG)  StartLDeviceEx(PL_ULONG hIfc, ULONG StreamId);

DllExport(ULONG)  StopLDeviceEx(PL_ULONG hIfc, ULONG StreamId);

DllExport(ULONG)  Release_LDEV2_Interface(PL_ULONG hIfc);
