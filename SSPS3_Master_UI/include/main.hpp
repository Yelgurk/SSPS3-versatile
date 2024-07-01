#include "UIService.hpp"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../resource/lv_resources.hpp"

#include "UIControls/UIDateTime.hpp"
#include "UIControls/UIFlowGunProgressBar.hpp"
#include "UIControls/UIList.hpp"
#include "UIControls/UIMachineStateBar.hpp"
#include "UIControls/UIObject.hpp"
#include "UIControls/UIScreen.hpp"
#include "UIControls/UITaskListItem.hpp"
#include "UIControls/UITaskRoadmapList.hpp"
#include "UIControls/UIValueSetter.hpp"

#ifndef SSPS3_APPLICATION_SOLUTION

#define SDA                         48
#define SCL                         47
#define INT                         12
#define STM_I2C_ADDR                0x30

extern TwoWire * itcw;
extern STM32_slave * STM32;

void print();

uint8_t OptIn_state[8] = { 0 };

#endif

UIService UI_service;
UITaskRoadmapList * UI_task_roadmap_control;
UIDateTime * UI_date_time;
UIMachineStateBar * UI_machine_state_bar;