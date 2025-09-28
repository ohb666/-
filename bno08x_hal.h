#ifndef __BNO08X_HAL_H_
#define __BNO08X_HAL_H_

#include "stm32f1xx_hal.h"  // STM32 HAL库头文件，根据您的MCU型号调整（如stm32f1xx_hal.h）

/** @defgroup BNO080_Constants BNO080常量定义
  * @{
  */

/** 默认I2C地址，BNO080支持0x4A或0x4B，由ADR引脚决定 */
#define BNO080_DEFAULT_ADDRESS 0x4B

/** I2C缓冲区最大长度，限制单次传输字节数 */
#define I2C_BUFFER_LENGTH 32

/** 最大数据包大小，SHTP协议定义的最大值 */
#define MAX_PACKET_SIZE 128

/** FRS元数据最大长度 */
#define MAX_METADATA_SIZE 9

/** SHTP通道定义 */
#define CHANNEL_COMMAND         0  // 命令通道
#define CHANNEL_EXECUTABLE      1  // 可执行通道
#define CHANNEL_CONTROL         2  // 控制通道
#define CHANNEL_REPORTS         3  // 报告通道
#define CHANNEL_WAKE_REPORTS    4  // 唤醒报告通道
#define CHANNEL_GYRO            5  // 陀螺仪通道

/** SHTP报告ID */
#define SHTP_REPORT_COMMAND_RESPONSE      0xF1  // 命令响应
#define SHTP_REPORT_COMMAND_REQUEST       0xF2  // 命令请求
#define SHTP_REPORT_FRS_READ_RESPONSE     0xF3  // FRS读取响应
#define SHTP_REPORT_FRS_READ_REQUEST      0xF4  // FRS读取请求
#define SHTP_REPORT_PRODUCT_ID_RESPONSE   0xF8  // 产品ID响应
#define SHTP_REPORT_PRODUCT_ID_REQUEST    0xF9  // 产品ID请求
#define SHTP_REPORT_BASE_TIMESTAMP        0xFB  // 基础时间戳
#define SHTP_REPORT_SET_FEATURE_COMMAND   0xFD  // 设置特征命令

/** 传感器报告ID */
#define SENSOR_REPORTID_ACCELEROMETER            0x01  // 加速度计
#define SENSOR_REPORTID_GYROSCOPE                0x02  // 陀螺仪
#define SENSOR_REPORTID_MAGNETIC_FIELD           0x03  // 磁力计
#define SENSOR_REPORTID_LINEAR_ACCELERATION      0x04  // 线性加速度
#define SENSOR_REPORTID_ROTATION_VECTOR          0x05  // 旋转向量
#define SENSOR_REPORTID_GRAVITY                  0x06  // 重力
#define SENSOR_REPORTID_GAME_ROTATION_VECTOR     0x08  // 游戏旋转向量
#define SENSOR_REPORTID_GEOMAGNETIC_ROTATION_VECTOR 0x09  // 地磁旋转向量
#define SENSOR_REPORTID_TAP_DETECTOR             0x10  // 敲击检测
#define SENSOR_REPORTID_STEP_COUNTER             0x11  // 步数计数器
#define SENSOR_REPORTID_STABILITY_CLASSIFIER     0x13  // 稳定性分类器
#define SENSOR_REPORTID_PERSONAL_ACTIVITY_CLASSIFIER 0x1E  // 个人活动分类器

/** FRS记录ID */
#define FRS_RECORDID_ACCELEROMETER          0xE302  // 加速度计记录
#define FRS_RECORDID_GYROSCOPE_CALIBRATED   0xE306  // 校准陀螺仪记录
#define FRS_RECORDID_MAGNETIC_FIELD_CALIBRATED 0xE309  // 校准磁力计记录
#define FRS_RECORDID_ROTATION_VECTOR        0xE30B  // 旋转向量记录

/** 命令ID */
#define COMMAND_ERRORS         1   // 错误命令
#define COMMAND_COUNTER        2   // 计数器命令
#define COMMAND_TARE           3   // 校准命令
#define COMMAND_INITIALIZE     4   // 初始化命令
#define COMMAND_DCD            6   // 数据组件描述符命令
#define COMMAND_ME_CALIBRATE   7   // ME校准命令
#define COMMAND_DCD_PERIOD_SAVE 9  // 周期性保存DCD命令
#define COMMAND_OSCILLATOR     10  // 振荡器命令
#define COMMAND_CLEAR_DCD      11  // 清除DCD命令

/** 校准选项 */
#define CALIBRATE_ACCEL          0  // 校准加速度计
#define CALIBRATE_GYRO           1  // 校准陀螺仪
#define CALIBRATE_MAG            2  // 校准磁力计
#define CALIBRATE_PLANAR_ACCEL   3  // 校准平面加速度计
#define CALIBRATE_ACCEL_GYRO_MAG 4  // 校准加速度计、陀螺仪和磁力计
#define CALIBRATE_STOP           5  // 停止校准

/**
  * @}
  */

/** @defgroup BNO080_Functions BNO080函数声明
  * @{
  */

/** 初始化BNO080传感器
  * @param hi2c I2C句柄指针
  * @param address 设备I2C地址（7位地址）
  */
void BNO080_Init(I2C_HandleTypeDef *hi2c, uint8_t address);

/** 执行软件复位 */
void softReset(void);

/** 获取复位原因
  * @return 复位原因代码
  */
uint8_t resetReason(void);

/** 将固定点数转换为浮点数
  * @param fixedPointValue 固定点数值
  * @param qPoint Q点位置
  * @return 转换后的浮点数值
  */
float qToFloat(int16_t fixedPointValue, uint8_t qPoint);

/** 检查是否有新数据可用
  * @return 1表示有数据，0表示无数据
  */
uint8_t dataAvailable(void);

/** 解析输入报告数据 */
void parseInputReport(void);

/** 获取四元数I分量 */
float getQuatI(void);

/** 获取四元数J分量 */
float getQuatJ(void);

/** 获取四元数K分量 */
float getQuatK(void);

/** 获取四元数实部 */
float getQuatReal(void);

/** 获取四元数弧度精度 */
float getQuatRadianAccuracy(void);

/** 获取四元数精度级别 */
uint8_t getQuatAccuracy(void);

/** 获取X轴加速度 */
float getAccelX(void);

/** 获取Y轴加速度 */
float getAccelY(void);

/** 获取Z轴加速度 */
float getAccelZ(void);

/** 获取加速度精度级别 */
uint8_t getAccelAccuracy(void);

/** 获取X轴线性加速度 */
float getLinAccelX(void);

/** 获取Y轴线性加速度 */
float getLinAccelY(void);

/** 获取Z轴线性加速度 */
float getLinAccelZ(void);

/** 获取线性加速度精度级别 */
uint8_t getLinAccelAccuracy(void);

/** 获取X轴陀螺仪数据 */
float getGyroX(void);

/** 获取Y轴陀螺仪数据 */
float getGyroY(void);

/** 获取Z轴陀螺仪数据 */
float getGyroZ(void);

/** 获取陀螺仪精度级别 */
uint8_t getGyroAccuracy(void);

/** 获取X轴磁力计数据 */
float getMagX(void);

/** 获取Y轴磁力计数据 */
float getMagY(void);

/** 获取Z轴磁力计数据 */
float getMagZ(void);

/** 获取磁力计精度级别 */
uint8_t getMagAccuracy(void);

/** 校准加速度计 */
void calibrateAccelerometer(void);

/** 校准陀螺仪 */
void calibrateGyro(void);

/** 校准磁力计 */
void calibrateMagnetometer(void);

/** 校准平面加速度计 */
void calibratePlanarAccelerometer(void);

/** 校准所有传感器 */
void calibrateAll(void);

/** 结束校准过程 */
void endCalibration(void);

/** 保存校准数据 */
void saveCalibration(void);

/** 获取步数计数
  * @return 当前步数
  */
uint16_t getStepCount(void);

/** 获取稳定性分类
  * @return 稳定性状态
  */
uint8_t getStabilityClassifier(void);

/** 获取活动分类
  * @return 活动类型
  */
uint8_t getActivityClassifier(void);

/** 设置传感器特征命令
  * @param reportID 报告ID
  * @param timeBetweenReports 报告间隔时间（毫秒）
  * @param specificConfig 特定配置参数
  */
void setFeatureCommand(uint8_t reportID, uint32_t timeBetweenReports, uint32_t specificConfig);

/** 发送命令
  * @param command 命令ID
  */
void sendCommand(uint8_t command);

/** 发送校准命令
  * @param thingToCalibrate 要校准的对象
  */
void sendCalibrateCommand(uint8_t thingToCalibrate);

/** 获取FRS记录的Q1值
  * @param recordID FRS记录ID
  * @return Q1值
  */
int16_t getQ1(uint16_t recordID);

/** 获取FRS记录的Q2值
  * @param recordID FRS记录ID
  * @return Q2值
  */
int16_t getQ2(uint16_t recordID);

/** 获取FRS记录的Q3值
  * @param recordID FRS记录ID
  * @return Q3值
  */
int16_t getQ3(uint16_t recordID);

/** 获取传感器分辨率
  * @param recordID FRS记录ID
  * @return 分辨率（浮点数）
  */
float getResolution(uint16_t recordID);

/** 获取传感器量程
  * @param recordID FRS记录ID
  * @return 量程（浮点数）
  */
float getRange(uint16_t recordID);

/** 读取FRS字数据
  * @param recordID FRS记录ID
  * @param wordNumber 字编号
  * @return 读取的32位数据
  */
uint32_t readFRSword(uint16_t recordID, uint8_t wordNumber);

/** 请求读取FRS数据
  * @param recordID FRS记录ID
  * @param readOffset 读取偏移量
  * @param blockSize 数据块大小
  */
void frsReadRequest(uint16_t recordID, uint16_t readOffset, uint16_t blockSize);

/** 读取FRS数据
  * @param recordID FRS记录ID
  * @param startLocation 起始位置
  * @param wordsToRead 要读取的字数
  * @return 1表示成功，0表示失败
  */
uint8_t readFRSdata(uint16_t recordID, uint8_t startLocation, uint8_t wordsToRead);

/** 启用旋转向量报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableRotationVector(uint32_t timeBetweenReports);

/** 启用游戏旋转向量报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableGameRotationVector(uint32_t timeBetweenReports);

/** 启用加速度计报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableAccelerometer(uint32_t timeBetweenReports);

/** 启用线性加速度计报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableLinearAccelerometer(uint32_t timeBetweenReports);

/** 启用陀螺仪报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableGyro(uint32_t timeBetweenReports);

/** 启用磁力计报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableMagnetometer(uint32_t timeBetweenReports);

/** 启用步数计数器报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableStepCounter(uint32_t timeBetweenReports);

/** 启用稳定性分类器报告
  * @param timeBetweenReports 报告间隔时间（毫秒）
  */
void enableStabilityClassifier(uint32_t timeBetweenReports);

/** 四元数转欧拉角
  * @param 参数
  */
void QuaternionToEulerAngles(float quatI, float quatJ, float quatK, float quatReal, 
                            float *roll, float *pitch, float *yaw);

/**
  * @}
  */

#endif /* __BNO08X_HAL_H_ */
