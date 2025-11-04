/**
 * @file        rp_math.c
 * @author      RobotPilots
 * @Version     v1.1
 * @brief       RobotPilots Robots' Math Libaray.
 * @update
 *              v1.0(11-September-2020)
 *              v1.1(13-November-2021)
 *                  1.增加位操作函数
 */
 
/* Includes ------------------------------------------------------------------*/
#include "rp_math.h"

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  低通滤波
  * @param  
  * @retval 
  */
float Lowpass(float X_last, float X_new, float K)
{
	return (X_last + (X_new - X_last) * K);
}


int16_t RampInt(int16_t final, int16_t now, int16_t ramp)
{
	int32_t buffer = 0;
	
	buffer = final - now;
	if (buffer > 0)
	{
		if (buffer > ramp)
			now += ramp;
		else
			now += buffer;
	}
	else
	{
		if (buffer < -ramp)
			now += -ramp;
		else
			now += buffer;
	}

	return now;
}

float RampFloat(float final, float now, float ramp)
{
	float buffer = 0;
	
	buffer = final - now;
	if (buffer > 0)
	{
		if (buffer > ramp)
			now += ramp;
		else
			now += buffer;
	}
	else
	{
		if (buffer < -ramp)
			now += -ramp;
		else
			now += buffer;
	}

	return now;	
}

float DeathZoom(float input, float center, float death)
{
	if(abs(input - center) < death)
		return center;
	return input;
}

//计算平均数，方差（不断更新）
void calu_variance1(float *num,float *aver,float *variance)
{
    static uint8_t refresh_i = 0;
    static double counter = 0;
    double sq_counter = 0;
    
    refresh_i++;
    
    counter += num[refresh_i - 1];
    *aver = counter / refresh_i;
    
    for(int i = 0;i < refresh_i; i++)
    {
        sq_counter += (num[i] - *aver) * (num[i] - *aver);
    }
    if(refresh_i > 1)
        *variance = (float)sq_counter / refresh_i;
}

void calu_variance2(float *num,float *aver,float *variance)
{
    static uint8_t refresh_i = 0;
    static double counter = 0;
    double sq_counter = 0;
    
    refresh_i++;
    
    counter += num[refresh_i - 1];
    *aver = counter / refresh_i;
    
    for(int i = 0;i < refresh_i; i++)
    {
        sq_counter += (num[i] - *aver) * (num[i] - *aver);
    }
    if(refresh_i > 1)
        *variance = (float)sq_counter / refresh_i;
}

// 函数：计算(min,max)某个精度的频率数组
/**
 * 	size 数组大小
 * 	*count	频率数组	index为数据内容，data为频率
 */

void calc_frequencies(float *data, uint32_t size, float *count,float min,float max,uint8_t precision) 
{
    for (uint32_t i = 0; i < size; i++) 
	{
        int index = (int)((data[i] - min) * (float)precision);  // 将数值放大precision倍
        if (index < ((max - min)* precision) && index >= 0)
		{
            count[index]++;
        }
    }
}

// 函数：计算前 N 个众数的平均值
void calc_top_n_mode_avg(float *count,uint8_t precision,uint8_t need_mode_num,float mode_aver,float min,float max) 
{
    uint32_t max_count = 0;  // 最大出现次数
	uint8_t all_num = (max - min)*precision;

    for (uint32_t i = 0; i < all_num; i++) 	//根据频率数组确定最大频率
	{
        if (count[i] > max_count) {
            max_count = count[i];
        }
    }

	//根据最大频率反推确定数字
    uint32_t top_n_count = 0;
    for (uint32_t i = 0; i < all_num; i++) 
	{
        if (count[i] == max_count) 
		{
            count[top_n_count] = i / (float)(precision) + min;  // 获取实际值并存储
            top_n_count++;
        }
        if (top_n_count >= need_mode_num) {
            break;
        }
    }
    
    float temp = 0;
    // Step 4: 计算这些前need_mode_num个众数的平均值
    for (uint32_t i = 0; i < top_n_count; i++) {
        temp += count[i];
    }
    mode_aver = temp / top_n_count;  // 计算平均值
}

