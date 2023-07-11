#include "SerialPort.h"


/*int main() {
	SerialPort p;
//	char *channel = new char[10];
	string a;
	cout << "请输入串口号：" ;
	cin >> a;
	if (p.open(a)){
		cout << "串口" <<a<< "打开成功：" << endl;
		cout << "************操作激光菜单************" << endl;
		cout << "\t1.总开关指令输入z打开" << endl;
		cout << "\t2.总开关指令输入x关闭" << endl;
		cout << "\t3.激光通道总关闭输入a" << endl;
		cout << "\t4.激光通道1打开输入s" << endl;
		cout << "\t5.激光通道2打开输入d" << endl;
		cout << "\t6.激光通道3打开输入f" << endl;
		cout << "\t7.激光亮度选择c" << endl;
		cout << "输入其他按键退出激光控制" << endl;
		cout << "输入操作指令：";
		while (1) {
			//显示输出指令菜单
			MenuContrl(p);
//				p.getReceive();
		}
	}
	else {
		cout << "串口打开失败！。。" << endl;
	}

	p.close();
	system("pause");
	return 0;

}

//6.7控制板对指令的答复很慢，程序不能一直等待答复再进行下一步指令输入，
//需要解决控制板返回乱码的问题

/*6.8 控制板返回乱码的问题已解决
当前问题：
1.无法通过指令打开激光器做测试，
2.由于控制板会持续多条的返回造成第一次等待返回过慢，而之后的交互中控制板返回会出现消息堆积
*/