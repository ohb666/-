#include "atgm336h.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

float latitude,longitude;
float gps_data[2];
char frame_buffer[128];

// 初始化全局GPS数据帧结构体
GpsFrame receDataFrame;

/**
 * atgm336h_parse - 解析NMEA数据帧
 * @nmea_sentence: 输入的NMEA数据字符串
 *
 * 该函数解析从GPS模块接收到的NMEA数据字符串，
 * 提取UTC时间、数据有效标识、纬度、经度等信息，
 * 并存储在全局结构体receDataFrame中。
 */
void atgm336h_parse(const char *nmea_sentence)
{
    // 检查输入的NMEA数据字符串是否合法
    if (nmea_sentence == NULL || nmea_sentence[0] != '$')
    {
      //  u5_printf("无效的NMEA语句!\n");
        return;
    }

    char *point = NULL;
    char *nextPoint = NULL;
    int tempVar = 0;

    // 解析NMEA数据字符串中的逗号分隔字段
    for (tempVar = 0; tempVar < 7; tempVar++)
    {
        if (tempVar == 0)
        {
            // 查找第一个逗号
            point = strstr(nmea_sentence, ",");
            if (!point)
            {
               // u5_printf("解析错误!\n");
                return;
            }
        }
        else
        {
            // 查找下一个逗号并提取字段
            point++;
            nextPoint = strstr(point, ",");
            if (nextPoint)
            {
                switch (tempVar)
                {
                case 1: // 提取UTC时间
                    memcpy(receDataFrame.UTCTime, point, nextPoint - point);
                    receDataFrame.UTCTime[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                case 2: // 提取数据有效标识
                    memcpy(receDataFrame.UsefullFlag, point, nextPoint - point);
                    receDataFrame.UsefullFlag[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                case 3: // 提取纬度信息
                    memcpy(receDataFrame.latitude, point, nextPoint - point);
                    receDataFrame.latitude[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                case 4: // 提取纬度方向
                    memcpy(receDataFrame.N_S, point, nextPoint - point);
                    receDataFrame.N_S[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                case 5: // 提取经度信息
                    memcpy(receDataFrame.longitude, point, nextPoint - point);
                    receDataFrame.longitude[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                case 6: // 提取经度方向
                    memcpy(receDataFrame.E_W, point, nextPoint - point);
                    receDataFrame.E_W[nextPoint - point] = '\0'; // 确保字符串以null结尾
                    break;
                }
                // 更新指针位置以继续查找下一个字段
                point = nextPoint;
                receDataFrame.isParseData = 1; // 设置解析完成标志
            }
            else
            {
              //  u5_printf("解析错误!\n");
                return;
            }
        }
    }

    // 根据数据有效标识输出相应的提示信息
    if (receDataFrame.UsefullFlag[0] == 'A')
    {
       // u5_printf("数据有效!\n");
    }
    else if (receDataFrame.UsefullFlag[0] == 'V')
    {
       // u5_printf("数据无效!\n");
    }
}

/**
 * atgm336h_transform - 转换解析后的经纬度信息
 *
 * 该函数将解析出的经纬度信息从字符串格式转换为浮点数格式，
 */
void atgm336h_transform(void)
{
    // 检查是否已经完成数据解析
    if (!receDataFrame.isParseData)
        return;

    int temp1 = 0;
    int temp2 = 0;

    // 将字符串格式的纬度信息转换为浮点数
    latitude = strtod(receDataFrame.latitude, NULL);
    // 将字符串格式的经度信息转换为浮点数
    longitude = strtod(receDataFrame.longitude, NULL);

    // 处理纬度信息（度分格式转换为十进制格式）
    if (latitude >= 1000.0f)
    {
        temp1 = (int)(latitude / 100);
        latitude = (float)temp1 + (latitude - temp1 * 100) / 60.0f;
    }

    // 处理经度信息（度分格式转换为十进制格式）
    if (longitude >= 1000.0f)
    {
        temp2 = (int)(longitude / 100);
        longitude = (float)temp2 + (longitude - temp2 * 100) / 60.0f;
    }

    // 根据方向调整正负
    if (receDataFrame.N_S[0] == 'S') latitude = -latitude;
    if (receDataFrame.E_W[0] == 'W') longitude = -longitude;
}

void parse_nmea_sentences(const char *nmea_data)
{
    const char *start = nmea_data;
    const char *end;

    // 初始化解析结构体
    memset(&receDataFrame, 0, sizeof(receDataFrame));

    while ((start = strstr(start, "$")) != NULL)
    {
        // 找到下一行的换行符
        end = strstr(start, "\n");

        if (end != NULL)
        {
            size_t length = end - start + 1;

            // 如果长度适合缓冲区，则拷贝
            if (length < sizeof(frame_buffer))
            {
                memcpy(frame_buffer, start, length);
                frame_buffer[length] = '\0'; // 确保字符串以null结尾

                // 检查帧类型并解析
                if ((memcmp(frame_buffer + 1, "GNRMC", 5) == 0) || (memcmp(frame_buffer + 1, "GPRMC", 5) == 0))
                {
                   // u5_printf("正在处理帧: %s", frame_buffer);
                    atgm336h_parse(frame_buffer);

                    if (receDataFrame.isParseData)
                    {
                        atgm336h_transform();
                        gps_data[0] = latitude;
                        gps_data[1] = longitude;
                      //  u5_printf("经纬度解析成功\r\n");
                    }
                    else
                    {
                       // u5_printf("GPS数据解析失败\r\n");
                    }
                }
            }

            // 移动指针到下一行
            start = end + 1;
        }
        else
        {
            break;
        }
    }
}
