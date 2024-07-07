#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../../SSPS3_Master_Domain/include/main.hpp"
#include "FRAM_DB.hpp"
#include "UIService.hpp"
#include "ProgramControl.hpp"
#include "BlowingControl.hpp"
#include "RtTaskManager.hpp"
#include "../resource/lv_resources.hpp"

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

extern TwoWire * itcw;
extern STM32_slave * STM32;
extern FRAM_DB * FRAM_db;
extern ProgramControl * Program_control;
extern BlowingControl * Blowing_control;
extern UIService * UI_service;

static RtTaskManager rt_task_manager = RtTaskManager();

void IRAM_ATTR interrupt_action() {
    interrupted_by_slave = true;
}

uint8_t OptIn_state[8] = { 0 };