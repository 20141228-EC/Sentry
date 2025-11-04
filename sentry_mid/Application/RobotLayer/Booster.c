#include "Booster.h"

booster_info_t booster_info = {
    .shoot_v = FRIC_25,
    .shoot_f = DIAL_25_SPEED,
    .angle = 0,
};

booster_t booster = {
    .DIAL_motor = &rm_motor[DIAL],
    .L_FRIC_motor = &rm_motor[FRIC_L],
    .R_FRIC_motor = &rm_motor[FRIC_R],

    .info = &booster_info,
    .work_state = DEV_OFFLINE,
    .ready_flag = 0,
};


static void FRIC_TO_SPEED(int16_t speed);
static void DIAL_TO_SPEED(int16_t speed);
static void DIAL_TO_ANGLE(int32_t angle);

static void ready_shoot(booster_t *booster);
static void single_shoot(booster_t *booster);
static void keep_shoot(booster_t *booster);
static void sleep_shoot(booster_t *booster);

void Booster_CTRL(booster_t *booster)
{	
    switch(booster->mode)
    {
        case READY:
        {
            ready_shoot(booster);
            break;
        }
        case SINGEL:
        {
            single_shoot(booster);
            break;
        }
        case KEEP:
        {
            keep_shoot(booster);
            break;
        }
        case SLEEP:
        {
            sleep_shoot(booster);
            break;
        }
		case CLEAR:
		{
			
			break;
		}
    }
}

static void ready_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(booster->info->shoot_v);
	DIAL_TO_SPEED(0);

	booster->info->angle = rm_motor[DIAL].info->angle_sum;
	if((abs(rm_motor[DIAL].info->speed - 0) < 50) && (abs(rm_motor[FRIC_L].info->speed - booster->info->shoot_v) < 20 || abs(rm_motor[FRIC_R].info->speed - booster->info->shoot_v) < 20))
	{
		booster->ready_flag = 1;
	}
}

static void single_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(booster->info->shoot_v);
	DIAL_TO_ANGLE(booster->info->angle);

	Motor_HandleStuck(&rm_motor[DIAL]);

}

static void keep_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(booster->info->shoot_v);
	DIAL_TO_SPEED(booster->info->shoot_f);
	
	Motor_HandleStuck(&rm_motor[DIAL]);

	booster->info->angle = rm_motor[DIAL].info->angle_sum;
	
}

static void sleep_shoot(booster_t *booster)
{
	FRIC_TO_SPEED(0);
	DIAL_TO_SPEED(0);

	booster->ready_flag = 0;
}


static void FRIC_TO_SPEED(int16_t speed)
{
	Motor_ToSpeed(&rm_motor[FRIC_L],speed);
	Motor_ToSpeed(&rm_motor[FRIC_R],-speed);
}

static void DIAL_TO_SPEED(int16_t speed)
{
	Motor_ToSpeed(&rm_motor[DIAL],speed);
}

static void DIAL_TO_ANGLE(int32_t angle)
{
	Motor_ToAxleAngle(&rm_motor[DIAL],angle);
}