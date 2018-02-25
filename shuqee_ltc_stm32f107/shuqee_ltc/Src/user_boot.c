#include "stm32f1xx_hal.h"
#include "user_config.h"
#include "user_boot.h"
#include "bootloader_bin.h"
#include "user_flash.h"
#include <string.h>

#pragma pack(1)
typedef struct boot_status
{
	bool update_flag;
} boot_status_t;
#pragma pack()

boot_status_t *boot_status = (boot_status_t *)USER_FLASH_BOOT_DATA_BASE;
const uint8_t bootloader_bin[] __attribute__((at(USER_FLASH_BOOTLOADER_BASE))) = {BOOTLOADER_BIN};
const boot_status_t boot_status_init __attribute__((at(USER_FLASH_BOOT_DATA_BASE))) = {
.update_flag = false
};
typedef void (*pFunction)(void);

void boot_set_update_flag(void)
{
	bool update_flag;
	update_flag = true;
	user_flash_write((uint32_t)&(boot_status->update_flag),
	                 sizeof(update_flag),
				     (uint8_t *)&(update_flag));
}

void boot_clean_update_flag(void)
{
	bool update_flag;
	update_flag = false;
	user_flash_write((uint32_t)&(boot_status->update_flag),
	                 sizeof(update_flag),
				     (uint8_t *)&(update_flag));
}
