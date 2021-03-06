/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FREERTOS_MEMORY_H
#define FREERTOS_MEMORY_H


#include "cmsis_os.h"
#include "stdlib.h"


void *operator new(size_t size);
void *operator new[](size_t size);
void operator delete(void *p) noexcept;
void operator delete[](void *p) noexcept;

#endif //FREERTOS_MEMORY_H
