#include"SerialPort.h"

void main() {
	char portNumber[10];
	printf("���봮�ں�: ");
	scanf("%s", portNumber);
	SerialPort p;
	if (p.open(portNumber)) {
		cout << "���ӳɹ�" << endl;
	}
	else
		cout << "����ʧ��" << endl;
	while (1) {
		if (p.sendCSC("YYYYYYYYYY"))
			cout << "��ͨ��1�ɹ�" << endl;
		else
			cout << "��ͨ��1ʧ��" << endl;
		Sleep(100);
		if (p.sendCDC(1, 70))
			cout << "����ռ�ձȳɹ�" << endl;
		else
			cout << "����ռ�ձ�ʧ��" << endl;
		Sleep(100);
		if (p.sendLMS('Y'))
			cout << "�����ָ��ͳɹ�" << endl;
		else {
			cout << "��λ�������Ӧ��ʱ�������λ��" << endl;
		}
		Sleep(500);
		if(p.sendLMS('N'))
			cout << "����ر�ָ��ͳɹ�" << endl;
		else {
			cout << "��λ������ر�Ӧ��ʱ�������λ��" << endl;
		}
		Sleep(10000);
	}
	/*unsigned char data[] = { 0x7B,0xA8,0x4C,0x4D,0x53,0x89};
	for (int i = 0; i < 6; i++) {
		cout << data[i] << " ";
	}*/
	system("pause");
}