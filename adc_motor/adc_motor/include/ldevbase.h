class LDaqBoard: public IDaqLDevice, public IDaqLDevice2
{
public:
      virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv);
      virtual ULONG   __stdcall AddRef();
      virtual ULONG   __stdcall Release();

      IFC(ULONG) inbyte( ULONG offset, PUCHAR data, ULONG len=1, ULONG key=0);
      IFC(ULONG) inword ( ULONG offset, PUSHORT data, ULONG len=2, ULONG key=0);
      IFC(ULONG) indword( ULONG offset, PULONG data, ULONG len=4, ULONG key=0);

      IFC(ULONG) outbyte ( ULONG offset, PUCHAR data, ULONG len=1, ULONG key=0);
      IFC(ULONG) outword ( ULONG offset, PUSHORT data, ULONG len=2, ULONG key=0);
      IFC(ULONG) outdword( ULONG offset, PULONG data, ULONG len=4, ULONG key=0);

      IFC(ULONG) inmbyte ( ULONG offset, PUCHAR data, ULONG len=1, ULONG key=0);
      IFC(ULONG) inmword ( ULONG offset, PUSHORT data, ULONG len=2, ULONG key=0);
      IFC(ULONG) inmdword( ULONG offset, PULONG data, ULONG len=4, ULONG key=0);

      IFC(ULONG) outmbyte ( ULONG offset, PUCHAR  data, ULONG len=1, ULONG key=0);
      IFC(ULONG) outmword ( ULONG offset, PUSHORT  data, ULONG len=2, ULONG key=0);
      IFC(ULONG) outmdword( ULONG offset, PULONG data, ULONG len=4, ULONG key=0);

      // Base functions
      IFC(ULONG) GetWord_DM(USHORT Addr, PUSHORT Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) PutWord_DM(USHORT Addr, USHORT Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) PutWord_PM(USHORT Addr, ULONG Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) GetWord_PM(USHORT Addr, PULONG Data) { return L_NOTSUPPORTED;}

      IFC(ULONG) GetArray_DM(USHORT Addr, ULONG Count, PUSHORT Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) PutArray_DM(USHORT Addr, ULONG Count, PUSHORT Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) PutArray_PM(USHORT Addr, ULONG Count, PULONG Data) { return L_NOTSUPPORTED;}
      IFC(ULONG) GetArray_PM(USHORT Addr, ULONG Count, PULONG Data) { return L_NOTSUPPORTED;}

      IFC(ULONG) SendCommand(USHORT cmd) { return L_NOTSUPPORTED;}

      // Service functions
      IFC(ULONG) PlataTest() { return L_NOTSUPPORTED;}

      IFC(ULONG) GetSlotParam(PSLOT_PAR slPar);// { return 0;}

      // Common functions
      IFC(HANDLE) OpenLDevice();
      IFC(ULONG)  CloseLDevice();

      IFC(ULONG)  SetParametersStream(PDAQ_PAR sp, ULONG *UsedSize, void** Data, void** Sync, ULONG StreamId = L_STREAM_ADC);
      IFC(ULONG)  RequestBufferStream(ULONG *Size, ULONG StreamId = L_STREAM_ADC); //in words
      IFC(ULONG)  FillDAQparameters(PDAQ_PAR sp);// {return L_NOTSUPPORTED; }

// two step must be
      IFC(ULONG)  InitStartLDevice();
      IFC(ULONG)  StartLDevice();
      IFC(ULONG)  StopLDevice();

      IFC(ULONG)  LoadBios(char *FileName) { return L_NOTSUPPORTED;}

      IFC(ULONG)  IoAsync(PDAQ_PAR sp);  // collect all async io operations

      IFC(ULONG) ReadPlataDescr(LPVOID pd) {return L_NOTSUPPORTED;}
      IFC(ULONG) WritePlataDescr(LPVOID pd, USHORT Ena=0) {return L_NOTSUPPORTED;} // ena - enables owerwrite 32 first word
      IFC(ULONG) ReadFlashWord(USHORT Addr, PUSHORT Data) {return L_NOTSUPPORTED;} // and byte
      IFC(ULONG) WriteFlashWord(USHORT Addr, USHORT Data) {return L_NOTSUPPORTED;}
      IFC(ULONG) EnableFlashWrite(USHORT Flag) {return L_NOTSUPPORTED;}

      IFC(ULONG) EnableCorrection(USHORT Ena) {return L_NOTSUPPORTED;}

      IFC(ULONG) GetParameter(ULONG name, PULONG param);/* {return L_NOTSUPPORTED;}*/
      IFC(ULONG) SetParameter(ULONG name, PULONG param);/* {return L_NOTSUPPORTED;}*/

      IFC(ULONG)  SetLDeviceEvent(HANDLE hEvent,ULONG EventId = L_STREAM_ADC)/* {return L_NOTSUPPORTED; }  */;

      IFC(ULONG)  InitStartLDeviceEx(ULONG StreamId);
      IFC(ULONG)  StartLDeviceEx(ULONG StreamId);
      IFC(ULONG)  StopLDeviceEx(ULONG StreamId);

public:
   LDaqBoard(ULONG Slot)
   {
      sal = new SAL();

   	m_cRef.counter =0;
   	m_Slot=Slot;
   	hVxd= INVALID_HANDLE_VALUE;
   	//for(int i=0;i<5;i++) hEvent[i] = 0; // обнулили массив событий
//      VBuffer = NULL;
//      DBuffer = NULL;

      map_inSize = 0;
      map_inBuffer = 0;
      map_outSize = 0;
      map_outBuffer = 0;
	}

   ~LDaqBoard() { delete sal;}

public:
   virtual ULONG  FillADCparameters(PDAQ_PAR sp) {return L_NOTSUPPORTED;}
   virtual ULONG  FillDACparameters(PDAQ_PAR sp) {return L_NOTSUPPORTED;}
   virtual ULONG  FillDDSparameters(PDAQ_PAR sp) {return L_NOTSUPPORTED;}

   virtual ULONG InputTTL(PDAQ_PAR sp) {return L_NOTSUPPORTED;} //2 in 1 all
   virtual ULONG OutputTTL(PDAQ_PAR sp) {return L_NOTSUPPORTED;}  // in each set channel
   virtual ULONG ConfigTTL(PDAQ_PAR sp) {return L_NOTSUPPORTED;} // 1221 and 1450

   virtual ULONG ConfigDAC(PDAQ_PAR sp) {return L_NOTSUPPORTED;}
   virtual ULONG OutputDAC(PDAQ_PAR sp) {return L_NOTSUPPORTED;} //2 in 1

   virtual ULONG InputADC(PDAQ_PAR sp) {return L_NOTSUPPORTED;}
   virtual ULONG ConfigADC(PDAQ_PAR sp) {return L_NOTSUPPORTED;}

// class specific function to extend base
/*
public:
   virtual HANDLE csOpenLDevice() { return hVxd; }
   virtual ULONG  csCloseLDevice(ULONG status) { return status; }
   virtual ULONG  csRequestBufferStream(ULONG *Size, ULONG StreamId, ULONG status) { return status; } //in words
   virtual ULONG  csSetParametersStream(PDAQ_PAR sp, PULONG UsedSize, void** Data, void** Sync, ULONG StreamId, ULONG status) { return status; }
*/
   virtual ULONG  csGetParameter(ULONG name, PULONG param, ULONG status) {return status;}
   virtual ULONG  csSetParameter(ULONG name, PULONG param, ULONG status) {return status;}

   virtual LPVOID GetDAQ_PtrSz(ULONG StreamId, ULONG *sz);

// service function transfer data from C++ struct to C struct, wrapper C interface and Linux
public:
	virtual void CopyDAQtoWDAQ(PDAQ_PAR dp, LPVOID ss, int sp_type);

   SAL *sal; // system abstraction layer class

private:
	atomic_t    m_cRef;
   ULONG       m_Slot;

protected:
   //  this is DEV_ALL
   HANDLE      hVxd;

   //HANDLE      hEvent[5]; // for ovelapped DIOC_START under NT
   //OVERLAPPED  ov[20];   // mem flag +10 ov[streamid+mem] for mem alloc request

   SLOT_PAR sl;

   ADC_PAR adc_par;  // to fill with FillDAQparam
   DAC_PAR dac_par;

   //DAC_PAR dds_par;
   //DDS_FM_PAR fm_par;

   // add for C-style driver in Linux
   WDAQ_PAR wadc_par;
   WDAQ_PAR wdac_par;

   PLATA_DESCR_U2 pdu;

	// size and pointers for data buffers in Linux
   int map_inSize;
   void *map_inBuffer;

   int map_outSize;
   void *map_outBuffer;

   int map_regSize;
   void *map_regBuffer;

   // set/get property
   ULONG *sync_addr;
   ULONG *data_addr;

   ULONG *sync_addr1;
   ULONG *data_addr1;

	ULONG user_prop[128];

   //PVOID VBuffer;
   //HANDLE      hEventV; // for ovelapped DIOC_START under NT
   //OVERLAPPED  ovV;

   //PVOID DBuffer;
   //HANDLE      hEventD; // for ovelapped DIOC_START under NT
   //OVERLAPPED  ovD;
};

