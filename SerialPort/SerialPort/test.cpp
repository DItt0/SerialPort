#include<iostream>
#include<string>
#include<vector>
#include<sstream>

using namespace std;


/*vector<string> stringToVector(const string& command) {
	istringstream ss(command);
	vector<string> operators;
	string op;
	while (ss >> op) {
		operators.push_back(op);
	}
	return operators;
}
//char���ͺϲ�
string characterMerge(const vector<char> &chars) {
	string result;
	result.append(chars.begin(), chars.end());
	return result;
}

void praseCommand(vector<string> command) {
	vector<int> instructions;
	//��string���͵ķ���ָ��ת��int��
	for (const auto&op : command) {
		unsigned int instruction = strtoul(op.c_str(), nullptr, 16);
		instructions.push_back(instruction);
	}
	//���ָ���	'A0'=160
	int frameLength = instructions[1] - 160;
	if (frameLength != instructions.size()) {
		cout << "����ָ��ȴ���" << endl;
	}
	//���У��λ�Ƿ���ȷ	����д
	vector<char> chars = { static_cast<char>(instructions[2]), static_cast<char>(instructions[3]), static_cast<char>(instructions[4]) };
	string commandWords = characterMerge(chars);
	//����ָ��Ӧ������
	if (commandWords == "CAN") {
		if (instructions[5] == 0) {
			cout <<"����ָ���:"<<frameLength<< "||CANָ��Ӧ��������ȷ" << endl;;
		}
		else if (instructions[5] == 1) {
			cout << "����ָ���:" << frameLength << "||CANָ��Ӧ�������ʽ����" << endl;;
		}
		else if (instructions[5] == 2) {
			cout << "����ָ���:" << frameLength << "||CANָ��Ӧ������У��ʹ���" << endl;;
		}
	}
	//�������ò���Ӧ��ָ��
	if (commandWords == "CFG") {
		if (frameLength != 30) {
			cout << "��û����������" << endl;
			return;
		}
		float hardWareVersion = instructions[26] * 0.1;
		float softWareVersion = instructions[27] * 0.1;
		cout << "����ָ���:" << frameLength << "||CFGָ��Ӧ������ع����:" << instructions[25] << "ms��Ӳ���汾:V" << hardWareVersion << ",����汾:V" << softWareVersion<<endl;
		for (int i = 6; i <= 25; i++) {
			//��ǰ����ͨ��
			if (i % 2) {
				cout << "ͨ��" << i / 2 - 2;
			}
			//ռ�ձ�
			else {
				cout << "��ռ�ձ�" << instructions[i] << endl;
			}
		}
	}
	//�����豸�̼��汾��Ӧ��
	if (commandWords == "FWV") {
		float hardWareVersion = instructions[5] * 0.1;
		float softWareVersion = instructions[6] * 0.1;
		cout << "����ָ���:" << frameLength << "||FWVָ��Ӧ��Ӳ���汾:V" << hardWareVersion << ",����汾:V" << softWareVersion << endl;
	}
	//���ز�Ʒ���к�Ӧ��ָ��
	if (commandWords == "SNA") {
		string SNA;
		vector<char> SNAchars = { static_cast<char>(instructions[5]), static_cast<char>(instructions[6]), static_cast<char>(instructions[7]),static_cast<char>(instructions[8]) };
		string SNAcommandWords = characterMerge(SNAchars);
		//���ֶ�λ����豸
		if (SNAcommandWords == "VXSED") {
			SNA = "���ֶ�λ����豸,";
		}
		//��ü�߶Ȳ����豸
		else if (SNAcommandWords == "VPSSS") {
			SNA = "��ü�߶Ȳ����豸,";
		}
		//��ȡ��������
		vector<char> productDateYear = { static_cast<char>(instructions[10]),static_cast<char>(instructions[11]),static_cast<char>(instructions[12]),static_cast<char>(instructions[13]) };
		vector<char> productDateMonth = { static_cast<char>(instructions[14]),static_cast<char>(instructions[15]) };
		string Year = characterMerge(productDateYear);
		string Month = characterMerge(productDateMonth);
		//��ȡ���к�
		vector<char> serialNumber = { static_cast<char>(instructions[16]),static_cast<char>(instructions[17]),static_cast<char>(instructions[18]),static_cast<char>(instructions[19]) };
		string serial_number = characterMerge(serialNumber);
		cout << "����ָ���:" << frameLength << "||SNAָ��Ӧ��" << SNA << "������" << Year << "��" << Month << "��," << "���к�:" << serial_number << endl;
	}
	//����׼������ָ��
	if (commandWords == "RDY") {
		cout << "�豸��ʼ���޹��ϣ��ȴ�ָ������" << endl;
	}
	//�����¶�ֵ����ָ��
	if (commandWords == "TPV") {
		float temperature = (instructions[5] + instructions[6]) * 0.01;
		cout << "����ָ���:" << frameLength << "||TPVָ��Ӧ���豸�ڲ��¶�ֵ��" << temperature << endl;
	}
}*/

/*int main() {

	//��ȡ��λ������ָ��
	string command = "7B A8 43 41 4E 00 E4 7D 8A";
	//����λ��ָ��ת����vector��
	vector<string> Receive_vector = stringToVector(command);
	//����һ����������
	vector<string> buffer;
	//��������ָ��
	for (const auto& byte : Receive_vector) {
		//��ָ���ÿһλ���buffer��
		buffer.push_back(byte);
		//����鵽֡βʱ��˵����ȡ����һ�������ķ���ָ��
		if (byte == "7D") {
			//����ָ���������
			praseCommand(buffer);
			//����ǰbuffer���
			buffer.clear();
		}
	}
	


	system("pause");
}*/