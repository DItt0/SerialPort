#include "SerialPort.h"

SerialPort::SerialPort(){
	bThreadRun = false;
	memset(buffer, 0, sizeof(unsigned char) * 1024 * 1024);
	front = 0;
	nowState = INIT_STATE;
}

SerialPort::~SerialPort() {

}

bool SerialPort::open(const char* portName,int baudrate,char parity,char databit,char stopbit,char synchronizeflag){

	this->synchronizeflag = synchronizeflag;
	HANDLE hcom = NULL;

	//拼接串口号
	char fullPortName[10];
	sprintf_s(fullPortName, "\\\\.\\COM%s", portName);
	
	if (this->synchronizeflag) {
		hcom = CreateFileA(fullPortName,	//串口名
			GENERIC_READ | GENERIC_WRITE,	//支持读写
			0,							//独占方式，串口不支持共享
			NULL,						//安全属性指针，默认值为NULL
			OPEN_EXISTING,				//打开现有的串口文件
			0,							//同步方式
			NULL);						//用于复制句柄文件，默认值为NULL，对串口而言该值必须置为NULL
	}
	else {
		hcom = CreateFileA(fullPortName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,		//异步方式
			NULL);
	}

	if (hcom == (HANDLE)-1) {
		return false;
	}
	//配置缓冲区大小
	/*
	hcom通信句柄
	输入缓冲区的大小（字节数）
	输出缓冲区的大小
	*/
	if (!SetupComm(hcom, 1024, 1024)) {
		return false;
	}
	DCB dcb;	//定义串行通信设备的控制设置
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = baudrate;	//设置波特率
	dcb.ByteSize = databit;		//设置数据位

	switch (parity) {	//校验位判断
	case 0:
		dcb.Parity = NOPARITY;	//无校验
		break;
	case 1:
		dcb.Parity = ODDPARITY;	//奇校验
		break;
	case 2:
		dcb.Parity = EVENPARITY;//偶校验
	case 3:
		dcb.Parity = MARKPARITY;//标记校验
	}

	switch (stopbit) {	//停止位判断
	case 0:
		dcb.StopBits = ONESTOPBIT;	//1位停止位
		break;
	case 1:
		dcb.StopBits = TWOSTOPBITS;	//2位停止位
		break;
	case 2:
		dcb.StopBits = ONE5STOPBITS;//1.5位停止位
		break;
	}

	if (!SetCommState(hcom, &dcb))	return false;//配置参数失败

	//设置读超时
	COMMTIMEOUTS timesouts;
	timesouts.ReadIntervalTimeout = 50;		//读时间间隔超时
	timesouts.ReadTotalTimeoutConstant = 0;//读时间常量
	timesouts.ReadTotalTimeoutMultiplier = 0;//读时间系数
	timesouts.WriteTotalTimeoutConstant = 50;//写时间常量
	timesouts.WriteTotalTimeoutMultiplier = 10;//写时间系数	单位均为毫秒
	SetCommTimeouts(hcom, &timesouts);

	PurgeComm(hcom, PURGE_TXCLEAR | PURGE_RXCLEAR);	//清空串口缓存
	::memcpy(pHandle, &hcom, sizeof(hcom));	//保存句柄

	//开启读取线程
	bThreadRun = true;
	thread receiveThread(&SerialPort::getReceive,this);
	receiveThread.detach();
	return true;
}

void SerialPort::close() {
	bThreadRun = false;
	HANDLE hcom = *(HANDLE*)pHandle;
	CloseHandle(hcom);
}

int SerialPort::send(unsigned char* data,int dataLength) {
	HANDLE hcom = *(HANDLE*)pHandle;
//	int dataLength = sizeof(data) / sizeof(unsigned char);
	DWORD dwbytewrite = dataLength;	//成功写入的数据字节数
	if (this->synchronizeflag) {	//同步方式
		BOOL bWritestat = WriteFile(hcom,	//句柄
			data,	//数据首地址
			dwbytewrite,	//要发送的数据字节数
			&dwbytewrite,	//用来接收返回成功发送的数据字节数
			NULL);	//同步方式

		if (!bWritestat)	return 0;
		return dwbytewrite;
	}
	else {//异步
		int dataLength = sizeof(data) / sizeof(unsigned char);
		DWORD dwErrorFlags;	//错误标志
		COMSTAT comstat;	//通讯状态
		OVERLAPPED osWrite;	//异步输入输出结构体
		memset(&osWrite, 0, sizeof(osWrite));
		osWrite.hEvent = CreateEvent(NULL, true, false, "WriteEvent");
		ClearCommError(hcom, &dwErrorFlags, &comstat);	//清楚通讯错误，获取设备当前状态
		BOOL bWritestat = WriteFile(hcom,
			data,
			dwbytewrite,
			&dwbytewrite,
			&osWrite);	//OVERLAPPED为异步发送
		if (!dwbytewrite) {
			if (GetLastError() == ERROR_IO_PENDING) {	//如果串口正在被写入
				WaitForSingleObject(osWrite.hEvent, 1000);
			}
			else {
				ClearCommError(hcom, &dwErrorFlags, &comstat);	//清楚通讯错误
				CloseHandle(osWrite.hEvent);
				return 0;
			}
		}
		return dataLength;
	}
}

int SerialPort::receive(char *rcvBuf) {
	HANDLE hcom = *(HANDLE*)pHandle;
//	PurgeComm(hcom, PURGE_TXCLEAR | PURGE_RXCLEAR);	//清空串口缓存
	char buf[512];
	if (this->synchronizeflag) {
		DWORD bytesRead;	//成功读取的数据字节数
		BOOL bReadstat = ReadFile(hcom,	//串口句柄
			buf,	//数据首地址
			sizeof(buf),	//要读取的数据最大字节数
			&bytesRead,	//用来接收返回成功发送的数据字节数
			NULL);	//同步方式
		if (bytesRead) {
			::memcpy(rcvBuf, buf, bytesRead);
			return bytesRead;
		}
	}
	//异步
	else {
		//DWORD 无符号长类型
		DWORD wCount = 1024;	//成功读取的数据字节数
		DWORD dwErrorFlags;		//错误标志
		COMSTAT comstat;		//通讯状态
		OVERLAPPED osRead;		//异步输入输出结构体
		memset(&osRead, 0, sizeof(osRead));
		osRead.hEvent = CreateEvent(NULL, true, false, "ReadEvent");

		ClearCommError(hcom, &dwErrorFlags, &comstat);
//		if (!comstat.cbInQue)	return "7";
		BOOL bReadstat = ReadFile(hcom,	//文件句柄
			buf,	//数据首地址
			wCount,	//读取数据的最大字节数
			&wCount,//接收返回成功发送的数据字节数
			&osRead);	//异步方式
		if (!bReadstat) {
			if (GetLastError() == ERROR_IO_PENDING) {	//串口正在被读取
				GetOverlappedResult(hcom, &osRead, &wCount, true);
			}
			else {
				ClearCommError(hcom, &dwErrorFlags, &comstat);
				CloseHandle(osRead.hEvent);
				//return "5";
			}
		}
		if (wCount) {
			::memcpy(rcvBuf, buf, wCount);
			return wCount;
		}
		return 0;
	}
}

char SerialPort::excuteCheck(unsigned char* data,int length) {
	//定义校验位
	char check = 0;
	//获取数组长度
//	int length = sizeof(data) / sizeof(unsigned char);
	for (int i = 1; i <= length; i++) {
		check ^= data[i];
	}
	return check;
}

bool SerialPort::sendLMS(char on){
	unsigned char data[8] = { 0x7B,0xA8,0x4C,0x4D,0x53,on };
	char check = excuteCheck(data,5);
	data[6] = check;
	data[7] = 0x7D;
	int count = send(data,8);
	if (count) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	else
		return false;
}

bool SerialPort::sendTGM(char on)
{
	unsigned char data[8] = { 0x7B,0xA8,0x54,0x47,0x4D,on };
	char check = excuteCheck(data,5);
	data[6] = check;
	data[7] = 0x7D;
	int count = send(data,8);
	if (count) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendTGC()
{
	unsigned char data[7] = { 0x7B,0xA7,0x54,0x47,0x43 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(CONFIGURATION_PARAMETER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendPRD()
{
	unsigned char data[7] = { 0x7B,0xA7,0x50,0x52,0x44 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendPWR()
{
	unsigned char data[7] = { 0x7B,0xA7,0x50,0x57,0x52 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendFDR()
{
	unsigned char data[7] = { 0x7B,0xA7,0x46,0x44,0x52 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendFWV()
{
	unsigned char data[7] = { 0x7B,0xA7,0x46,0x57,0x56 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendESC()
{
	unsigned char data[7] = { 0x7B,0xA7,0x45,0x53,0x43 };
	char check = excuteCheck(data,4);
	data[5] = check;
	data[6] = 0x7D;
	if (send(data,7)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	return false;
}

bool SerialPort::sendCDC(int channel, int dutyCycle) {
	unsigned char data[9] = { 0x7B,0xA9,0x43,0x44,0x43,channel,dutyCycle };
	char check = excuteCheck(data,6);
	data[7] = check;
	data[8] = 0x7D;
	if (send(data,9)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	else
		return false;
}

bool SerialPort::sendCSC(string controlSet) {
	unsigned char data[17] = { 0x7B,0xB1,0x43,0x53,0x43 };
	for (int i = 5; i <= 14; i++) {
		data[i] = controlSet[i - 5];
	}
	char check = excuteCheck(data,14);
	data[15] = check;
	data[16] = 0x7D;
	if (send(data,17)) {
		return WaitReplay(COMMAND_ANSWER_STATE, 2000);
	}
	else
		return false;
}

bool SerialPort::WaitReplay(DEVICE_STATE state, int timeout)
{
	//发送指令后等待应答时间
	clock_t startTime = clock();
	while (1)
	{
		//获取应答时间
		clock_t stopTime = clock();
		//当应答状态为指令应答或者状态应答时
		if (nowState == state)
		{
			//初始化应答
			nowState = INIT_STATE;
			return true;
		}
		if (stopTime - startTime > timeout)
			return false;
		Sleep(50);
	}
}

void SerialPort::getReceive() {
	while(bThreadRun){
		int bytes = receive((char*)buffer + front);
		//vector<string> Receive_vector = stringToVector(printAsHex(ret));
		front += bytes;
		//缓冲区满
		if (front > 1024 * 1000)
		{
			//重置缓冲区
			memset(buffer, 0, sizeof(unsigned char) * 1024 * 1024);
			front = 0;
			continue;
		}
		//调用指令解析函数
		praseCommand(buffer);
/*		int i = 0;
		while (buffer[i] != '\0') {
			cout << buffer[i];
			i++;
		}*/
		//Sleep(100);
	}
}

//将参数中的字符串转为16进制
string printAsHex(const string &str) {
	stringstream ss;
	ss << hex << uppercase << setfill('0');
	for (char c : str) {
		ss << setw(2) << static_cast<int>(static_cast<unsigned char>(c)) << " ";
		
	}
	return ss.str();
//	cout << ss.str() << endl;
}

/*void MenuContrl(SerialPort p) {
	static bool flag1=true, flag2=true, flag3=true;
	//string ret;
	char order = _getch();
//	cin >> order;
	cout << endl;
	switch (order) {
	case 'Z':
	case 'z':
		//激光器总开关打开指令
//		p.send("7B A8 4C 4D 53 59 A3 7D");
		p.lazerSwitch('Y');
		break;
	case 'X':
	case 'x':
		//激光器总开关关闭指令
//		p.send("7B A8 4C 4D 53 4E B4 7D");
//		p.sendEnd();
		break;
	case 'A':
	case 'a':
		//激光器通道总关闭指令
		//p.send("7B B1 43 53 43 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E E2 7D");
		p.channelEnd();
		cout << "激光器通道全部关闭" << endl;
		flag1 = flag2 = flag3 = false;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'S':
	case 's':
		//打开激光器通道1指令
		//p.send("7B B1 43 53 43 59 59 59 59 59 59 59 59 59 59 E2 7D");
		p.channel1open();
		flag1 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'D':
	case 'd':
		//打开激光器通道2指令
		//p.send("7B B1 43 53 43 4E 59 4E 4E 4E 4E 4E 4E 4E 4E F5 7D");
//		p.channel2open();
		flag2 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'F':
	case 'f':
		//打开激光器通道3指令
		//p.send("7B B1 43 53 43 4E 59 4E 4E 4E 4E 4E 4E 4E 4E F5 7D");
		p.channel3open();
		flag3 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'C':
	case 'c':
		ShowChannel(flag1, flag2, flag3);
		/*int channel, luminance;
		cout << "输入选择的激光通道（1，2，3）:";
		cin >> channel;
		cout << endl;
		cout << "输入激光占空比：";
		cin >> luminance;
		cout << endl;
		ret = CheckDigit(channel, luminance);
		p.send(ret);
		p.razerLuminance();
		break;
	default:
		
		p.close();
//		exit(-1);
	}
}*/

//发送调用指令
//定义方法自动计算指令校验位	异或运算符	^
string CheckDigit (const int channel, const int luminance) {
	string ret = "7B ";
	vector<int> a = { 0xA9,0x43,0x44,0x43 };	//数据位数组
	int check = a[0] ^ a[1];	//校验位
	a.push_back(channel);
	a.push_back(luminance);
	for (int i = 0; i < a.size(); i++) {	//for循环计算校验位
		if (i >= 2) {
			check = check ^ a[i];
		}
		stringstream ss;
		/*
		hex：指定流中16进制
		uppercase：指定十六进制数字和科学记数法中的指数以大写形式显示
		setw：指定流中下一个元素的显示字段的宽度	iomanip
		setfill：设置用于填充右对齐显示中的空格字符	iomanip
		*/
		ss << hex << uppercase << setw(2) << setfill('0') << a[i];
		//字符串拼接
		ret = ret + ss.str() + " ";
	}
	stringstream ss;
	//校验位转换为16进制末尾拼接
	ss << hex << check;
	ret += ss.str() + " " + "7D";
	//将小写字母转换为大写字母
/*	for (int i = 0; i < ret.length(); i++) {
		if (ret[i] >= 'a'&&ret[i] <= 'f')
			ret[i] -= 32;
	}*/
	return ret;
}


void SerialPort::praseCommand(unsigned char *buf) {
	while (1) {
		int head = findHead(buf);
		if (head != -1) {
			int tail = findTail(buf);
			if (tail != -1) {
				//当前buf[head] == 0x7B
				//CAN
				if (buf[head+2] == 0x43 && buf[head+3] == 0x41 && buf[head+4] == 0x4E) {
					nowState = COMMAND_ANSWER_STATE;
					if (buf[head + 5] == 0x00)	cout << "指令应答，命令格式正确" << endl;
					else if (buf[head + 5] == 0x01)	cout << "指令应答，命令格式错误" << endl;
					else if (buf[head + 5] == 0x02)	cout << "指令应答，命令校验和错误" << endl;
				}
				//CFG
				else if (buf[head + 2] == 0x43 && buf[head + 3] == 0x46 && buf[head + 4] == 0x47) {
					nowState = CONFIGURATION_PARAMETER_STATE;
					float hardwareVersion = buf[head + 5] * 0.1;
					float softwareVersion = buf[head + 6] * 0.1;
					//占空比输出
					//pass
					cout << "配置参数应答，" << "相机曝光参数" << buf[head + 25] << "硬件版本：V" << hardwareVersion << "，软件版本：V" << softwareVersion << endl;
				}
				//FWV
				else if (buf[head + 2] == 0x46 && buf[head + 3] == 0x57 && buf[head + 4] == 0x56) {
					nowState = FIRMWARE_VERSION_STATE;
					float hardwareVersion = buf[head + 5] * 0.1;
					float softwareVersion = buf[head + 6] * 0.1;
					cout << "固件版本应答，硬件版本：V" << hardwareVersion << "，软件版本：V" << softwareVersion << endl;
				}
				//SNA
				else if (buf[head + 2] == 0x53 && buf[head + 3] == 0x4E && buf[head + 4] == 0x41) {
					nowState = PRODUCT_NO_STATE;
					string SNA;
					//VXSED
					if (buf[head + 5] == 0x56 && buf[head + 6] == 0x58 && buf[head + 7] == 0x53 && buf[head + 8] == 0x45 && buf[head + 9] == 0x44) {
						SNA = "四轮定位检测设备,";
					}
					//VPSSS
					if (buf[head + 5] == 0x56 && buf[head + 6] == 0x50 && buf[head + 7] == 0x53 && buf[head + 8] == 0x53 && buf[head + 9] == 0x53) {
						SNA = "轮眉高度测量设备,";
					}
					cout << "产品序列号应答，" << SNA << "生产日期：" << buf[head + 10] << buf[head + 11] << buf[head + 12] << buf[head + 13] << "年" << buf[head + 14] << buf[head + 15] << "月，" << "序列号：" << buf[head + 16] << buf[head + 17] << buf[head + 18] << buf[head + 19] << endl;
				}
				//RDY
				else if (buf[head + 2] == 0x52 && buf[head + 3] == 0x44 && buf[head + 4] == 0x59) {
					nowState = BE_READY_STATE;
					cout << "准备就绪应答" << endl;
				}
				//TPV
				else if (buf[head + 2] == 0x54 && buf[head + 3] == 0x50 && buf[head + 4] == 0x56) {
					nowState = RETURN_TEMPERATURE_STATE;
					float temperature = (buf[head + 5] * 256 + buf[head + 6]) * 0.01;
					cout << "温度返回应答，当前温度" << temperature << "℃" << endl;
				}
				else {
					cout << "下位机应答错误" << endl;
				}
				front -= tail+1;
				::memcpy(buf, buf + tail+1, front);
			}
			else {
				//查找不到帧尾
				break;
			}
		}
		else {
			//查找不到帧头
			break;
		}
	}
}

int SerialPort::findHead(unsigned char *buf) {
	for (int i = 0; i < front; i++) {
		if (buf[i] == 0x7B) {
			return i;
		}
	}
	//返回-1时表示当前缓冲池数据查找不到帧头
	return -1;
}

int SerialPort::findTail(unsigned char *buf) {
	//从查找到的帧头开始继续向后遍历查找帧尾
	int head = findHead(buf);
	for (int i = head+1; i < front; i++) {
		if (buf[i] == 0x7D) {
			return i;
		}
	}
	return -1;
}

