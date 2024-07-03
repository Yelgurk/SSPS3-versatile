#include "UIService.hpp"
#include "../../SSPS3_Master_Infrastructure/include/main.hpp"
#include "../resource/lv_resources.hpp"

#include "UIControls/UIBlowingControl.hpp"
#include "UIControls/UIBlowValListItem.hpp"
#include "UIControls/UIDateTime.hpp"
#include "UIControls/UIFlowGunProgressBar.hpp"
#include "UIControls/UIMachineStateBar.hpp"
#include "UIControls/UIMenuList.hpp"
#include "UIControls/UIMenuListItem.hpp"
#include "UIControls/UINotifyBar.hpp"
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

UIMachineStateBar * UI_machine_state_bar;
UIDateTime * UI_date_time;
UINotifyBar * UI_notify_bar;

UITaskRoadmapList * UI_task_roadmap_control;
vector<UITaskListItem> * UI_task_steps;

UIMenuList * UI_menu_list_user;
UIMenuListItem * UI_settings_user_datetime;
UIMenuListItem * UI_settings_user_pump;
UIMenuListItem * UI_settings_user_pasteurizer_template_1;
UIValueSetter * UI_Set1;
UIValueSetter * UI_Set2;
UIValueSetter * UI_Set3;

UIBlowingControl * UI_blowing_control;
UIBlowValListItem * Blow_var_1;
UIBlowValListItem * Blow_var_2;
UIBlowValListItem * Blow_var_3;
UIBlowValListItem * Blow_var_timer;