#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <algorithm>
#include <ctime>
#include <sstream>
#include <stdio.h>
using namespace std;

struct Node {
	string name;
	int bandwidth=0;
};

/*
 * 函数名：         GetIniKeyString
 * 入口参数：        title
 *                      配置文件中一组数据的标识
 *                  key
 *                      这组数据中要读出的值的标识
 *                  filename
 *                      要读取的文件路径
 * 返回值：         找到需要查的值则返回正确结果
 *                  否则返回NULL
 */
char* GetIniKeyString(char* title, char* key, char* filename)
{
	FILE* fp;
	int  flag = 0;
	char sTitle[32], * wTmp;
	static char sLine[1024];

	sprintf(sTitle, "[%s]", title);
	if (NULL == (fp = fopen(filename, "r"))) {
		perror("fopen");
		return NULL;
	}


	while (NULL != fgets(sLine, 1024, fp)) {
		// 这是注释行  
		if (0 == strncmp("//", sLine, 2)) continue;
		if ('#' == sLine[0])              continue;

		wTmp = strchr(sLine, '=');
		if ((NULL != wTmp) && (1 == flag)) {
			if (0 == strncmp(key, sLine, wTmp - sLine)) { // 长度依文件读取的为准  
				sLine[strlen(sLine)] = '\0';
				fclose(fp);
				return wTmp + 1;
			}
		}
		else {
			if (0 == strncmp(sTitle, sLine, strlen(sLine) - 1)) { // 长度依文件读取的为准  
				flag = 1; // 找到标题位置  
			}
		}
	}
	fclose(fp);
	return NULL;
}

/*
 * 函数名：         GetIniKeyInt
 * 入口参数：        title
 *                      配置文件中一组数据的标识
 *                  key
 *                      这组数据中要读出的值的标识
 *                  filename
 *                      要读取的文件路径
 * 返回值：         找到需要查的值则返回正确结果
 *                  否则返回NULL
 */
int GetIniKeyInt(char* title, char* key, char* filename)
{
	return atoi(GetIniKeyString(title, key, filename));
}

int main() {
	ifstream infile("..\\data\\demand.csv", ios::in);
	if (!infile.is_open())
	{
		cout << "读取文件失败1" << endl;
		return 0;
	}
	string line;

	//获取第一行的用户数量和对应的名称
	getline(infile, line);//读第一行获取用户节点名
	istringstream readstr1(line);//string数据流化
	string name;
	int clientNum=0;//记录客户数量
	vector<string> clientName;
	getline(readstr1, name, ',');//去掉第一个字符串"mtime"
	while (getline(readstr1, name, ',')) {//将一行数据按','分割
		clientName.emplace_back(name);
		++clientNum;
	}

	//循环读取每行数据，获取每个时刻各用户的带宽需求
	vector<vector<int>> demand;
	int Time = 0;//记录共有多少时刻数据
	while (getline(infile, line)) {
		vector<int> temp(clientNum);
		string dem;
		istringstream readstr2(line);//string数据流化
		getline(readstr2, dem, ',');//去掉第一个时刻字符
		for (int i = 0; i < clientNum; ++i) {
			getline(readstr2, dem, ',');
			temp[i] = atoi(dem.c_str());//字符串转int
		}
		demand.emplace_back(temp);
		++Time;
	}

	//读取并记录每个节点的名字和带宽
	ifstream infile2("..\\data\\site_bandwidth.csv", ios::in);
	if (!infile2.is_open())
	{
		cout << "读取文件失败2" << endl;
		return 0;
	}
	string line2;
	vector<Node> nodes;//存放每个边缘节点的名字和数量
	int nodeNum=0;//记录节点数量
	getline(infile2, line2);//跳过第一行
	while (getline(infile2, line2)) {
		Node temp;
		string tem;
		istringstream readstr2(line2);
		getline(readstr2, tem, ',');
		temp.name = tem;
		getline(readstr2, tem, ',');
		temp.bandwidth = atoi(tem.c_str());
		nodes.emplace_back(temp);
		++nodeNum;
	}

	//读取qos上限值
	char title[] = "config";
	char key[] = "qos_constraint";
	char filename[] = "..\\data\\config.ini";
	int qos_constraint = GetIniKeyInt(title, key, filename);

	//读取客户和节点间的qos关系
	ifstream infile3("..\\data\\qos.csv", ios::in);
	if (!infile3.is_open())
	{
		cout << "读取文件失败3" << endl;
		return 0;
	}
	vector<vector<int>> qosMatrix;//qos矩阵
	string line3;
	getline(infile3, line3);//跳过第一行
	while (getline(infile3, line3)) {
		vector<int> temp(clientNum);
		string qos;
		istringstream readstr3(line3);
		getline(readstr3, qos, ',');//去掉第一个字符串
		for (int i = 0; i < clientNum; ++i) {
			getline(readstr3, qos, ',');
			temp[i] = atoi(qos.c_str());//字符串转int
		}
		qosMatrix.emplace_back(temp);
	}
	/*
	//测试输出用户数量
	cout << "用户数量："<< clientNum << endl;
	//测试输出用户名（按原表的顺序）
	cout << "用户名" << endl;
	for (string str : clientName) {
		cout << str << " ";
	}
	cout << endl;
	//测试输出用户需求（每行表示一个时刻，每列表示每个客户的需求，与clientName里顺序一致）
	cout << "用户需求" << endl;
	for (auto& tim : demand) {
		for (int dem : tim) {
			cout << dem << " ";
		}
		cout << endl;
	}
	//测试输出节点数量
	cout << "节点数量：" << nodeNum << endl;
	//测试输出边缘节点名字和数量
	for (auto& node : nodes) {
		cout << node.name << " " << node.bandwidth << endl;
	}
	//测试输出qos限制
	cout << "qos限制：" << qos_constraint << endl;
	//测试输出客户节点和边缘节点之间的qos（行为边缘节点，顺序与nodes相同；列为客户，顺序与clientName相同）
	cout << "客户节点和边缘节点之间的qos:" << endl;
	for (auto& qos : qosMatrix) {
		for (int q : qos) {
			cout << q << " ";
		}
		cout << endl;
	}
	*/

	return 0;
}
