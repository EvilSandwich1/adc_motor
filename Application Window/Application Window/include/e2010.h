class DaqE2010: public LDaqBoard
{
public:
   // Base functions
   //IFC(ULONG) GetArray_DM(USHORT Addr, ULONG Count, PUSHORT Data);
   //IFC(ULONG) PutArray_DM(USHORT Addr, ULONG Count, PUSHORT Data);
   //IFC(ULONG) PutArray_PM(USHORT Addr, DWORD Count, PULONG Data);
   //IFC(ULONG) GetArray_PM(USHORT Addr, DWORD Count, PULONG Data);

   //IFC(ULONG) SendCommand(USHORT cmd);


   // Service functions
   IFC(ULONG) PlataTest();
   IFC(ULONG) EnableCorrection(USHORT Ena);
      
   IFC(ULONG)  LoadBios(char *FileName);

   IFC(ULONG)  ReadPlataDescr(LPVOID pd);
   IFC(ULONG)  WritePlataDescr(LPVOID pd, USHORT Ena);

public:
   DaqE2010(ULONG Slot) :LDaqBoard(Slot) {}
   ULONG  FillADCparameters(PDAQ_PAR sp);

   ULONG InputTTL(PDAQ_PAR sp);  //2 in 1 all
   ULONG OutputTTL(PDAQ_PAR sp);  // in each set channel
   ULONG ConfigTTL(PDAQ_PAR sp); // 1221 and 1450 780C e440

   ULONG OutputDAC(PDAQ_PAR sp); //2 in 1
//   ULONG InputADC(PDAQ_PAR sp);

   USHORT CRC16CALC(UCHAR *Buffer, USHORT Size)
   {
      USHORT w = 0x0;
      USHORT i, j;

      for(i = 0x0; i < Size; i++)
      {
         w ^= (USHORT)((USHORT)(Buffer[i]) << 0x8);
         for(j = 0x0; j < 8; j++)
         {
            if(w & (USHORT)0x8000) w = (USHORT)((w << 0x1) ^ 0x8005);
            else w <<= 0x1;
         }
      }
      return w;
   }
   ULONG csGetParameter(ULONG name, PULONG param, ULONG status);
   ULONG csSetParameter(ULONG name, PULONG param, ULONG status);

protected:
};
