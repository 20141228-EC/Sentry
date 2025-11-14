#ifndef _SERIALPORT_H_ // 头文件保护宏，避免重复包含
#define _SERIALPORT_H_

#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <termios.h>
#include <linux/serial.h>
#include <memory.h>
// #include <opencv2/opencv.hpp>
#include "CRC_Check.h"

/**
 * 敌方位置（大yaw系） z=0代表未发现
 */
struct EnemyPosition
{
    int16_t x;
    int16_t y;
    int16_t z = 0;
} __attribute__((packed));

struct EnemyData
{
    uint8_t target_enemy_num;            // 目标敌方兵种
    EnemyPosition target_enemy_position; // 目标兵种位置

    /* 敌方位置（未发现则默认z为0） 1英雄 2工程 3-5步兵  6哨兵 7前哨站 8基地 */
    EnemyPosition enemys_positions[8];

} __attribute__((packed));

// 大世界坐标，来自雷达和uwb
struct CoordInfo
{
    uint8_t x;
    uint8_t y;
} __attribute__((packed));

// 视觉->电控
struct VisionData
{
    uint8_t header = 0xA5;
    uint8_t mode;
    uint8_t CRC8 = 0;

    uint8_t if_hit = 1; // 控制电控是否开枪
    uint8_t nuc_flag;   // 区分小电脑
    uint8_t hit_mode;   //

    uint16_t add_bullet_num; // 兑换子弹数量
    uint8_t if_revive;      // 是否复活，默认为1

    // 发给自瞄控制击打目标
    uint8_t no_hit_enemy2 : 1; // 不打工程
    uint8_t no_hit_enemy6 : 1; // 不打哨兵
    uint8_t no_hit_enemy7 : 1; // 不打前哨
    uint8_t no_hit_enemy8 : 1; // 不打基地

    uint8_t nav_mode; // 导航

    // 导航速度
    int16_t vx;
    int16_t vy;

    int16_t nav_yaw; // 雷达的角度

    EnemyData omni_enemy_data;  // 全向感知数据

    uint16_t CRC16 = 0;
} __attribute__((packed));

// 电控->视觉
struct CarData
{
    uint8_t header = 0xA5;
    uint8_t mode;
    uint8_t CRC8;

    uint8_t game_start;  // 比赛开始标志位
    uint8_t enemy_color; // 敌方颜色

    uint8_t if_find_enemy; // 左右头标志位相加  取值 0-2

    uint8_t armor3_size = 0; // 0:小装甲板 1:大装甲板
    uint8_t armor4_size = 0;
    uint8_t armor5_size = 0; // 国赛才有5号步兵

    // 大yaw轴
    int16_t big_yaw = 0;
    uint16_t remain_bullet = 400;  // 开局

    // 己方血量
    uint16_t hero_HP = 200;
    uint16_t engineer_HP = 250;
    uint16_t infantry3_HP = 150;
    uint16_t infantry4_HP = 150;
    uint16_t infantry5_HP = 150;
    uint16_t sentry_HP = 400; // 前哨站爆掉之后+600
    uint16_t outpost_HP = 1500;
    uint16_t base_HP = 5000;

    // 敌方血量
    uint16_t enemy_hero_HP = 200;
    uint16_t enemy_engineer_HP = 250;
    uint16_t enemy_infantry3_HP = 150;
    uint16_t enemy_infantry4_HP = 150;
    uint16_t enemy_infantry5_HP = 150;
    uint16_t enmey_sentry_HP = 400;
    uint16_t enemy_outpost_HP = 1500;
    uint16_t enemy_base_HP = 5000;

    // 从自瞄获取敌方位置
    EnemyData enemys_data; // 1英雄 2工程 3-5步兵 6哨兵 7前哨 8基地

    CoordInfo allies_coord[8]; // 己方坐标，从裁判系统获取
    CoordInfo enemys_coord[8]; // 敌方坐标，看雷达准不准

    uint16_t CRC16;
} __attribute__((packed));

class SerialPort
{
public:
    explicit SerialPort();
    ~SerialPort();
    bool init();
    bool recieve(CarData &cardata);
    bool recieve2(CarData &cardata);
    void send(VisionData &vd);
    void close_port();
    int result;
    uint8_t mode;
    bool check_data(VisionData &vd);
    bool check_data(const CarData &cardata);

private:
    void set_brate();
    int set_bit();
    int fd;
    int boudrate, databits, stopbits, parity;
    unsigned char rdata[4096];
    unsigned char Tdata[30];
};

#endif
