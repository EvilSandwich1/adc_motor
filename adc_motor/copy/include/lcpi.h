class DaqLCPI: public LDaqBoard
{
public:
   // Base functions
   IFC(ULONG) GetWord_DM(USHORT Addr, PUSHORT Data);
   IFC(ULONG) PutWord_DM(USHORT Addr, USHORT Data); // Addr кодирует 0-DM 1-PM 

   IFC(ULONG) GetArray_DM(USHORT Addr, ULONG Count, PUSHORT Data);
   IFC(ULONG) PutArray_DM(USHORT Addr, ULONG Count, PUSHORT Data); // Addr кодирует 0-DM 1-PM 
   
   IFC(ULONG) SendCommand(USHORT cmd);
      
      // Service functions
   IFC(ULONG) PlataTest();
      
   IFC(ULONG)  LoadBios(char *FileName);

public:
   DaqLCPI(ULONG Slot) :LDaqBoard(Slot) {}

   ULONG csGetParameter(ULONG name, PULONG param, ULONG status);
   ULONG csSetParameter(ULONG name, PULONG param, ULONG status);
   ULONG FillADCparameters(PDAQ_PAR sp);

protected:
};
