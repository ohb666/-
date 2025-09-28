#include "atgm336h.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "usart.h"

float latitude,longitude;
float gps_data[2];
char frame_buffer[128];

// ��ʼ��ȫ��GPS����֡�ṹ��
GpsFrame receDataFrame;

/**
 * atgm336h_parse - ����NMEA����֡
 * @nmea_sentence: �����NMEA�����ַ���
 *
 * �ú���������GPSģ����յ���NMEA�����ַ�����
 * ��ȡUTCʱ�䡢������Ч��ʶ��γ�ȡ����ȵ���Ϣ��
 * ���洢��ȫ�ֽṹ��receDataFrame�С�
 */
void atgm336h_parse(const char *nmea_sentence)
{
    // ��������NMEA�����ַ����Ƿ�Ϸ�
    if (nmea_sentence == NULL || nmea_sentence[0] != '$')
    {
      //  u5_printf("��Ч��NMEA���!\n");
        return;
    }

    char *point = NULL;
    char *nextPoint = NULL;
    int tempVar = 0;

    // ����NMEA�����ַ����еĶ��ŷָ��ֶ�
    for (tempVar = 0; tempVar < 7; tempVar++)
    {
        if (tempVar == 0)
        {
            // ���ҵ�һ������
            point = strstr(nmea_sentence, ",");
            if (!point)
            {
               // u5_printf("��������!\n");
                return;
            }
        }
        else
        {
            // ������һ�����Ų���ȡ�ֶ�
            point++;
            nextPoint = strstr(point, ",");
            if (nextPoint)
            {
                switch (tempVar)
                {
                case 1: // ��ȡUTCʱ��
                    memcpy(receDataFrame.UTCTime, point, nextPoint - point);
                    receDataFrame.UTCTime[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                case 2: // ��ȡ������Ч��ʶ
                    memcpy(receDataFrame.UsefullFlag, point, nextPoint - point);
                    receDataFrame.UsefullFlag[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                case 3: // ��ȡγ����Ϣ
                    memcpy(receDataFrame.latitude, point, nextPoint - point);
                    receDataFrame.latitude[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                case 4: // ��ȡγ�ȷ���
                    memcpy(receDataFrame.N_S, point, nextPoint - point);
                    receDataFrame.N_S[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                case 5: // ��ȡ������Ϣ
                    memcpy(receDataFrame.longitude, point, nextPoint - point);
                    receDataFrame.longitude[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                case 6: // ��ȡ���ȷ���
                    memcpy(receDataFrame.E_W, point, nextPoint - point);
                    receDataFrame.E_W[nextPoint - point] = '\0'; // ȷ���ַ�����null��β
                    break;
                }
                // ����ָ��λ���Լ���������һ���ֶ�
                point = nextPoint;
                receDataFrame.isParseData = 1; // ���ý�����ɱ�־
            }
            else
            {
              //  u5_printf("��������!\n");
                return;
            }
        }
    }

    // ����������Ч��ʶ�����Ӧ����ʾ��Ϣ
    if (receDataFrame.UsefullFlag[0] == 'A')
    {
       // u5_printf("������Ч!\n");
    }
    else if (receDataFrame.UsefullFlag[0] == 'V')
    {
       // u5_printf("������Ч!\n");
    }
}

/**
 * atgm336h_transform - ת��������ľ�γ����Ϣ
 *
 * �ú������������ľ�γ����Ϣ���ַ�����ʽת��Ϊ��������ʽ��
 */
void atgm336h_transform(void)
{
    // ����Ƿ��Ѿ�������ݽ���
    if (!receDataFrame.isParseData)
        return;

    int temp1 = 0;
    int temp2 = 0;

    // ���ַ�����ʽ��γ����Ϣת��Ϊ������
    latitude = strtod(receDataFrame.latitude, NULL);
    // ���ַ�����ʽ�ľ�����Ϣת��Ϊ������
    longitude = strtod(receDataFrame.longitude, NULL);

    // ����γ����Ϣ���ȷָ�ʽת��Ϊʮ���Ƹ�ʽ��
    if (latitude >= 1000.0f)
    {
        temp1 = (int)(latitude / 100);
        latitude = (float)temp1 + (latitude - temp1 * 100) / 60.0f;
    }

    // ��������Ϣ���ȷָ�ʽת��Ϊʮ���Ƹ�ʽ��
    if (longitude >= 1000.0f)
    {
        temp2 = (int)(longitude / 100);
        longitude = (float)temp2 + (longitude - temp2 * 100) / 60.0f;
    }

    // ���ݷ����������
    if (receDataFrame.N_S[0] == 'S') latitude = -latitude;
    if (receDataFrame.E_W[0] == 'W') longitude = -longitude;
}

void parse_nmea_sentences(const char *nmea_data)
{
    const char *start = nmea_data;
    const char *end;

    // ��ʼ�������ṹ��
    memset(&receDataFrame, 0, sizeof(receDataFrame));

    while ((start = strstr(start, "$")) != NULL)
    {
        // �ҵ���һ�еĻ��з�
        end = strstr(start, "\n");

        if (end != NULL)
        {
            size_t length = end - start + 1;

            // ��������ʺϻ��������򿽱�
            if (length < sizeof(frame_buffer))
            {
                memcpy(frame_buffer, start, length);
                frame_buffer[length] = '\0'; // ȷ���ַ�����null��β

                // ���֡���Ͳ�����
                if ((memcmp(frame_buffer + 1, "GNRMC", 5) == 0) || (memcmp(frame_buffer + 1, "GPRMC", 5) == 0))
                {
                   // u5_printf("���ڴ���֡: %s", frame_buffer);
                    atgm336h_parse(frame_buffer);

                    if (receDataFrame.isParseData)
                    {
                        atgm336h_transform();
                        gps_data[0] = latitude;
                        gps_data[1] = longitude;
                      //  u5_printf("��γ�Ƚ����ɹ�\r\n");
                    }
                    else
                    {
                       // u5_printf("GPS���ݽ���ʧ��\r\n");
                    }
                }
            }

            // �ƶ�ָ�뵽��һ��
            start = end + 1;
        }
        else
        {
            break;
        }
    }
}
