
#include <includes.h>
#include "./flash/bsp_spi_flash.h"


typedef enum { FAILED = 0, PASSED = !FAILED} TestStatus;


#define TxBufferSize1   (countof(TxBuffer1) - 1)
#define RxBufferSize1   (countof(TxBuffer1) - 1)
#define countof(a)      (sizeof(a) / sizeof(*(a)))
#define  BufferSize (countof(Tx_Buffer)-1)

#define  FLASH_WriteAddress     0x00000
#define  FLASH_ReadAddress      FLASH_WriteAddress
#define  FLASH_SectorToErase    FLASH_WriteAddress

     
uint8_t Tx_Buffer[] = "2018/12/10\r\n";
uint8_t Rx_Buffer[BufferSize];

__IO uint32_t DeviceID = 0;
__IO uint32_t FlashID = 0;
__IO TestStatus TransferStatus1 = FAILED;

void Delay(__IO uint32_t nCount);
TestStatus Buffercmp(uint8_t* pBuffer1,uint8_t* pBuffer2, uint16_t BufferLength);

void Test_flash(void)
{ 	

	printf("\r\n test flash \r\n");
	
	/* 8M flash W25Q64 */
	SPI_FLASH_Init();
	
	/*  Flash Device ID */
	DeviceID = SPI_FLASH_ReadDeviceID();	
	Delay( 200 );
	
	/*  SPI Flash ID */
	FlashID = SPI_FLASH_ReadID();	
	printf("\r\n FlashID is 0x%X,\
	Manufacturer Device ID is 0x%X\r\n", FlashID, DeviceID);
	
	/* SPI Flash ID */
	if (FlashID == sFLASH_ID)
	{	
		printf("\r\n flash W25Q64 !\r\n");
		

		// erase 4k
		SPI_FLASH_SectorErase(FLASH_SectorToErase);	 	 
		

		SPI_FLASH_BufferWrite(Tx_Buffer, FLASH_WriteAddress, BufferSize);		
		printf("\r\n %s \r\t", Tx_Buffer);
		
		SPI_FLASH_BufferRead(Rx_Buffer, FLASH_ReadAddress, BufferSize);
		printf("\r\n%s \r\n", Rx_Buffer);
        
		/* compare */
		TransferStatus1 = Buffercmp(Tx_Buffer, Rx_Buffer, BufferSize);
		
		if( PASSED == TransferStatus1 )
		{ 
			printf("\r\n pass!\n\r");
		}
		else
		{        
			printf("\r\n fail!\n\r");
		}
    }
	else// if (FlashID == sFLASH_ID)
	{ 
		printf("\r\nW25Q64 ID!\n\r");
	}
	
}


TestStatus Buffercmp(uint8_t* pBuffer1, uint8_t* pBuffer2, uint16_t BufferLength)
{
  while(BufferLength--)
  {
    if(*pBuffer1 != *pBuffer2)
    {
      return FAILED;
    }

    pBuffer1++;
    pBuffer2++;
  }
  return PASSED;
}

void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
}
/*********************************************END OF FILE**********************/
