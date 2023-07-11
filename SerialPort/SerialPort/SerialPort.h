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
	//ָ��Ӧ��
	COMMAND_ANSWER_STATE,
	//���ò���Ӧ��
	CONFIGURATION_PARAMETER_STATE,
	//�豸�̼��汾Ӧ��
	FIRMWARE_VERSION_STATE,
	//��Ʒ���к�Ӧ��
	PRODUCT_NO_STATE,
	//׼������Ӧ��
	BE_READY_STATE,
	//�¶ȷ���Ӧ��
	RETURN_TEMPERATURE_STATE,
};


class SerialPort {
public:
	SerialPort();
	~SerialPort();
	
	/*
	�򿪴���
	portname:��������COM3��COM4..
	baudrate:������
	parity:У��λ��0Ϊ��У�飬1Ϊ��У�飬2ΪżУ�飬3Ϊ���У��
	databit:����λ
	stopbit:ֹͣλ��1Ϊ1λֹͣλ��2Ϊ2λֹͣλ��3Ϊ1.5λֹͣλ
	synchronizable:ͬ�����첽��0Ϊ�첽��1Ϊͬ��
	*/
	bool open(const char* portName, int baudrate = 460800, char parity = 0, char databit = 8, char stopbit = 1, char synchronizeflag = 0);
	//�رմ���
	void close();
	//�������ݣ����ͳɹ��������ݳ��ȣ�ʧ�ܷ���0
	int send(unsigned char *a,int length);
	//�������ݣ��ɹ����ض�ȡʵ�����ݵĳ��ȣ�ʧ�ܷ���0
	int receive(char *rcvBuf);
	//������λ��Ӧ��
	void praseCommand(unsigned char *buf);
	//�ӻ�����в���֡ͷ
	int findHead(unsigned char *buf);
	//�ӻ�����в���֡β
	int findTail(unsigned char *buf);
	//����У���
	char excuteCheck(unsigned char* data,int length);
	//Ӧ��״̬/��ʱ �ж�
	bool WaitReplay(DEVICE_STATE state, int timeout);
	//�̺߳�����������λ��Ӧ�𱣴��ڻ�����в�����ָ���������
	void getReceive();


	//interface
	/*
	ͨ��ռ�ձ�����
	����˵����1.ͨ��������1-10
	2.ռ�ձȲ�������Χ��0x00-0x64 ��ʾ0-100
	����ָ��Ӧ��*/
	bool sendCDC(int channel, int dutyCycl);
	/*
	ͨ����������
	����˵����
	����ָ��Ӧ��*/
	bool sendCSC(string controlSet);	//todo
	/*
	��Դ�ܿ�������
	����˵����Y��ʾ�������򿪣�N��ʾ�������ر�
	����ָ��Ӧ��*/
	bool sendLMS(char on);
	/*
	����ģʽ����
	����˵����1��ʾָ�����2��ʾ��ƽ����
	����ָ��Ӧ��*/
	bool sendTGM(char on);
	/*
	ָ�������
	����ָ��Ӧ��*/
	bool sendTGC();
	/*
	������ȡָ��
	�������ò���Ӧ��ָ����¶�Ӧ��ָ��*/
	bool sendPRD();
	/*
	����д��ָ��
	����ָ��Ӧ��*/
	bool sendPWR();
	/*
	�ָ���������ָ��
	����ָ��Ӧ��*/
	bool sendFDR();
	/*
	�豸�̼��汾��ѯָ��
	�����豸�̼��汾��Ӧ��*/
	bool sendFWV();
	/*
	�豸�Լ�ָ��
	����ָ��Ӧ������*/
	bool sendESC();

private:
	int pHandle[16];
	bool synchronizeflag;
	bool bThreadRun;
	//����Ӧ�𻺳���
	unsigned char* buffer = new unsigned char[1024 * 1024];
	//��ǰ�������ڵ�char����
	int front;
	DEVICE_STATE nowState;
};

//�˵����ƺ���
//void MenuContrl(SerialPort p);
//����У��λ
string CheckDigit(int channel, int luminance);
//���ַ���תΪ16�������
string printAsHex(const string &str); 