#include"SerialPort.h"

void main() {
	char portNumber[10];
	printf("输入串口号: ");
	scanf("%s", portNumber);
	SerialPort p;
	if (p.open(portNumber)) {
		cout << "连接成功" << endl;
	}
	else
		cout << "连接失败" << endl;
	while (1) {
		if (p.sendCSC("YYYYYYYYYY"))
			cout << "打开通道1成功" << endl;
		else
			cout << "打开通道1失败" << endl;
		Sleep(100);
		if (p.sendCDC(1, 70))
			cout << "调节占空比成功" << endl;
		else
			cout << "调节占空比失败" << endl;
		Sleep(100);
		if (p.sendLMS('Y'))
			cout << "激光打开指令发送成功" << endl;
		else {
			cout << "下位机激光打开应答超时，检查下位机" << endl;
		}
		Sleep(500);
		if(p.sendLMS('N'))
			cout << "激光关闭指令发送成功" << endl;
		else {
			cout << "下位机激光关闭应答超时，检查下位机" << endl;
		}
		Sleep(10000);
	}
	/*unsigned char data[] = { 0x7B,0xA8,0x4C,0x4D,0x53,0x89};
	for (int i = 0; i < 6; i++) {
		cout << data[i] << " ";
	}*/
	system("pause");
}