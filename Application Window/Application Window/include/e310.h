


class DaqE310: public LDaqBoard
{
public:
   // Service functions
   IFC(ULONG) PlataTest();

   IFC(ULONG)  ReadPlataDescr(LPVOID pd);
   IFC(ULONG)  WritePlataDescr(LPVOID pd, USHORT Ena);

public:
   DaqE310(ULONG Slot) :LDaqBoard(Slot) {}

   ULONG InputTTL(PDAQ_PAR sp);
   ULONG OutputTTL(PDAQ_PAR sp);
   ULONG ConfigTTL(PDAQ_PAR sp);

   ULONG ConfigADC(PDAQ_PAR sp);
   ULONG InputADC(PDAQ_PAR sp);


   inline USHORT CRC16CALC(UCHAR *Buffer, USHORT Size)
   {
      USHORT crc = 0x0;
      USHORT i, j;

      for(i = 0x0; i < Size; i++)
      {
         crc ^= (USHORT)((USHORT)(Buffer[i]) << 0x8);
         for(j = 0x0; j < 8; j++)
         {
            if(crc & (USHORT)0x8000) crc = (USHORT)((crc << 0x1) ^ 0x8005); else crc <<= 0x1;
         }
      }
      return crc;
   }


   ULONG csGetParameter(ULONG name, PULONG param, ULONG status);
   ULONG csSetParameter(ULONG name, PULONG param, ULONG status);

   ULONG FillDDSparameters(PDAQ_PAR sp);

protected:
};
