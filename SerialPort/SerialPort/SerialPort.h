#pragma once
#include<iostream>
#include<stdio.h>
#include<string>
#include<vector>
#include<sstream>
#include<iomanip>
#include<thread>
#include <conio.h>
#include<WinSock2.h>
#include<Windows.h>
#pragma  warning (disable:4996) 
using namespace std;


enum DEVICE_STATE {
	INIT_STATE = 100,
	//指令应答
	COMMAND_ANSWER_STATE,
	//配置参数应答
	CONFIGURATION_PARAMETER_STATE,
	//设备固件版本应答
	FIRMWARE_VERSION_STATE,
	//产品序列号应答
	PRODUCT_NO_STATE,
	//准备就绪应答
	BE_READY_STATE,
	//温度返回应答
	RETURN_TEMPERATURE_STATE,
};


class SerialPort {
public:
	SerialPort();
	~SerialPort();
	
	/*
	打开串口
	portname:串口名，COM3，COM4..
	baudrate:波特率
	parity:校验位，0为无校验，1为奇校验，2为偶校验，3为标记校验
	databit:数据位
	stopbit:停止位，1为1位停止位，2为2位停止位，3为1.5位停止位
	synchronizable:同步、异步，0为异步，1为同步
	*/
	bool open(const char* portName, int baudrate = 460800, char parity = 0, char databit = 8, char stopbit = 1, char synchronizeflag = 0);
	//关闭串口
	void close();
	//发送数据，发送成功返回数据长度，失败返回0
	int send(unsigned char *a,int length);
	//接收数据，成功返回读取实际数据的长度，失败返回0
	int receive(char *rcvBuf);
	//解析下位机应答
	void praseCommand(unsigned char *buf);
	//从缓冲池中查找帧头
	int findHead(unsigned char *buf);
	//从缓冲池中查找帧尾
	int findTail(unsigned char *buf);
	//计算校验和
	char excuteCheck(unsigned char* data,int length);
	//应答状态/超时 判断
	bool WaitReplay(DEVICE_STATE state, int timeout);
	//线程函数，接收下位机应答保存在缓冲池中并调用指令解析函数
	void getReceive();


	//interface
	/*
	通道占空比设置
	参数说明：1.通道参数，1-10
	2.占空比参数，范围是0x00-0x64 表示0-100
	返回指令应答*/
	bool sendCDC(int channel, int dutyCycl);
	/*
	通道开关设置
	参数说明：
	返回指令应答*/
	bool sendCSC(string controlSet);	//todo
	/*
	光源总开关设置
	参数说明：Y表示激光器打开，N表示激光器关闭
	返回指令应答*/
	bool sendLMS(char on);
	/*
	触发模式配置
	参数说明：1表示指令触发，2表示电平触发
	返回指令应答*/
	bool sendTGM(char on);
	/*
	指令触发控制
	返回指令应答*/
	bool sendTGC();
	/*
	参数读取指令
	返回配置参数应答指令和温度应答指令*/
	bool sendPRD();
	/*
	参数写入指令
	返回指令应答*/
	bool sendPWR();
	/*
	恢复出厂设置指令
	返回指令应答*/
	bool sendFDR();
	/*
	设备固件版本查询指令
	返回设备固件版本号应答*/
	bool sendFWV();
	/*
	设备自检指令
	返回指令应答命令*/
	bool sendESC();

private:
	int pHandle[16];
	bool synchronizeflag;
	bool bThreadRun;
	//接收应答缓冲区
	unsigned char* buffer = new unsigned char[1024 * 1024];
	//当前缓冲区内的char数量
	int front;
	DEVICE_STATE nowState;
};

//菜单控制函数
//void MenuContrl(SerialPort p);
//计算校验位
string CheckDigit(int channel, int luminance);
//将字符串转为16进制输出
string printAsHex(const string &str); 