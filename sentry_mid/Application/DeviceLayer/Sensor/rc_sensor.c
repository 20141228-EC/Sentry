/**
 * @file        rc_sensor.c
 * @author      RobotPilots@2020
 * @Version     V1.0
 * @date        9-September-2020
 * @brief       Device Rc.
 */
 
/* Includes ------------------------------------------------------------------*/
#include "rc_sensor.h"
#include "rp_math.h"

void rc_sensor_init(rc_sensor_t *rc_sen);
void rc_sensor_update(rc_sensor_t *rc_sen, uint8_t *rxBuf);
void rc_sensor_heart_beat(rc_sensor_t *rc_sen);
void rc_sensor_check(rc_sensor_t *rc_sen);
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

static void keyboard_cnt_max_set(rc_sensor_t *rc_sen);
static void keyboard_status_update(key_board_info_t *key);


/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
// 遥控器驱动
drv_uart_t	rc_sensor_driver = {
	.id = DRV_UART2,
	.tx_byte = NULL,
};

// 遥控器信息
rc_sensor_info_t 	rc_sensor_info = {
	
	.offline_max_cnt = 60,
};

// 遥控器传感器
rc_sensor_t	rc_sensor = {
	.info = &rc_sensor_info,
	.init = rc_sensor_init,
	.update = rc_sensor_update,
	.check = rc_sensor_check,
	.heart_beat = rc_sensor_heart_beat,
	.work_state = DEV_OFFLINE,
	.id = DEV_ID_RC,
};

/* Private functions ---------------------------------------------------------*/
void rc_sensor_init(rc_sensor_t *rc_sen)
{
	// 初始化为离线状态
	rc_sen->info->offline_cnt = rc_sen->info->offline_max_cnt + 1;
	rc_sen->work_state = DEV_OFFLINE;
	
	//清零
	RC_ResetData(rc_sen);
	
	//按键长按时间设置
	keyboard_cnt_max_set(rc_sen);
	
	if(rc_sen->id == DEV_ID_RC)
		rc_sen->errno = NONE_ERR;
	else
		rc_sen->errno = DEV_ID_ERR;
}

/**
 *	@brief	遥控器数据检查
 *  step[0]:拨轮推到顶跳变
 *  step[1]:拨轮往上推一点跳变
 *  step[2]:拨轮推到底跳变
 *  step[3]:拨轮往下推一点跳变
 *  不知道谁写的抽象玩意，注释没有一点
 */
static void rc_sensor_check(rc_sensor_t *rc_sen)
{
	rc_sensor_info_t *rc_info = rc_sen->info;
	
	if(abs(rc_info->ch0) > 660 ||
	   abs(rc_info->ch1) > 660 ||
	   abs(rc_info->ch2) > 660 ||
	   abs(rc_info->ch3) > 660)
	{
		rc_sen->errno = DEV_DATA_ERR;
		rc_info->ch0 = 0;
		rc_info->ch1 = 0;
		rc_info->ch2 = 0;
		rc_info->ch3 = 0;		
		rc_info->s1.value = RC_SW_MID;
		rc_info->s2.value = RC_SW_MID;
		rc_info->thumbwheel.value = 0;
		rc_info->thumbwheel.value_last = 0;
	}
	else
	{
		rc_sen->errno = NONE_ERR;
	}
}

/**
 *	@brief	遥控器心跳包
 */
void rc_sensor_heart_beat(rc_sensor_t *rc_sen)	//去掉了static
{
	rc_sensor_info_t *rc_info = rc_sen->info;

	rc_info->offline_cnt++;
	if(rc_info->offline_cnt > rc_info->offline_max_cnt) {
		rc_info->offline_cnt = rc_info->offline_max_cnt;
		rc_sen->work_state = DEV_OFFLINE;
	} 
	else {
		/* 离线->在线 */
		if(rc_sen->work_state == DEV_OFFLINE)
		{
			rc_sen->work_state = DEV_ONLINE;
		}
	}
}

/************************************ 外部可调用功能函数 ***************************************/
/**
 *	@brief	小于50就视为回中
 */
bool RC_IsChannelReset(void)		
{
	if(  (DeathZoom(rc_sensor_info.ch0, 0, 50) == 0) && 
		 (DeathZoom(rc_sensor_info.ch1, 0, 50) == 0) && 
		 (DeathZoom(rc_sensor_info.ch2, 0, 50) == 0) && 
		 (DeathZoom(rc_sensor_info.ch3, 0, 50) == 0))	
	{
		return true;
	}
	return false;		
}

/**
  * @brief  鼠标数据更新
  */
void rc_interrupt_update(rc_sensor_t *rc_sen)
{
	/* 鼠标速度均值滤波 */
	static int16_t mouse_x[REMOTE_SMOOTH_TIMES], mouse_y[REMOTE_SMOOTH_TIMES];
	static int16_t index = 0;
	if(index == REMOTE_SMOOTH_TIMES)
	{
		index = 0;
	}
	rc_sen->info->mouse_x -= (float)mouse_x[index] / (float)REMOTE_SMOOTH_TIMES;
	rc_sen->info->mouse_y -= (float)mouse_y[index] / (float)REMOTE_SMOOTH_TIMES;
	mouse_x[index] = rc_sen->info->mouse_vx;
	mouse_y[index] = rc_sen->info->mouse_vy;
	rc_sen->info->mouse_x += (float)mouse_x[index] / (float)REMOTE_SMOOTH_TIMES;
	rc_sen->info->mouse_y += (float)mouse_y[index] / (float)REMOTE_SMOOTH_TIMES;
	
	index++;
	
}

/**
 *	@brief	更新键盘状态
 */
void keyboard_update(rc_sensor_info_t	*info)
{
  keyboard_status_update(&info->mouse_btn_l);
  keyboard_status_update(&info->mouse_btn_r);
  keyboard_status_update(&info->Q);
  keyboard_status_update(&info->W);
  keyboard_status_update(&info->E);
  keyboard_status_update(&info->R);
  keyboard_status_update(&info->A);
  keyboard_status_update(&info->S);
  keyboard_status_update(&info->D);
  keyboard_status_update(&info->F);
  keyboard_status_update(&info->G);
  keyboard_status_update(&info->Z);
  keyboard_status_update(&info->X);
  keyboard_status_update(&info->C);
  keyboard_status_update(&info->V);
  keyboard_status_update(&info->B);
  keyboard_status_update(&info->Shift);
  keyboard_status_update(&info->Ctrl);
}

/**
 *	@brief	s跳变更新
 */
void rc_sensor_s_step(rc_sensor_t *rc_sen)	//24.12.11已测试
{
	rc_sen->info->s1.value_last = rc_sen->info->s1.value;
	rc_sen->info->s2.value_last = rc_sen->info->s2.value;
}

/**
 *	@brief	拇指跳变更新
 *  @note	实现中间拨到上位和中间拨到下位,MID2UP和MID2DOWN翻转
 */
void rc_sensor_thumbwheel_step(rc_sensor_t *rc_sen)			//24.12.11已测试
{
	if(rc_sen->info->thumbwheel.value == 0 && rc_sen->info->thumbwheel.value_last == -600)
	{
		if(rc_sen->info->thumbwheel.MID2UP == false)
			rc_sen->info->thumbwheel.MID2UP = true;
		else
			rc_sen->info->thumbwheel.MID2UP = false;

		rc_sen->info->thumbwheel.value_last = 0;
	}
	else if(rc_sen->info->thumbwheel.value == 0 && rc_sen->info->thumbwheel.value_last == 600)
	{
		if(rc_sen->info->thumbwheel.MID2DOWN == false)
			rc_sen->info->thumbwheel.MID2DOWN = true;
		else
			rc_sen->info->thumbwheel.MID2DOWN = false;

		rc_sen->info->thumbwheel.value_last = 0;
	}

	if(rc_sen->info->thumbwheel.value >= 600)
	{
		rc_sen->info->thumbwheel.value_last = 600;
	}
	else if(rc_sen->info->thumbwheel.value <= -600)
	{
		rc_sen->info->thumbwheel.value_last = -600;
	}
	
}

/**
 *	@brief	遥控器数据重置
 */
void RC_ResetData(rc_sensor_t *rc)
{
	// 通道值强行设置成中间值(不拨动摇杆的状态)
	rc->info->ch0 = 0;
	rc->info->ch1 = 0;
	rc->info->ch2 = 0;
	rc->info->ch3 = 0;
	// 左右开关选择强行设置成中间值状态
	rc->info->s1.value = RC_SW_MID;
	rc->info->s2.value = RC_SW_MID;
	rc->info->s1.value_last = RC_SW_MID;
	rc->info->s2.value_last = RC_SW_MID;
	// 鼠标
	rc->info->mouse_vx = 0;
	rc->info->mouse_vy = 0;
	rc->info->mouse_vz = 0;
	rc->info->mouse_x = 0.f;
	rc->info->mouse_y = 0.f;
	rc->info->mouse_z = 0.f;
	rc->info->mouse_btn_l.value = 0;
	rc->info->mouse_btn_r.value = 0;
	// 键盘
	rc->info->key_v = 0;
    rc->info->W.value = 0;
    rc->info->S.value = 0;
    rc->info->A.value = 0;
    rc->info->D.value = 0;
    rc->info->Shift.value = 0;
    rc->info->Ctrl.value = 0;
    rc->info->Q.value = 0;
    rc->info->E.value = 0;
    rc->info->R.value = 0;
    rc->info->F.value = 0;
    rc->info->G.value = 0;
    rc->info->Z.value = 0;
    rc->info->X.value = 0;
    rc->info->C.value = 0;
    rc->info->V.value = 0;
    rc->info->B.value = 0;
	// 左拨轮
	rc->info->thumbwheel.value = 0;
	rc->info->thumbwheel.value_last = 0;
	rc->info->thumbwheel.MID2UP = false;
	rc->info->thumbwheel.MID2DOWN = false;
}
/************************************ 功能小函数 ***************************************/
/**
 *	@brief	更新键盘按键状态
 *  release -> release_to_press -> short_press -> long_press -> press_to_release
 */
static void keyboard_status_update(key_board_info_t *key)
{
	key->last_status = key->status;

    switch(key->value)
    {
        case 0:
        {
            if(key->cnt != 0)
            {
                key->status = press_to_release;
                key->cnt = 0;
            }
            else
            {
                key->status = release;
                key->cnt = 0;
            }
            break;
        }
        case 1:
        {
            key->cnt++;
            if(key->cnt == 1)
            {
                key->status = release_to_press;
            }
            else if(key->cnt >= key->cnt_max)
            {
                key->status = long_press;
				key->cnt = key->cnt_max;
            }
            else
            {
                key->status = short_press;
            }
			
        } 
    }
}

/**
  * @brief  按键长按时间设置
  */
static void keyboard_cnt_max_set(rc_sensor_t *rc_sen)
{
  rc_sensor_info_t *info = rc_sen->info;
  info->mouse_btn_l.cnt_max = MOUSE_BTN_L_CNT_MAX;
  info->mouse_btn_r.cnt_max = MOUSE_BTN_R_CNT_MAX;
  info->Q.cnt_max = KEY_Q_CNT_MAX;
  info->W.cnt_max = KEY_W_CNT_MAX;
  info->E.cnt_max = KEY_E_CNT_MAX;
  info->R.cnt_max = KEY_R_CNT_MAX;
  info->A.cnt_max = KEY_A_CNT_MAX;
  info->S.cnt_max = KEY_S_CNT_MAX;
  info->D.cnt_max = KEY_D_CNT_MAX;
  info->F.cnt_max = KEY_F_CNT_MAX;
  info->G.cnt_max = KEY_G_CNT_MAX;
  info->Z.cnt_max = KEY_Z_CNT_MAX;
  info->X.cnt_max = KEY_X_CNT_MAX;
  info->C.cnt_max = KEY_C_CNT_MAX;
  info->V.cnt_max = KEY_V_CNT_MAX;
  info->B.cnt_max = KEY_B_CNT_MAX;
  info->Shift.cnt_max = KEY_SHIFT_CNT_MAX;
  info->Ctrl.cnt_max = KEY_CTRL_CNT_MAX;
}
