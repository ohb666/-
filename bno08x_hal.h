#ifndef __BNO08X_HAL_H_
#define __BNO08X_HAL_H_

#include "stm32f1xx_hal.h"  // STM32 HAL��ͷ�ļ�����������MCU�ͺŵ�������stm32f1xx_hal.h��

/** @defgroup BNO080_Constants BNO080��������
  * @{
  */

/** Ĭ��I2C��ַ��BNO080֧��0x4A��0x4B����ADR���ž��� */
#define BNO080_DEFAULT_ADDRESS 0x4B

/** I2C��������󳤶ȣ����Ƶ��δ����ֽ��� */
#define I2C_BUFFER_LENGTH 32

/** ������ݰ���С��SHTPЭ�鶨������ֵ */
#define MAX_PACKET_SIZE 128

/** FRSԪ������󳤶� */
#define MAX_METADATA_SIZE 9

/** SHTPͨ������ */
#define CHANNEL_COMMAND         0  // ����ͨ��
#define CHANNEL_EXECUTABLE      1  // ��ִ��ͨ��
#define CHANNEL_CONTROL         2  // ����ͨ��
#define CHANNEL_REPORTS         3  // ����ͨ��
#define CHANNEL_WAKE_REPORTS    4  // ���ѱ���ͨ��
#define CHANNEL_GYRO            5  // ������ͨ��

/** SHTP����ID */
#define SHTP_REPORT_COMMAND_RESPONSE      0xF1  // ������Ӧ
#define SHTP_REPORT_COMMAND_REQUEST       0xF2  // ��������
#define SHTP_REPORT_FRS_READ_RESPONSE     0xF3  // FRS��ȡ��Ӧ
#define SHTP_REPORT_FRS_READ_REQUEST      0xF4  // FRS��ȡ����
#define SHTP_REPORT_PRODUCT_ID_RESPONSE   0xF8  // ��ƷID��Ӧ
#define SHTP_REPORT_PRODUCT_ID_REQUEST    0xF9  // ��ƷID����
#define SHTP_REPORT_BASE_TIMESTAMP        0xFB  // ����ʱ���
#define SHTP_REPORT_SET_FEATURE_COMMAND   0xFD  // ������������

/** ����������ID */
#define SENSOR_REPORTID_ACCELEROMETER            0x01  // ���ٶȼ�
#define SENSOR_REPORTID_GYROSCOPE                0x02  // ������
#define SENSOR_REPORTID_MAGNETIC_FIELD           0x03  // ������
#define SENSOR_REPORTID_LINEAR_ACCELERATION      0x04  // ���Լ��ٶ�
#define SENSOR_REPORTID_ROTATION_VECTOR          0x05  // ��ת����
#define SENSOR_REPORTID_GRAVITY                  0x06  // ����
#define SENSOR_REPORTID_GAME_ROTATION_VECTOR     0x08  // ��Ϸ��ת����
#define SENSOR_REPORTID_GEOMAGNETIC_ROTATION_VECTOR 0x09  // �ش���ת����
#define SENSOR_REPORTID_TAP_DETECTOR             0x10  // �û����
#define SENSOR_REPORTID_STEP_COUNTER             0x11  // ����������
#define SENSOR_REPORTID_STABILITY_CLASSIFIER     0x13  // �ȶ��Է�����
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E  // ���˻������

/** FRS��¼ID */
#define FRS_RECORDID_ACCELEROMETER          0xE302  // ���ٶȼƼ�¼
#define FRS_RECORDID_GYROSCOPE_CALIBRATED   0xE306  // У׼�����Ǽ�¼
#define FRS_RECORDID_MAGNETIC_FIELD_CALIBRATED 0xE309  // У׼�����Ƽ�¼
#define FRS_RECORDID_ROTATION_VECTOR        0xE30B  // ��ת������¼

/** ����ID */
#define COMMAND_ERRORS         1   // ��������
#define COMMAND_COUNTER        2   // ����������
#define COMMAND_TARE           3   // У׼����
#define COMMAND_INITIALIZE     4   // ��ʼ������
#define COMMAND_DCD            6   // �����������������
#define COMMAND_ME_CALIBRATE   7   // MEУ׼����
#define COMMAND_DCD_PERIOD_SAVE 9  // �����Ա���DCD����
#define COMMAND_OSCILLATOR     10  // ��������
#define COMMAND_CLEAR_DCD      11  // ���DCD����

/** У׼ѡ�� */
#define CALIBRATE_ACCEL          0  // У׼���ٶȼ�
#define CALIBRATE_GYRO           1  // У׼������
#define CALIBRATE_MAG            2  // У׼������
#define CALIBRATE_PLANAR_ACCEL   3  // У׼ƽ����ٶȼ�
#define CALIBRATE_ACCEL_GYRO_MAG 4  // У׼���ٶȼơ������Ǻʹ�����
#define CALIBRATE_STOP           5  // ֹͣУ׼

/**
  * @}
  */

/** @defgroup BNO080_Functions BNO080��������
  * @{
  */

/** ��ʼ��BNO080������
  * @param hi2c I2C���ָ��
  * @param address �豸I2C��ַ��7λ��ַ��
  */
void BNO080_Init(I2C_HandleTypeDef *hi2c, uint8_t address);

/** ִ�������λ */
void softReset(void);

/** ��ȡ��λԭ��
  * @return ��λԭ�����
  */
uint8_t resetReason(void);

/** ���̶�����ת��Ϊ������
  * @param fixedPointValue �̶�����ֵ
  * @param qPoint Q��λ��
  * @return ת����ĸ�����ֵ
  */
float qToFloat(int16_t fixedPointValue, uint8_t qPoint);

/** ����Ƿ��������ݿ���
  * @return 1��ʾ�����ݣ�0��ʾ������
  */
uint8_t dataAvailable(void);

/** �������뱨������ */
void parseInputReport(void);

/** ��ȡ��Ԫ��I���� */
float getQuatI(void);

/** ��ȡ��Ԫ��J���� */
float getQuatJ(void);

/** ��ȡ��Ԫ��K���� */
float getQuatK(void);

/** ��ȡ��Ԫ��ʵ�� */
float getQuatReal(void);

/** ��ȡ��Ԫ�����Ⱦ��� */
float getQuatRadianAccuracy(void);

/** ��ȡ��Ԫ�����ȼ��� */
uint8_t getQuatAccuracy(void);

/** ��ȡX����ٶ� */
float getAccelX(void);

/** ��ȡY����ٶ� */
float getAccelY(void);

/** ��ȡZ����ٶ� */
float getAccelZ(void);

/** ��ȡ���ٶȾ��ȼ��� */
uint8_t getAccelAccuracy(void);

/** ��ȡX�����Լ��ٶ� */
float getLinAccelX(void);

/** ��ȡY�����Լ��ٶ� */
float getLinAccelY(void);

/** ��ȡZ�����Լ��ٶ� */
float getLinAccelZ(void);

/** ��ȡ���Լ��ٶȾ��ȼ��� */
uint8_t getLinAccelAccuracy(void);

/** ��ȡX������������ */
float getGyroX(void);

/** ��ȡY������������ */
float getGyroY(void);

/** ��ȡZ������������ */
float getGyroZ(void);

/** ��ȡ�����Ǿ��ȼ��� */
uint8_t getGyroAccuracy(void);

/** ��ȡX����������� */
float getMagX(void);

/** ��ȡY����������� */
float getMagY(void);

/** ��ȡZ����������� */
float getMagZ(void);

/** ��ȡ�����ƾ��ȼ��� */
uint8_t getMagAccuracy(void);

/** У׼���ٶȼ� */
void calibrateAccelerometer(void);

/** У׼������ */
void calibrateGyro(void);

/** У׼������ */
void calibrateMagnetometer(void);

/** У׼ƽ����ٶȼ� */
void calibratePlanarAccelerometer(void);

/** У׼���д����� */
void calibrateAll(void);

/** ����У׼���� */
void endCalibration(void);

/** ����У׼���� */
void saveCalibration(void);

/** ��ȡ��������
  * @return ��ǰ����
  */
uint16_t getStepCount(void);

/** ��ȡ�ȶ��Է���
  * @return �ȶ���״̬
  */
uint8_t getStabilityClassifier(void);

/** ��ȡ�����
  * @return �����
  */
uint8_t getActivityClassifier(void);

/** ���ô�������������
  * @param reportID ����ID
  * @param timeBetweenReports ������ʱ�䣨���룩
  * @param specificConfig �ض����ò���
  */
void setFeatureCommand(uint8_t reportID, uint32_t timeBetweenReports, uint32_t specificConfig);

/** ��������
  * @param command ����ID
  */
void sendCommand(uint8_t command);

/** ����У׼����
  * @param thingToCalibrate ҪУ׼�Ķ���
  */
void sendCalibrateCommand(uint8_t thingToCalibrate);

/** ��ȡFRS��¼��Q1ֵ
  * @param recordID FRS��¼ID
  * @return Q1ֵ
  */
int16_t getQ1(uint16_t recordID);

/** ��ȡFRS��¼��Q2ֵ
  * @param recordID FRS��¼ID
  * @return Q2ֵ
  */
int16_t getQ2(uint16_t recordID);

/** ��ȡFRS��¼��Q3ֵ
  * @param recordID FRS��¼ID
  * @return Q3ֵ
  */
int16_t getQ3(uint16_t recordID);

/** ��ȡ�������ֱ���
  * @param recordID FRS��¼ID
  * @return �ֱ��ʣ���������
  */
float getResolution(uint16_t recordID);

/** ��ȡ����������
  * @param recordID FRS��¼ID
  * @return ���̣���������
  */
float getRange(uint16_t recordID);

/** ��ȡFRS������
  * @param recordID FRS��¼ID
  * @param wordNumber �ֱ��
  * @return ��ȡ��32λ����
  */
uint32_t readFRSword(uint16_t recordID, uint8_t wordNumber);

/** �����ȡFRS����
  * @param recordID FRS��¼ID
  * @param readOffset ��ȡƫ����
  * @param blockSize ���ݿ��С
  */
void frsReadRequest(uint16_t recordID, uint16_t readOffset, uint16_t blockSize);

/** ��ȡFRS����
  * @param recordID FRS��¼ID
  * @param startLocation ��ʼλ��
  * @param wordsToRead Ҫ��ȡ������
  * @return 1��ʾ�ɹ���0��ʾʧ��
  */
uint8_t readFRSdata(uint16_t recordID, uint8_t startLocation, uint8_t wordsToRead);

/** ������ת��������
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableRotationVector(uint32_t timeBetweenReports);

/** ������Ϸ��ת��������
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableGameRotationVector(uint32_t timeBetweenReports);

/** ���ü��ٶȼƱ���
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableAccelerometer(uint32_t timeBetweenReports);

/** �������Լ��ٶȼƱ���
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableLinearAccelerometer(uint32_t timeBetweenReports);

/** ���������Ǳ���
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableGyro(uint32_t timeBetweenReports);

/** ���ô����Ʊ���
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableMagnetometer(uint32_t timeBetweenReports);

/** ���ò�������������
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableStepCounter(uint32_t timeBetweenReports);

/** �����ȶ��Է���������
  * @param timeBetweenReports ������ʱ�䣨���룩
  */
void enableStabilityClassifier(uint32_t timeBetweenReports);

/** ��Ԫ��תŷ����
  * @param ����
  */
void QuaternionToEulerAngles(float quatI, float quatJ, float quatK, float quatReal, 
                            float *roll, float *pitch, float *yaw);

/**
  * @}
  */

#endif /* __BNO08X_HAL_H_ */
