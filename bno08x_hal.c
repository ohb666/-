#include "bno08x_hal.h"
#include <math.h>
#include <string.h>

/** @defgroup BNO080_Variables BNO080ȫ�ֱ���
 * @{
 */

// ��̬����ǰ������
static uint8_t receivePacket(void);
static uint8_t sendPacket(uint8_t channelNumber, uint8_t dataLength);

/** I2C���������HAL��ͨ�� */
static I2C_HandleTypeDef *hi2c_bno080;

/** �豸I2C��ַ��7λ��ַ�� */
static uint8_t _deviceAddress;

/** SHTPЭ��ͷ����������4�ֽ� */
static uint8_t shtpHeader[4];

/** SHTPЭ�����ݻ����� */
static uint8_t shtpData[MAX_PACKET_SIZE];

/** ��ͨ�������кţ�����SHTPͨ�� */
static uint8_t sequenceNumber[6] = {0, 0, 0, 0, 0, 0};

/** �������к� */
static uint8_t commandSequenceNumber = 0;

/** FRSԪ���ݻ����� */
static uint32_t metaData[MAX_METADATA_SIZE];

/** ԭʼ���������� */
static uint16_t rawAccelX, rawAccelY, rawAccelZ;            // ���ٶȼ�ԭʼ����
static uint8_t accelAccuracy;                               // ���ٶȼƾ���
static uint16_t rawLinAccelX, rawLinAccelY, rawLinAccelZ;   // ���Լ��ٶ�ԭʼ����
static uint8_t accelLinAccuracy;                            // ���Լ��ٶȾ���
static uint16_t rawGyroX, rawGyroY, rawGyroZ;               // ������ԭʼ����
static uint8_t gyroAccuracy;                                // �����Ǿ���
static uint16_t rawMagX, rawMagY, rawMagZ;                  // ������ԭʼ����
static uint8_t magAccuracy;                                 // �����ƾ���
static uint16_t rawQuatI, rawQuatJ, rawQuatK, rawQuatReal;  // ��Ԫ��ԭʼ����
static uint16_t rawQuatRadianAccuracy;                      // ��Ԫ�����Ⱦ���
static uint8_t quatAccuracy;                                // ��Ԫ������
static uint16_t stepCount;                                  // ��������
static uint8_t stabilityClassifier;                         // �ȶ��Է���
static uint8_t activityClassifier;                          // �����
static uint8_t activityConfidences[10];                     // ����Ŷ�����
static uint8_t *_activityConfidences = activityConfidences; // ָ�����Ŷȵ�ָ��

/** Qֵ�����ڹ̶�������������ת�� */
static int rotationVector_Q1 = 14;      // ��ת������Qֵ
static int accelerometer_Q1 = 8;        // ���ٶȼƵ�Qֵ
static int linear_accelerometer_Q1 = 8; // ���Լ��ٶȼƵ�Qֵ
static int gyro_Q1 = 9;                 // �����ǵ�Qֵ
static int magnetometer_Q1 = 4;         // �����Ƶ�Qֵ

/**
 * @}
 */

/** @defgroup BNO080_Functions BNO080����ʵ��
 * @{
 */

/**
 * @brief ��ʼ��BNO080������
 * @param hi2c I2C���ָ�룬��STM32 HAL���ṩ
 * @param address �豸I2C��ַ��7λ��ַ������0x4B��
 */
void BNO080_Init(I2C_HandleTypeDef *hi2c, uint8_t address)
{
    hi2c_bno080 = hi2c;       // ����I2C���
    _deviceAddress = address; // �����豸��ַ
}

/**
 * @brief ִ�������λ
 *        ���͸�λ�����ս��ջ�����
 */
void softReset(void)
{
    shtpData[0] = 1;                   // ���ø�λ����
    sendPacket(CHANNEL_EXECUTABLE, 1); // ���͵���ִ��ͨ��
    HAL_Delay(50);                     // �ȴ�50ms��ȷ����λ���
    while (receivePacket() == 1)
        ;          // ��ս��ջ�����
    HAL_Delay(50); // �ٴεȴ�50ms
    while (receivePacket() == 1)
        ; // �ٴ���ջ�����
}

/**
 * @brief ��ȡ��λԭ��
 * @return ��λԭ����룬0��ʾʧ�ܻ�����Ӧ
 */
uint8_t resetReason(void)
{
    shtpData[0] = SHTP_REPORT_PRODUCT_ID_REQUEST; // ���ò�ƷID����
    shtpData[1] = 0;                              // �������
    sendPacket(CHANNEL_CONTROL, 2);               // �������󵽿���ͨ��
    if (receivePacket() == 1)
    { // ������յ�����
        if (shtpData[0] == SHTP_REPORT_PRODUCT_ID_RESPONSE)
        {                       // �����Ӧ����
            return shtpData[1]; // ���ظ�λԭ��
        }
    }
    return 0; // δ�յ���Ч��Ӧ������0
}

/**
 * @brief ���̶�����ת��Ϊ������
 * @param fixedPointValue �̶�����ֵ
 * @param qPoint Q��λ�ã�С������λ����
 * @return ת����ĸ�����ֵ
 */
float qToFloat(int16_t fixedPointValue, uint8_t qPoint)
{
    float qFloat = (float)fixedPointValue; // ת��Ϊ������
    qFloat *= powf(2.0f, -qPoint);         // ����Q��λ������
    return qFloat;                         // ���ؽ��
}

/**
 * @brief ����Ƿ��������ݿ���
 * @return 1��ʾ�������ݣ�0��ʾ������
 */
uint8_t dataAvailable(void)
{
    if (receivePacket() == 1)
    { // ���Խ������ݰ�
        if (shtpHeader[2] == CHANNEL_REPORTS && shtpData[0] == SHTP_REPORT_BASE_TIMESTAMP)
        {                       // ����Ƿ�Ϊ����ͨ���ͻ���ʱ���
            parseInputReport(); // �������յ��ı���
            return 1;           // �����ݿ���
        }
    }
    return 0; // ������
}

/**
 * @brief �������뱨������
 *        ���ݱ���ID��ȡ���洢����������
 */
void parseInputReport(void)
{
    int dataLength = ((uint16_t)shtpHeader[1] << 8 | shtpHeader[0]) & ~(1 << 15); // �������ݳ��ȣ�ȥ������λ��
    dataLength -= 4;                                                              // ��ȥͷ������

    uint8_t status = shtpData[5 + 2] & 0x03;                                                           // ��ȡ״̬/���ȣ���2λ��
    uint16_t data1 = (uint16_t)shtpData[5 + 5] << 8 | shtpData[5 + 4];                                 // ��ȡ����1
    uint16_t data2 = (uint16_t)shtpData[5 + 7] << 8 | shtpData[5 + 6];                                 // ��ȡ����2
    uint16_t data3 = (uint16_t)shtpData[5 + 9] << 8 | shtpData[5 + 8];                                 // ��ȡ����3
    uint16_t data4 = (dataLength - 5 > 9) ? ((uint16_t)shtpData[5 + 11] << 8 | shtpData[5 + 10]) : 0;  // ��ȡ����4�������ڣ�
    uint16_t data5 = (dataLength - 5 > 11) ? ((uint16_t)shtpData[5 + 13] << 8 | shtpData[5 + 12]) : 0; // ��ȡ����5�������ڣ�

    switch (shtpData[5])
    {                                   // ���ݱ���ID��������
    case SENSOR_REPORTID_ACCELEROMETER: // ���ٶȼƱ���
        accelAccuracy = status;
        rawAccelX = data1;
        rawAccelY = data2;
        rawAccelZ = data3;
        break;
    case SENSOR_REPORTID_LINEAR_ACCELERATION: // ���Լ��ٶȱ���
        accelLinAccuracy = status;
        rawLinAccelX = data1;
        rawLinAccelY = data2;
        rawLinAccelZ = data3;
        break;
    case SENSOR_REPORTID_GYROSCOPE: // �����Ǳ���
        gyroAccuracy = status;
        rawGyroX = data1;
        rawGyroY = data2;
        rawGyroZ = data3;
        break;
    case SENSOR_REPORTID_MAGNETIC_FIELD: // �����Ʊ���
        magAccuracy = status;
        rawMagX = data1;
        rawMagY = data2;
        rawMagZ = data3;
        break;
    case SENSOR_REPORTID_ROTATION_VECTOR:      // ��ת��������
    case SENSOR_REPORTID_GAME_ROTATION_VECTOR: // ��Ϸ��ת��������
        quatAccuracy = status;
        rawQuatI = data1;
        rawQuatJ = data2;
        rawQuatK = data3;
        rawQuatReal = data4;
        rawQuatRadianAccuracy = data5;
        break;
    case SENSOR_REPORTID_STEP_COUNTER: // ��������������
        stepCount = data3;
        break;
    case SENSOR_REPORTID_STABILITY_CLASSIFIER: // �ȶ��Է���������
        stabilityClassifier = shtpData[5 + 4];
        break;
    case SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER: // ���˻����������
        activityClassifier = shtpData[5 + 5];
        for (uint8_t x = 0; x < 9; x++)
        {
            _activityConfidences[x] = shtpData[5 + 6 + x]; // �洢���Ŷ�
        }
        break;
    default:
        break; // δʶ��ı�������
    }
}

/**
 * @brief ��ȡ��Ԫ��I����
 * @return ��������ʾ��I����
 */
float getQuatI(void)
{
    return qToFloat(rawQuatI, rotationVector_Q1);
}

/**
 * @brief ��ȡ��Ԫ��J����
 * @return ��������ʾ��J����
 */
float getQuatJ(void)
{
    return qToFloat(rawQuatJ, rotationVector_Q1);
}

/**
 * @brief ��ȡ��Ԫ��K����
 * @return ��������ʾ��K����
 */
float getQuatK(void)
{
    return qToFloat(rawQuatK, rotationVector_Q1);
}

/**
 * @brief ��ȡ��Ԫ��ʵ��
 * @return ��������ʾ��ʵ��
 */
float getQuatReal(void)
{
    return qToFloat(rawQuatReal, rotationVector_Q1);
}

/**
 * @brief ��ȡ��Ԫ�����Ⱦ���
 * @return ��������ʾ�Ļ��Ⱦ���
 */
float getQuatRadianAccuracy(void)
{
    return qToFloat(rawQuatRadianAccuracy, rotationVector_Q1);
}

/**
 * @brief ��ȡ��Ԫ�����ȼ���
 * @return ���ȼ���0-3��
 */
uint8_t getQuatAccuracy(void)
{
    return quatAccuracy;
}

/**
 * @brief ��ȡX����ٶ�
 * @return ��������ʾ��X����ٶȣ���λ��m/s?��
 */
float getAccelX(void)
{
    return qToFloat(rawAccelX, accelerometer_Q1);
}

/**
 * @brief ��ȡY����ٶ�
 * @return ��������ʾ��Y����ٶȣ���λ��m/s?��
 */
float getAccelY(void)
{
    return qToFloat(rawAccelY, accelerometer_Q1);
}

/**
 * @brief ��ȡZ����ٶ�
 * @return ��������ʾ��Z����ٶȣ���λ��m/s?��
 */
float getAccelZ(void)
{
    return qToFloat(rawAccelZ, accelerometer_Q1);
}

/**
 * @brief ��ȡ���ٶȾ��ȼ���
 * @return ���ȼ���0-3��
 */
uint8_t getAccelAccuracy(void)
{
    return accelAccuracy;
}

/**
 * @brief ��ȡX�����Լ��ٶ�
 * @return ��������ʾ��X�����Լ��ٶȣ���λ��m/s?��
 */
float getLinAccelX(void)
{
    return qToFloat(rawLinAccelX, linear_accelerometer_Q1);
}

/**
 * @brief ��ȡY�����Լ��ٶ�
 * @return ��������ʾ��Y�����Լ��ٶȣ���λ��m/s?��
 */
float getLinAccelY(void)
{
    return qToFloat(rawLinAccelY, linear_accelerometer_Q1);
}

/**
 * @brief ��ȡZ�����Լ��ٶ�
 * @return ��������ʾ��Z�����Լ��ٶȣ���λ��m/s?��
 */
float getLinAccelZ(void)
{
    return qToFloat(rawLinAccelZ, linear_accelerometer_Q1);
}

/**
 * @brief ��ȡ���Լ��ٶȾ��ȼ���
 * @return ���ȼ���0-3��
 */
uint8_t getLinAccelAccuracy(void)
{
    return accelLinAccuracy;
}

/**
 * @brief ��ȡX������������
 * @return ��������ʾ��X����ٶȣ���λ��rad/s��
 */
float getGyroX(void)
{
    return qToFloat(rawGyroX, gyro_Q1);
}

/**
 * @brief ��ȡY������������
 * @return ��������ʾ��Y����ٶȣ���λ��rad/s��
 */
float getGyroY(void)
{
    return qToFloat(rawGyroY, gyro_Q1);
}

/**
 * @brief ��ȡZ������������
 * @return ��������ʾ��Z����ٶȣ���λ��rad/s��
 */
float getGyroZ(void)
{
    return qToFloat(rawGyroZ, gyro_Q1);
}

/**
 * @brief ��ȡ�����Ǿ��ȼ���
 * @return ���ȼ���0-3��
 */
uint8_t getGyroAccuracy(void)
{
    return gyroAccuracy;
}

/**
 * @brief ��ȡX�����������
 * @return ��������ʾ��X��ų�ǿ�ȣ���λ����T��
 */
float getMagX(void)
{
    return qToFloat(rawMagX, magnetometer_Q1);
}

/**
 * @brief ��ȡY�����������
 * @return ��������ʾ��Y��ų�ǿ�ȣ���λ����T��
 */
float getMagY(void)
{
    return qToFloat(rawMagY, magnetometer_Q1);
}

/**
 * @brief ��ȡZ�����������
 * @return ��������ʾ��Z��ų�ǿ�ȣ���λ����T��
 */
float getMagZ(void)
{
    return qToFloat(rawMagZ, magnetometer_Q1);
}

/**
 * @brief ��ȡ�����ƾ��ȼ���
 * @return ���ȼ���0-3��
 */
uint8_t getMagAccuracy(void)
{
    return magAccuracy;
}

/**
 * @brief ��ȡ��������
 * @return ��ǰ����
 */
uint16_t getStepCount(void)
{
    return stepCount;
}

/**
 * @brief ��ȡ�ȶ��Է���
 * @return �ȶ���״̬
 */
uint8_t getStabilityClassifier(void)
{
    return stabilityClassifier;
}

/**
 * @brief ��ȡ�����
 * @return ��ǰ�����
 */
uint8_t getActivityClassifier(void)
{
    return activityClassifier;
}

/**
 * @brief У׼���ٶȼ�
 */
void calibrateAccelerometer(void)
{
    sendCalibrateCommand(CALIBRATE_ACCEL);
}

/**
 * @brief У׼������
 */
void calibrateGyro(void)
{
    sendCalibrateCommand(CALIBRATE_GYRO);
}

/**
 * @brief У׼������
 */
void calibrateMagnetometer(void)
{
    sendCalibrateCommand(CALIBRATE_MAG);
}

/**
 * @brief У׼ƽ����ٶȼ�
 */
void calibratePlanarAccelerometer(void)
{
    sendCalibrateCommand(CALIBRATE_PLANAR_ACCEL);
}

/**
 * @brief У׼���д�����
 */
void calibrateAll(void)
{
    sendCalibrateCommand(CALIBRATE_ACCEL_GYRO_MAG);
}

/**
 * @brief ����У׼����
 */
void endCalibration(void)
{
    sendCalibrateCommand(CALIBRATE_STOP);
}

/**
 * @brief ����У׼����
 */
void saveCalibration(void)
{
    for (uint8_t x = 3; x < 12; x++)
    {
        shtpData[x] = 0; // ��������ֶ�
    }
    sendCommand(COMMAND_DCD); // ���ͱ���DCD����
}

/**
 * @brief ���ô�������������
 * @param reportID ����ID����SENSOR_REPORTID_ROTATION_VECTOR��
 * @param timeBetweenReports ������ʱ�䣨���룩
 * @param specificConfig �ض����ò�����ͨ��Ϊ0��
 */
void setFeatureCommand(uint8_t reportID, uint32_t timeBetweenReports, uint32_t specificConfig)
{
    uint32_t microsBetweenReports = timeBetweenReports * 1000; // ת��Ϊ΢��
    shtpData[0] = SHTP_REPORT_SET_FEATURE_COMMAND;             // ������������
    shtpData[1] = reportID;                                    // ����ID
    shtpData[2] = 0;                                           // �����ֽ�
    shtpData[3] = 0;                                           // �����ֽ�
    shtpData[4] = 0;                                           // �����ֽ�
    shtpData[5] = (microsBetweenReports >> 0) & 0xFF;          // ���������ֽ�
    shtpData[6] = (microsBetweenReports >> 8) & 0xFF;          // �������ε��ֽ�
    shtpData[7] = (microsBetweenReports >> 16) & 0xFF;         // �������θ��ֽ�
    shtpData[8] = (microsBetweenReports >> 24) & 0xFF;         // ���������ֽ�
    shtpData[9] = 0;                                           // �����ֽ�
    shtpData[10] = 0;                                          // �����ֽ�
    shtpData[11] = 0;                                          // �����ֽ�
    shtpData[12] = 0;                                          // �����ֽ�
    shtpData[13] = (specificConfig >> 0) & 0xFF;               // ���ò������ֽ�
    shtpData[14] = (specificConfig >> 8) & 0xFF;               // ���ò����ε��ֽ�
    shtpData[15] = (specificConfig >> 16) & 0xFF;              // ���ò����θ��ֽ�
    shtpData[16] = (specificConfig >> 24) & 0xFF;              // ���ò������ֽ�
    sendPacket(CHANNEL_CONTROL, 17);                           // ���͵�����ͨ��������17�ֽ�
}

/**
 * @brief ��������
 * @param command ����ID����COMMAND_DCD��
 */
void sendCommand(uint8_t command)
{
    shtpData[0] = SHTP_REPORT_COMMAND_REQUEST; // ��������
    shtpData[1] = commandSequenceNumber++;     // �������к�
    shtpData[2] = command;                     // ����ID
    for (uint8_t i = 3; i < 12; i++)
    {
        shtpData[i] = 0; // ���ʣ���ֽ�
    }
    sendPacket(CHANNEL_CONTROL, 12); // ���͵�����ͨ��������12�ֽ�
}

/**
 * @brief ����У׼����
 * @param thingToCalibrate ҪУ׼�Ķ�����CALIBRATE_ACCEL��
 */
void sendCalibrateCommand(uint8_t thingToCalibrate)
{
    for (uint8_t x = 3; x < 12; x++)
    {
        shtpData[x] = 0; // ��������ֶ�
    }
    switch (thingToCalibrate)
    { // ����У׼�������ñ�־λ
    case CALIBRATE_ACCEL:
        shtpData[3] = 1; // ���ٶȼ�У׼
        break;
    case CALIBRATE_GYRO:
        shtpData[4] = 1; // ������У׼
        break;
    case CALIBRATE_MAG:
        shtpData[5] = 1; // ������У׼
        break;
    case CALIBRATE_PLANAR_ACCEL:
        shtpData[7] = 1; // ƽ����ٶȼ�У׼
        break;
    case CALIBRATE_ACCEL_GYRO_MAG:
        shtpData[3] = 1; // ���ٶȼ�
        shtpData[4] = 1; // ������
        shtpData[5] = 1; // ������
        break;
    case CALIBRATE_STOP:
        // �������ñ�־������ȫ0��ʾֹͣ
        break;
    default:
        break;
    }
    sendCommand(COMMAND_ME_CALIBRATE); // ����MEУ׼����
}

/**
 * @brief ������ת��������
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableRotationVector(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_ROTATION_VECTOR, timeBetweenReports, 0);
}

/**
 * @brief ������Ϸ��ת��������
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableGameRotationVector(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_GAME_ROTATION_VECTOR, timeBetweenReports, 0);
}

/**
 * @brief ���ü��ٶȼƱ���
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableAccelerometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_ACCELEROMETER, timeBetweenReports, 0);
}

/**
 * @brief �������Լ��ٶȼƱ���
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableLinearAccelerometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_LINEAR_ACCELERATION, timeBetweenReports, 0);
}

/**
 * @brief ���������Ǳ���
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableGyro(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_GYROSCOPE, timeBetweenReports, 0);
}

/**
 * @brief ���ô����Ʊ���
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableMagnetometer(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_MAGNETIC_FIELD, timeBetweenReports, 0);
}

/**
 * @brief ���ò�������������
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableStepCounter(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_STEP_COUNTER, timeBetweenReports, 0);
}

/**
 * @brief �����ȶ��Է���������
 * @param timeBetweenReports ������ʱ�䣨���룩
 */
void enableStabilityClassifier(uint32_t timeBetweenReports)
{
    setFeatureCommand(SENSOR_REPORTID_STABILITY_CLASSIFIER, timeBetweenReports, 0);
}

/**
 * @brief ��ȡFRS��¼��Q1ֵ
 * @param recordID FRS��¼ID����FRS_RECORDID_ACCELEROMETER��
 * @return Q1ֵ����ʧ�ܷ���0
 */
int16_t getQ1(uint16_t recordID)
{
    if (readFRSdata(recordID, 7, 1))
    {                                           // ��ȡ��7���֣�Q1��
        return (int16_t)(metaData[0] & 0xFFFF); // ���ص�16λ��ΪQ1
    }
    return 0; // ��ȡʧ�ܷ���0
}

/**
 * @brief ��ȡFRS��¼��Q2ֵ
 * @param recordID FRS��¼ID
 * @return Q2ֵ����ʧ�ܷ���0
 */
int16_t getQ2(uint16_t recordID)
{
    if (readFRSdata(recordID, 8, 1))
    {                                        // ��ȡ��8���֣�Q2��
        return (int16_t)(metaData[0] >> 16); // ���ظ�16λ��ΪQ2
    }
    return 0;
}

/**
 * @brief ��ȡFRS��¼��Q3ֵ
 * @param recordID FRS��¼ID
 * @return Q3ֵ����ʧ�ܷ���0
 */
int16_t getQ3(uint16_t recordID)
{
    if (readFRSdata(recordID, 9, 1))
    {                                           // ��ȡ��9���֣�Q3��
        return (int16_t)(metaData[0] & 0xFFFF); // ���ص�16λ��ΪQ3
    }
    return 0;
}

/**
 * @brief ��ȡ�������ֱ���
 * @param recordID FRS��¼ID
 * @return �ֱ��ʣ�������������ʧ�ܷ���0
 */
float getResolution(uint16_t recordID)
{
    if (readFRSdata(recordID, 4, 1))
    { // ��ȡ��4���֣��ֱ��ʣ�
        uint32_t res = metaData[0];
        return *(float *)&res; // ��32λ����ת��Ϊ������
    }
    return 0.0f;
}

/**
 * @brief ��ȡ����������
 * @param recordID FRS��¼ID
 * @return ���̣�������������ʧ�ܷ���0
 */
float getRange(uint16_t recordID)
{
    if (readFRSdata(recordID, 2, 1))
    { // ��ȡ��2���֣����̣�
        uint32_t range = metaData[0];
        return *(float *)&range; // ��32λ����ת��Ϊ������
    }
    return 0.0f;
}

/**
 * @brief ��ȡFRS����������
 * @param recordID FRS��¼ID
 * @param wordNumber �ֱ�ţ���0��ʼ��
 * @return ��ȡ��32λ���ݣ���ʧ�ܷ���0
 */
uint32_t readFRSword(uint16_t recordID, uint8_t wordNumber)
{
    if (readFRSdata(recordID, wordNumber, 1))
    {
        return metaData[0]; // ���ض�ȡ��������
    }
    return 0;
}

/**
 * @brief �����ȡFRS����
 * @param recordID FRS��¼ID
 * @param readOffset ��ȡƫ����������Ϊ��λ��
 * @param blockSize ���ݿ��С������Ϊ��λ��
 */
void frsReadRequest(uint16_t recordID, uint16_t readOffset, uint16_t blockSize)
{
    shtpData[0] = SHTP_REPORT_FRS_READ_REQUEST; // FRS��ȡ����
    shtpData[1] = 0;                            // �����ֽ�
    shtpData[2] = recordID & 0xFF;              // ��¼ID���ֽ�
    shtpData[3] = recordID >> 8;                // ��¼ID���ֽ�
    shtpData[4] = readOffset & 0xFF;            // ƫ�������ֽ�
    shtpData[5] = readOffset >> 8;              // ƫ�������ֽ�
    shtpData[6] = blockSize & 0xFF;             // ���С���ֽ�
    shtpData[7] = blockSize >> 8;               // ���С���ֽ�
    sendPacket(CHANNEL_CONTROL, 8);             // ���͵�����ͨ��������8�ֽ�
}

/**
 * @brief ��ȡFRS����
 * @param recordID FRS��¼ID
 * @param startLocation ��ʼλ�ã��ֱ�ţ�
 * @param wordsToRead Ҫ��ȡ������
 * @return 1��ʾ�ɹ���0��ʾʧ��
 */
uint8_t readFRSdata(uint16_t recordID, uint8_t startLocation, uint8_t wordsToRead)
{
    frsReadRequest(recordID, startLocation, wordsToRead); // ���Ͷ�ȡ����
    HAL_Delay(10);                                        // �ȴ���Ӧ
    for (uint8_t attempts = 0; attempts < 10; attempts++)
    { // ��ೢ��10��
        if (receivePacket() == 1)
        { // �������ݰ�
            if (shtpHeader[2] == CHANNEL_CONTROL && shtpData[0] == SHTP_REPORT_FRS_READ_RESPONSE)
            {                                                                     // �����Ӧ����
                uint16_t readRecordID = (uint16_t)shtpData[3] << 8 | shtpData[2]; // ��ȡ�ļ�¼ID
                if (readRecordID != recordID)
                    continue;                            // ��֤��¼ID
                uint8_t dataOffset = shtpData[5] & 0x0F; // ����ƫ����
                for (uint8_t i = 0; i < wordsToRead; i++)
                {
                    uint8_t index = 6 + (i * 4); // ������ʼ����
                    if (index + 3 < MAX_PACKET_SIZE)
                    {
                        metaData[i] = ((uint32_t)shtpData[index + 3] << 24) |
                                      ((uint32_t)shtpData[index + 2] << 16) |
                                      ((uint32_t)shtpData[index + 1] << 8) |
                                      shtpData[index]; // ���4�ֽ�Ϊ32λ��
                    }
                }
                return 1; // ��ȡ�ɹ�
            }
        }
        HAL_Delay(5); // ÿ�γ��Լ��5ms
    }
    return 0; // ��ȡʧ��
}

/**
 * @brief ����SHTP���ݰ�
 * @return 1��ʾ�ɹ����գ�0��ʾʧ��
 */
static uint8_t receivePacket(void)
{
    uint8_t header[4]; // ͷ��������
    if (HAL_I2C_Master_Receive(hi2c_bno080, _deviceAddress << 1, header, 4, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // ��ȡͷ��ʧ��
    }

    uint16_t packetLength = ((uint16_t)header[1] << 8 | header[0]) & ~(1 << 15); // �������ݰ�����
    if (packetLength <= 4)
    {                                  // �������С�ڵ���ͷ�����ȣ�������
        memcpy(shtpHeader, header, 4); // ����ͷ��
        return 1;                      // ���سɹ�����ͷ����
    }

    uint16_t dataLength = packetLength - 4; // ���ݲ��ֳ���
    memcpy(shtpHeader, header, 4);          // ����ͷ����ȫ�ֱ���

    uint16_t bytesRemaining = dataLength; // ʣ���ֽ���
    uint16_t dataSpot = 0;                // ���ݴ洢λ��

    while (bytesRemaining > 0)
    {                                                                                                                         // �ֿ��ȡ����
        uint16_t numberOfBytesToRead = (bytesRemaining > (I2C_BUFFER_LENGTH - 4)) ? (I2C_BUFFER_LENGTH - 4) : bytesRemaining; // ���㱾�ζ�ȡ�ֽ���
        uint8_t temp[I2C_BUFFER_LENGTH];                                                                                      // ��ʱ������
        if (HAL_I2C_Master_Receive(hi2c_bno080, _deviceAddress << 1, temp, numberOfBytesToRead + 4, HAL_MAX_DELAY) != HAL_OK)
        {
            return 0; // ��ȡʧ��
        }
        for (uint8_t x = 0; x < numberOfBytesToRead; x++)
        {
            if (dataSpot < MAX_PACKET_SIZE)
            {                                       // ��ֹ���������
                shtpData[dataSpot++] = temp[x + 4]; // ����ͷ����4�ֽڣ��洢����
            }
        }
        bytesRemaining -= numberOfBytesToRead; // ����ʣ���ֽ���
    }
    return 1; // ���ճɹ�
}

/**
 * @brief ����SHTP���ݰ�
 * @param channelNumber ͨ���ţ���CHANNEL_CONTROL��
 * @param dataLength ���ݳ��ȣ�������ͷ����
 * @return 1��ʾ���ͳɹ���0��ʾʧ��
 */
static uint8_t sendPacket(uint8_t channelNumber, uint8_t dataLength)
{
    uint16_t packetLength = dataLength + 4; // �ܳ��ȣ�ͷ��4�ֽ� + ���ݣ�
    uint8_t packet[packetLength];           // ���ݰ�������

    packet[0] = packetLength & 0xFF;             // ���ȵ��ֽ�
    packet[1] = (packetLength >> 8) & 0xFF;      // ���ȸ��ֽ�
    packet[2] = channelNumber;                   // ͨ����
    packet[3] = sequenceNumber[channelNumber]++; // ���кţ�������

    for (uint8_t i = 0; i < dataLength; i++)
    {
        packet[4 + i] = shtpData[i]; // �������
    }

    if (HAL_I2C_Master_Transmit(hi2c_bno080, _deviceAddress << 1, packet, packetLength, HAL_MAX_DELAY) != HAL_OK)
    {
        return 0; // ����ʧ��
    }
    return 1; // ���ͳɹ�
}

#define M_PI 3.14159265358979323846
/**
 * @brief ����Ԫ��ת��Ϊŷ����
 * @param quatI ��Ԫ����I���� (x)
 * @param quatJ ��Ԫ����J���� (y)
 * @param quatK ��Ԫ����K���� (z)
 * @param quatReal ��Ԫ����ʵ�� (w)
 * @return ŷ���ǽṹ�壨��λ���ȣ�
 */
void QuaternionToEulerAngles(float quatI, float quatJ, float quatK, float quatReal, 
                            float *roll, float *pitch, float *yaw)
{
    // ��Ԫ����һ��
    float norm = sqrtf(quatI*quatI + quatJ*quatJ + quatK*quatK + quatReal*quatReal);
    quatI /= norm;
    quatJ /= norm;
    quatK /= norm;
    quatReal /= norm;
    
    // ����ŷ����
    // ������ (Pitch)
    float sinp = 2.0f * (quatReal * quatJ - quatK * quatI);
    if (fabsf(sinp) >= 1)
        *pitch = (sinp >= 0 ? 1 : -1) * (M_PI / 2) * 180.0f / M_PI; // ����㴦��
    else
        *pitch = asinf(sinp) * 180.0f / M_PI;
    
    // ����� (Roll)
    float sinr_cosp = 2.0f * (quatReal * quatI + quatJ * quatK);
    float cosr_cosp = 1.0f - 2.0f * (quatI * quatI + quatJ * quatJ);
    *roll = atan2f(sinr_cosp, cosr_cosp) * 180.0f / M_PI;
    
    // ƫ���� (Yaw)
    float siny_cosp = 2.0f * (quatReal * quatK + quatI * quatJ);
    float cosy_cosp = 1.0f - 2.0f * (quatJ * quatJ + quatK * quatK);
    *yaw = atan2f(siny_cosp, cosy_cosp) * 180.0f / M_PI;
}
