#include <stdint.h>

typedef struct { volatile int counter; } atomic_t;

class SAL // system abstraction layer
{
public:
	SAL();
	~SAL();
   void LSetLastError(uint32_t Err);
   uint32_t LGetLastError(void);

	void * LCreateFile(int slot);
	int    LCloseHandle(void * hDevice);
	uint32_t LDeviceIoControl(void *     hDevice,
                             uint32_t   dwIoControlCode,
                             void *     lpInBuffer,
                             uint32_t   nInBufferSize,
                             void *     lpOutBuffer,
                             uint32_t   nOutBufferSize,
                             uint32_t * lpBytesReturned,
                             uint32_t   StreamId);

   uint32_t LWaitOverlapped(void * hDevice, uint32_t StreamId); // ждет завершения overlapped операции

   void LUnmapMemory(void * lpAddr, int32_t Size );
   
   void * LMapMemory(void * hDevice, void * lpAddr, int32_t Size, int32_t off);
   
   void * LVirtualAlloc(size_t size);
   int LVirtualFree(void * addr);
   int LIsWin32();
   uint32_t CTL(uint32_t cmd);


//	MemAlloc(int StreamId);
//	MemFree(int StreamId);
//	MemMap(int StreamId);
//	MemUnmap(int StreamId);


   void atomic_inc(atomic_t *v);
   void atomic_dec(atomic_t *v);

private:
	class sySAL;
	sySAL *d;	
};
