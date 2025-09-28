#ifndef ATGM336H_H
#define ATGM336H_H

#include <stdint.h>

// 定义解析GPS数据帧的结构体
typedef struct
{
    char UTCTime[16];    // UTC时间
    char UsefullFlag[2]; // 数据有效标识 ('A' 表示有效, 'V' 表示无效)
    char latitude[16];   // 纬度信息
    char N_S[2];         // 纬度方向 ('N' 或 'S')
    char longitude[16];  // 经度信息
    char E_W[2];         // 经度方向 ('E' 或 'W')
    int isParseData;     // 数据解析标志位 (1 表示解析完成)
} GpsFrame;

extern GpsFrame receDataFrame; // 定义全局GPS数据帧

// 函数声明
void atgm336h_parse(const char *nmea_sentence); // 解析NMEA数据帧
void atgm336h_transform(void); // 转换解析后的经纬度信息
void parse_nmea_sentences(const char *nmea_data); // 解析NMEA数据流
#endif // ATGM336H_H
