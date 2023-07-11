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

	//ƴ�Ӵ��ں�
	char fullPortName[10];
	sprintf_s(fullPortName, "\\\\.\\COM%s", portName);
	
	if (this->synchronizeflag) {
		hcom = CreateFileA(fullPortName,	//������
			GENERIC_READ | GENERIC_WRITE,	//֧�ֶ�д
			0,							//��ռ��ʽ�����ڲ�֧�ֹ���
			NULL,						//��ȫ����ָ�룬Ĭ��ֵΪNULL
			OPEN_EXISTING,				//�����еĴ����ļ�
			0,							//ͬ����ʽ
			NULL);						//���ڸ��ƾ���ļ���Ĭ��ֵΪNULL���Դ��ڶ��Ը�ֵ������ΪNULL
	}
	else {
		hcom = CreateFileA(fullPortName,
			GENERIC_READ | GENERIC_WRITE,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,		//�첽��ʽ
			NULL);
	}

	if (hcom == (HANDLE)-1) {
		return false;
	}
	//���û�������С
	/*
	hcomͨ�ž��
	���뻺�����Ĵ�С���ֽ�����
	����������Ĵ�С
	*/
	if (!SetupComm(hcom, 1024, 1024)) {
		return false;
	}
	DCB dcb;	//���崮��ͨ���豸�Ŀ�������
	memset(&dcb, 0, sizeof(dcb));
	dcb.DCBlength = sizeof(dcb);
	dcb.BaudRate = baudrate;	//���ò�����
	dcb.ByteSize = databit;		//��������λ

	switch (parity) {	//У��λ�ж�
	case 0:
		dcb.Parity = NOPARITY;	//��У��
		break;
	case 1:
		dcb.Parity = ODDPARITY;	//��У��
		break;
	case 2:
		dcb.Parity = EVENPARITY;//żУ��
	case 3:
		dcb.Parity = MARKPARITY;//���У��
	}

	switch (stopbit) {	//ֹͣλ�ж�
	case 0:
		dcb.StopBits = ONESTOPBIT;	//1λֹͣλ
		break;
	case 1:
		dcb.StopBits = TWOSTOPBITS;	//2λֹͣλ
		break;
	case 2:
		dcb.StopBits = ONE5STOPBITS;//1.5λֹͣλ
		break;
	}

	if (!SetCommState(hcom, &dcb))	return false;//���ò���ʧ��

	//���ö���ʱ
	COMMTIMEOUTS timesouts;
	timesouts.ReadIntervalTimeout = 50;		//��ʱ������ʱ
	timesouts.ReadTotalTimeoutConstant = 0;//��ʱ�䳣��
	timesouts.ReadTotalTimeoutMultiplier = 0;//��ʱ��ϵ��
	timesouts.WriteTotalTimeoutConstant = 50;//дʱ�䳣��
	timesouts.WriteTotalTimeoutMultiplier = 10;//дʱ��ϵ��	��λ��Ϊ����
	SetCommTimeouts(hcom, &timesouts);

	PurgeComm(hcom, PURGE_TXCLEAR | PURGE_RXCLEAR);	//��մ��ڻ���
	::memcpy(pHandle, &hcom, sizeof(hcom));	//������

	//������ȡ�߳�
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
	DWORD dwbytewrite = dataLength;	//�ɹ�д��������ֽ���
	if (this->synchronizeflag) {	//ͬ����ʽ
		BOOL bWritestat = WriteFile(hcom,	//���
			data,	//�����׵�ַ
			dwbytewrite,	//Ҫ���͵������ֽ���
			&dwbytewrite,	//�������շ��سɹ����͵������ֽ���
			NULL);	//ͬ����ʽ

		if (!bWritestat)	return 0;
		return dwbytewrite;
	}
	else {//�첽
		int dataLength = sizeof(data) / sizeof(unsigned char);
		DWORD dwErrorFlags;	//�����־
		COMSTAT comstat;	//ͨѶ״̬
		OVERLAPPED osWrite;	//�첽��������ṹ��
		memset(&osWrite, 0, sizeof(osWrite));
		osWrite.hEvent = CreateEvent(NULL, true, false, "WriteEvent");
		ClearCommError(hcom, &dwErrorFlags, &comstat);	//���ͨѶ���󣬻�ȡ�豸��ǰ״̬
		BOOL bWritestat = WriteFile(hcom,
			data,
			dwbytewrite,
			&dwbytewrite,
			&osWrite);	//OVERLAPPEDΪ�첽����
		if (!dwbytewrite) {
			if (GetLastError() == ERROR_IO_PENDING) {	//����������ڱ�д��
				WaitForSingleObject(osWrite.hEvent, 1000);
			}
			else {
				ClearCommError(hcom, &dwErrorFlags, &comstat);	//���ͨѶ����
				CloseHandle(osWrite.hEvent);
				return 0;
			}
		}
		return dataLength;
	}
}

int SerialPort::receive(char *rcvBuf) {
	HANDLE hcom = *(HANDLE*)pHandle;
//	PurgeComm(hcom, PURGE_TXCLEAR | PURGE_RXCLEAR);	//��մ��ڻ���
	char buf[512];
	if (this->synchronizeflag) {
		DWORD bytesRead;	//�ɹ���ȡ�������ֽ���
		BOOL bReadstat = ReadFile(hcom,	//���ھ��
			buf,	//�����׵�ַ
			sizeof(buf),	//Ҫ��ȡ����������ֽ���
			&bytesRead,	//�������շ��سɹ����͵������ֽ���
			NULL);	//ͬ����ʽ
		if (bytesRead) {
			::memcpy(rcvBuf, buf, bytesRead);
			return bytesRead;
		}
	}
	//�첽
	else {
		//DWORD �޷��ų�����
		DWORD wCount = 1024;	//�ɹ���ȡ�������ֽ���
		DWORD dwErrorFlags;		//�����־
		COMSTAT comstat;		//ͨѶ״̬
		OVERLAPPED osRead;		//�첽��������ṹ��
		memset(&osRead, 0, sizeof(osRead));
		osRead.hEvent = CreateEvent(NULL, true, false, "ReadEvent");

		ClearCommError(hcom, &dwErrorFlags, &comstat);
//		if (!comstat.cbInQue)	return "7";
		BOOL bReadstat = ReadFile(hcom,	//�ļ����
			buf,	//�����׵�ַ
			wCount,	//��ȡ���ݵ�����ֽ���
			&wCount,//���շ��سɹ����͵������ֽ���
			&osRead);	//�첽��ʽ
		if (!bReadstat) {
			if (GetLastError() == ERROR_IO_PENDING) {	//�������ڱ���ȡ
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
	//����У��λ
	char check = 0;
	//��ȡ���鳤��
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
	//����ָ���ȴ�Ӧ��ʱ��
	clock_t startTime = clock();
	while (1)
	{
		//��ȡӦ��ʱ��
		clock_t stopTime = clock();
		//��Ӧ��״̬Ϊָ��Ӧ�����״̬Ӧ��ʱ
		if (nowState == state)
		{
			//��ʼ��Ӧ��
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
		//��������
		if (front > 1024 * 1000)
		{
			//���û�����
			memset(buffer, 0, sizeof(unsigned char) * 1024 * 1024);
			front = 0;
			continue;
		}
		//����ָ���������
		praseCommand(buffer);
/*		int i = 0;
		while (buffer[i] != '\0') {
			cout << buffer[i];
			i++;
		}*/
		//Sleep(100);
	}
}

//�������е��ַ���תΪ16����
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
		//�������ܿ��ش�ָ��
//		p.send("7B A8 4C 4D 53 59 A3 7D");
		p.lazerSwitch('Y');
		break;
	case 'X':
	case 'x':
		//�������ܿ��عر�ָ��
//		p.send("7B A8 4C 4D 53 4E B4 7D");
//		p.sendEnd();
		break;
	case 'A':
	case 'a':
		//������ͨ���ܹر�ָ��
		//p.send("7B B1 43 53 43 4E 4E 4E 4E 4E 4E 4E 4E 4E 4E E2 7D");
		p.channelEnd();
		cout << "������ͨ��ȫ���ر�" << endl;
		flag1 = flag2 = flag3 = false;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'S':
	case 's':
		//�򿪼�����ͨ��1ָ��
		//p.send("7B B1 43 53 43 59 59 59 59 59 59 59 59 59 59 E2 7D");
		p.channel1open();
		flag1 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'D':
	case 'd':
		//�򿪼�����ͨ��2ָ��
		//p.send("7B B1 43 53 43 4E 59 4E 4E 4E 4E 4E 4E 4E 4E F5 7D");
//		p.channel2open();
		flag2 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'F':
	case 'f':
		//�򿪼�����ͨ��3ָ��
		//p.send("7B B1 43 53 43 4E 59 4E 4E 4E 4E 4E 4E 4E 4E F5 7D");
		p.channel3open();
		flag3 = true;
		ShowChannel(flag1, flag2, flag3);
		break;
	case 'C':
	case 'c':
		ShowChannel(flag1, flag2, flag3);
		/*int channel, luminance;
		cout << "����ѡ��ļ���ͨ����1��2��3��:";
		cin >> channel;
		cout << endl;
		cout << "���뼤��ռ�ձȣ�";
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

//���͵���ָ��
//���巽���Զ�����ָ��У��λ	��������	^
string CheckDigit (const int channel, const int luminance) {
	string ret = "7B ";
	vector<int> a = { 0xA9,0x43,0x44,0x43 };	//����λ����
	int check = a[0] ^ a[1];	//У��λ
	a.push_back(channel);
	a.push_back(luminance);
	for (int i = 0; i < a.size(); i++) {	//forѭ������У��λ
		if (i >= 2) {
			check = check ^ a[i];
		}
		stringstream ss;
		/*
		hex��ָ������16����
		uppercase��ָ��ʮ���������ֺͿ�ѧ�������е�ָ���Դ�д��ʽ��ʾ
		setw��ָ��������һ��Ԫ�ص���ʾ�ֶεĿ��	iomanip
		setfill��������������Ҷ�����ʾ�еĿո��ַ�	iomanip
		*/
		ss << hex << uppercase << setw(2) << setfill('0') << a[i];
		//�ַ���ƴ��
		ret = ret + ss.str() + " ";
	}
	stringstream ss;
	//У��λת��Ϊ16����ĩβƴ��
	ss << hex << check;
	ret += ss.str() + " " + "7D";
	//��Сд��ĸת��Ϊ��д��ĸ
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
				//��ǰbuf[head] == 0x7B
				//CAN
				if (buf[head+2] == 0x43 && buf[head+3] == 0x41 && buf[head+4] == 0x4E) {
					nowState = COMMAND_ANSWER_STATE;
					if (buf[head + 5] == 0x00)	cout << "ָ��Ӧ�������ʽ��ȷ" << endl;
					else if (buf[head + 5] == 0x01)	cout << "ָ��Ӧ�������ʽ����" << endl;
					else if (buf[head + 5] == 0x02)	cout << "ָ��Ӧ������У��ʹ���" << endl;
				}
				//CFG
				else if (buf[head + 2] == 0x43 && buf[head + 3] == 0x46 && buf[head + 4] == 0x47) {
					nowState = CONFIGURATION_PARAMETER_STATE;
					float hardwareVersion = buf[head + 5] * 0.1;
					float softwareVersion = buf[head + 6] * 0.1;
					//ռ�ձ����
					//pass
					cout << "���ò���Ӧ��" << "����ع����" << buf[head + 25] << "Ӳ���汾��V" << hardwareVersion << "������汾��V" << softwareVersion << endl;
				}
				//FWV
				else if (buf[head + 2] == 0x46 && buf[head + 3] == 0x57 && buf[head + 4] == 0x56) {
					nowState = FIRMWARE_VERSION_STATE;
					float hardwareVersion = buf[head + 5] * 0.1;
					float softwareVersion = buf[head + 6] * 0.1;
					cout << "�̼��汾Ӧ��Ӳ���汾��V" << hardwareVersion << "������汾��V" << softwareVersion << endl;
				}
				//SNA
				else if (buf[head + 2] == 0x53 && buf[head + 3] == 0x4E && buf[head + 4] == 0x41) {
					nowState = PRODUCT_NO_STATE;
					string SNA;
					//VXSED
					if (buf[head + 5] == 0x56 && buf[head + 6] == 0x58 && buf[head + 7] == 0x53 && buf[head + 8] == 0x45 && buf[head + 9] == 0x44) {
						SNA = "���ֶ�λ����豸,";
					}
					//VPSSS
					if (buf[head + 5] == 0x56 && buf[head + 6] == 0x50 && buf[head + 7] == 0x53 && buf[head + 8] == 0x53 && buf[head + 9] == 0x53) {
						SNA = "��ü�߶Ȳ����豸,";
					}
					cout << "��Ʒ���к�Ӧ��" << SNA << "�������ڣ�" << buf[head + 10] << buf[head + 11] << buf[head + 12] << buf[head + 13] << "��" << buf[head + 14] << buf[head + 15] << "�£�" << "���кţ�" << buf[head + 16] << buf[head + 17] << buf[head + 18] << buf[head + 19] << endl;
				}
				//RDY
				else if (buf[head + 2] == 0x52 && buf[head + 3] == 0x44 && buf[head + 4] == 0x59) {
					nowState = BE_READY_STATE;
					cout << "׼������Ӧ��" << endl;
				}
				//TPV
				else if (buf[head + 2] == 0x54 && buf[head + 3] == 0x50 && buf[head + 4] == 0x56) {
					nowState = RETURN_TEMPERATURE_STATE;
					float temperature = (buf[head + 5] * 256 + buf[head + 6]) * 0.01;
					cout << "�¶ȷ���Ӧ�𣬵�ǰ�¶�" << temperature << "��" << endl;
				}
				else {
					cout << "��λ��Ӧ�����" << endl;
				}
				front -= tail+1;
				::memcpy(buf, buf + tail+1, front);
			}
			else {
				//���Ҳ���֡β
				break;
			}
		}
		else {
			//���Ҳ���֡ͷ
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
	//����-1ʱ��ʾ��ǰ��������ݲ��Ҳ���֡ͷ
	return -1;
}

int SerialPort::findTail(unsigned char *buf) {
	//�Ӳ��ҵ���֡ͷ��ʼ��������������֡β
	int head = findHead(buf);
	for (int i = head+1; i < front; i++) {
		if (buf[i] == 0x7D) {
			return i;
		}
	}
	return -1;
}

