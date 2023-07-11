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
//char类型合并
string characterMerge(const vector<char> &chars) {
	string result;
	result.append(chars.begin(), chars.end());
	return result;
}

void praseCommand(vector<string> command) {
	vector<int> instructions;
	//将string类型的返回指令转成int型
	for (const auto&op : command) {
		unsigned int instruction = strtoul(op.c_str(), nullptr, 16);
		instructions.push_back(instruction);
	}
	//检查指令长度	'A0'=160
	int frameLength = instructions[1] - 160;
	if (frameLength != instructions.size()) {
		cout << "返回指令长度错误！" << endl;
	}
	//检查校验位是否正确	懒得写
	vector<char> chars = { static_cast<char>(instructions[2]), static_cast<char>(instructions[3]), static_cast<char>(instructions[4]) };
	string commandWords = characterMerge(chars);
	//返回指令应答命令
	if (commandWords == "CAN") {
		if (instructions[5] == 0) {
			cout <<"返回指令长度:"<<frameLength<< "||CAN指令应答，命令正确" << endl;;
		}
		else if (instructions[5] == 1) {
			cout << "返回指令长度:" << frameLength << "||CAN指令应答，命令格式错误" << endl;;
		}
		else if (instructions[5] == 2) {
			cout << "返回指令长度:" << frameLength << "||CAN指令应答，命令校验和错误" << endl;;
		}
	}
	//返回配置参数应答指令
	if (commandWords == "CFG") {
		if (frameLength != 30) {
			cout << "还没考虑这个情况" << endl;
			return;
		}
		float hardWareVersion = instructions[26] * 0.1;
		float softWareVersion = instructions[27] * 0.1;
		cout << "返回指令长度:" << frameLength << "||CFG指令应答，相机曝光参数:" << instructions[25] << "ms，硬件版本:V" << hardWareVersion << ",软件版本:V" << softWareVersion<<endl;
		for (int i = 6; i <= 25; i++) {
			//当前便是通道
			if (i % 2) {
				cout << "通道" << i / 2 - 2;
			}
			//占空比
			else {
				cout << "的占空比" << instructions[i] << endl;
			}
		}
	}
	//返回设备固件版本号应答
	if (commandWords == "FWV") {
		float hardWareVersion = instructions[5] * 0.1;
		float softWareVersion = instructions[6] * 0.1;
		cout << "返回指令长度:" << frameLength << "||FWV指令应答，硬件版本:V" << hardWareVersion << ",软件版本:V" << softWareVersion << endl;
	}
	//返回产品序列号应答指令
	if (commandWords == "SNA") {
		string SNA;
		vector<char> SNAchars = { static_cast<char>(instructions[5]), static_cast<char>(instructions[6]), static_cast<char>(instructions[7]),static_cast<char>(instructions[8]) };
		string SNAcommandWords = characterMerge(SNAchars);
		//四轮定位检测设备
		if (SNAcommandWords == "VXSED") {
			SNA = "四轮定位检测设备,";
		}
		//轮眉高度测量设备
		else if (SNAcommandWords == "VPSSS") {
			SNA = "轮眉高度测量设备,";
		}
		//获取生产年月
		vector<char> productDateYear = { static_cast<char>(instructions[10]),static_cast<char>(instructions[11]),static_cast<char>(instructions[12]),static_cast<char>(instructions[13]) };
		vector<char> productDateMonth = { static_cast<char>(instructions[14]),static_cast<char>(instructions[15]) };
		string Year = characterMerge(productDateYear);
		string Month = characterMerge(productDateMonth);
		//获取序列号
		vector<char> serialNumber = { static_cast<char>(instructions[16]),static_cast<char>(instructions[17]),static_cast<char>(instructions[18]),static_cast<char>(instructions[19]) };
		string serial_number = characterMerge(serialNumber);
		cout << "返回指令长度:" << frameLength << "||SNA指令应答，" << SNA << "生产于" << Year << "年" << Month << "月," << "序列号:" << serial_number << endl;
	}
	//返回准备就绪指令
	if (commandWords == "RDY") {
		cout << "设备初始化无故障，等待指令输入" << endl;
	}
	//返回温度值返回指令
	if (commandWords == "TPV") {
		float temperature = (instructions[5] + instructions[6]) * 0.01;
		cout << "返回指令长度:" << frameLength << "||TPV指令应答，设备内部温度值：" << temperature << endl;
	}
}*/

/*int main() {

	//获取下位机返回指令
	string command = "7B A8 43 41 4E 00 E4 7D 8A";
	//将下位机指令转存在vector中
	vector<string> Receive_vector = stringToVector(command);
	//创建一个缓冲容器
	vector<string> buffer;
	//遍历接收指令
	for (const auto& byte : Receive_vector) {
		//将指令的每一位存进buffer中
		buffer.push_back(byte);
		//当检查到帧尾时，说明读取到了一个完整的返回指令
		if (byte == "7D") {
			//调用指令解析函数
			praseCommand(buffer);
			//将当前buffer清空
			buffer.clear();
		}
	}
	


	system("pause");
}*/