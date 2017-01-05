#ifndef VS1063A_SPI_H
#define VS1063A_SPI_H

void WriteSci(uint8_t addr, uint16_t data);
uint16_t ReadSci(uint8_t addr);
int WriteSdi(const uint8_t *data, uint32_t bytes);

void  VS1063InitSPI(void);
void  VS1063AdjustSPIFast(void); /*Adjust SPI clock to fast*/

#endif /*VS1063A_SPI_H*/
