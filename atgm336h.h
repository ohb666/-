#ifndef ATGM336H_H
#define ATGM336H_H

#include <stdint.h>

// �������GPS����֡�Ľṹ��
typedef struct
{
    char UTCTime[16];    // UTCʱ��
    char UsefullFlag[2]; // ������Ч��ʶ ('A' ��ʾ��Ч, 'V' ��ʾ��Ч)
    char latitude[16];   // γ����Ϣ
    char N_S[2];         // γ�ȷ��� ('N' �� 'S')
    char longitude[16];  // ������Ϣ
    char E_W[2];         // ���ȷ��� ('E' �� 'W')
    int isParseData;     // ���ݽ�����־λ (1 ��ʾ�������)
} GpsFrame;

extern GpsFrame receDataFrame; // ����ȫ��GPS����֡

// ��������
void atgm336h_parse(const char *nmea_sentence); // ����NMEA����֡
void atgm336h_transform(void); // ת��������ľ�γ����Ϣ
void parse_nmea_sentences(const char *nmea_data); // ����NMEA������
#endif // ATGM336H_H
