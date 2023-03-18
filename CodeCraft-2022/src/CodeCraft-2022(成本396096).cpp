#include <iostream>
#include <string>
#include <vector>
//#include <unordered_map>
#include <fstream>
#include <algorithm>
//#include <ctime>
#include <sstream>
//#include <cstdio>
#include <queue>
#include <string.h>
#include <utility>
#include <math.h>

using namespace std;

const int INF = 0x7ffffff;

struct Node {
	string name;
	int bandwidth=0;
};


/*
 * 函数名：         GetIniKeyString
 * 入口参数：       title
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
			if (0 == strncmp(sTitle, sLine, strlen(sLine) - 2)) { // 长度依文件读取的为准  
				flag = 1; // 找到标题位置  
			}
		}
	}
	fclose(fp);
	return NULL;
}

/*
 * 函数名：         GetIniKeyInt
 * 入口参数：       title
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

int BFS(int s, int t, vector<vector<int>>& Adj, vector<int>& pre, vector<int>& flow, int clientNum, int nodeNum, vector<pair<int,int>> &priority) {
	queue<int> q;
	for (int& x : pre) {
		x = -1;
	}
	pre[s] = 0;
	q.push(s);
	flow[s] = INF;
	while (!q.empty()) {
		int x = q.front();
		q.pop();
		if (x == t) break;
		//一次只找一个增广路 
		//遍历客户节点，按照priority_client数组进行遍历
		for (int i = 0; i < clientNum; ++i) {
			if (Adj[x][i] > 0 && pre[i] == -1)
			{
				pre[i] = x;
				flow[i] = min(flow[x], Adj[x][i]);
				q.push(i);
			}
		}

		//遍历边缘节点，按照priority数组进行遍历
		for (int i = 0; i < nodeNum; ++i) {
			int label = priority[i].first + clientNum;//转化为在邻接矩阵中的标号
			if (Adj[x][label] > 0 && pre[label] == -1)
			{
				pre[label] = x;
				flow[label] = min(flow[x], Adj[x][label]);
				q.push(label);
			}
		}

		//遍历起点和终点，正常顺序遍历
		for (int i = clientNum + nodeNum; i < Adj.size(); ++i) {
			if (Adj[x][i] > 0 && pre[i] == -1)
			{
				pre[i] = x;
				flow[i] = min(flow[x], Adj[x][i]);
				q.push(i);
			}
		}
			
	}
	if (pre[t] == -1) return -1;
	else return flow[t];
}

/**
 * 遍历流图和代价图，返回最小代价小的最大流
 */
int SPFA(vector<vector<int>>& cost_net, vector<vector<int>>& AdjacencyMatrix, vector<int>& pre, int clientNum, int nodeNum)
{
	int start = nodeNum + clientNum;
	int end = start + 1;
	queue<int> que;
	vector<int> in_queue(nodeNum + clientNum+2, 0);
	vector<int> dist(nodeNum + clientNum + 2, INF);
	que.push(start);
	dist[start] = 0;
	pre[start] = -1;
	pre[end] = -1;
	//寻找代价最小的路径
	while (!que.empty())
	{
		int temp = que.front();
		que.pop();
		in_queue[temp] = 0;
		for (int i = 0; i < nodeNum + clientNum + 2; i++)
		{
			if (AdjacencyMatrix[temp][i] > 0)
			{
				if (dist[i] > dist[temp] + cost_net[temp][i] )
				{
					dist[i] = dist[temp] + cost_net[temp][i];
					pre[i] = temp;
					if (in_queue[i] == 0) {
						in_queue[i] = 1;
						que.push(i);
					}
				}
			}
		}
	}
	if (dist[end] == INF || pre[end] == -1)
		return 0;
	//找到了代价最短的路径,对流图进行增广
	int flow = INF;
	int front = nodeNum + clientNum + 1; //目标节点
	while (front != -1 && pre[front] != -1)
	{
		flow = min(flow, AdjacencyMatrix[pre[front]][front]);
		front = pre[front];
	}
	front = nodeNum + clientNum + 1;
	while (front != -1 && pre[front] != -1)
	{
		AdjacencyMatrix[pre[front]][front] -= flow;
		AdjacencyMatrix[front][pre[front]] += flow;
		front = pre[front];
	}
	return flow;
}

bool cmp1(const pair<int, int>& a, const pair<int, int>& b) {
	return a.first < b.first;
}

bool cmp2(const pair<int,int>& a, const pair<int,int>& b) {
	return a.second < b.second;
}

int main() {
	ifstream infile("/data/demand.csv", ios::in);
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
		if(*name.rbegin()=='\r'){
			name=name.substr(0,name.size()-1);
		}
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
	ifstream infile2("/data/site_bandwidth.csv", ios::in);
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
	char filename[] = "/data/config.ini";
	int qos_constraint = GetIniKeyInt(title, key, filename);

	//读取客户和节点间的qos关系
	ifstream infile3("/data/qos.csv", ios::in);
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
	//测试输出有多少个时刻
	cout<< "时刻数：" << Time << endl;
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
	//测试输出边缘节点名字和带宽
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

	////记录每个时刻起点选择客户的顺序
	//vector<pair<int, int>> priority_client(clientNum);
	//for (int i = 0; i < clientNum; ++i) {
	//	priority_client[i] = { i,0 };
	//}
	//记录每个时刻客户选择边缘节点的顺序
	vector<pair<int, int>> priority(nodeNum);
	for (int i = 0; i < nodeNum; ++i) {
		priority[i] = { i,nodes[i].bandwidth };//初始化
	}
	sort(priority.begin(), priority.end(), cmp2);

	//将qos小于上限的置为1，否则为0
	for (int i = 0; i < nodeNum; ++i) {
		for (int j = 0; j < clientNum; ++j) {
			qosMatrix[i][j] = (qosMatrix[i][j] < qos_constraint ? 1 : 0);
			//if (qosMatrix[i][j] == 1) {
			//	++priority[i].second;
			//	++priority_client[j].second;
			//}
		}
	}
	//sort(priority_client.begin(), priority_client.end(), cmp1);
	//sort(priority.begin(), priority.end(), cmp2);
	
	
	ofstream outfile;
	outfile.open("/output/solution.txt",ios::out);
	if (!outfile.is_open())
	{
		cout << "写文件失败" << endl;
		return 0;
	}

	//记录每个边缘节点各个时刻所用带宽
	vector<vector<int>> used(nodeNum);
	vector<vector<int>> used_copy(nodeNum);

	////轮流充满0.05Time的时刻
	//int n05 = (int)floor(Time * 0.05);
	//int turn = 0;

	int n95 = (int)ceil(Time * 0.95);//95百分位

	//存储每个时间点的分配方案
	vector<vector<vector<int>>> record(Time, vector<vector<int>>(clientNum,vector<int>(nodeNum,0)));

	for (int i = 0; i < Time; ++i) {
		//构造一个邻接矩阵，包括用户节点(0 - clientNum-1)、边缘节点(clientNum - nodeNum+clientNum-1)、起点(clientNum+nodeNum)、终点(clientNum+nodeNum+1)
		int len = clientNum + nodeNum + 2;//邻接矩阵边长
		vector<vector<int>> AdjacencyMatrix(len, vector<int>(len, 0));

		//预处理邻接矩阵中用户到边缘节点的边
		for (int row = 0; row < clientNum; ++row) {
			for (int col = clientNum; col < nodeNum + clientNum; ++col) {
				if (qosMatrix[col - clientNum][row] == 1) {//如果qos满足，就把这条边的容量设为用户的需求大小
					AdjacencyMatrix[row][col] = demand[i][row];
				}
			}
		}

		//预处理起点到用户的边
		for (int col = 0; col < clientNum; ++col) {
			AdjacencyMatrix[clientNum + nodeNum][col] = demand[i][col];//将起点到每个用户边的容量设为用户需求大小
		}

		//计算当前时刻所有流量平均分给所有边缘节点的流量
		int total = 0;
		for (auto& x : demand[i]) {
			total += x;
		}
		//int average = total / nodeNum + 1;//+1防止小数

		////预处理边缘节点到终点的边
		//for (int row = clientNum; row < nodeNum + clientNum; ++row) {
		//	AdjacencyMatrix[row][clientNum + nodeNum + 1] = min(nodes[row - clientNum].bandwidth,average);//将边缘节点到终点的边的容量设为平均值，若带宽比均值还小，取带宽
		//	//AdjacencyMatrix[row][clientNum + nodeNum + 1] = nodes[row - clientNum].bandwidth;//将边缘节点到终点的边的容量设为边缘节点的带宽
		//}

		//预处理边缘节点到终点的边
		for (int row = 0; row < nodeNum; ++row) {
			int label = priority[row].first + clientNum;
			AdjacencyMatrix[label][clientNum + nodeNum + 1] = 4151;//先把带宽限制为95百分位值
		}

		/*
		//测试输出邻接矩阵
		for (auto& m : AdjacencyMatrix) {
			for (auto& n : m) {
				cout << n << " ";
			}
			cout << endl;
		}
		system("pause");
		*/

		int maxflow = 0;
		int increase = 0;//increase为增广的流量
		vector<int> pre(len), flow(len);//pre为增广路径中每个点的前驱，flow为源点到这个点的流量
		while ((increase = BFS(clientNum + nodeNum, clientNum + nodeNum + 1,AdjacencyMatrix,pre,flow,clientNum,nodeNum,priority)) != -1) {
			int k = clientNum + nodeNum + 1;
			while (k != clientNum + nodeNum )
			{
				int last = pre[k];//从后往前找路径
				AdjacencyMatrix[last][k] -= increase;
				AdjacencyMatrix[k][last] += increase;
				k = last;
			}
			maxflow += increase;
		}

		//判断最大流是否等于总需求
		//若没分配完，则增加带宽（增加至原有带宽）
		if (maxflow < total) {
			//增加边缘节点到终点的边的带宽
			//for (int row = clientNum; row < nodeNum + clientNum; ++row) {
			//	AdjacencyMatrix[row][clientNum + nodeNum + 1] += nodes[row - clientNum].bandwidth>average?nodes[row - clientNum].bandwidth-averag;
			//}
			for (int row = 0; row < nodeNum; ++row) {
				int label = priority[row].first + clientNum;
				AdjacencyMatrix[label][clientNum + nodeNum + 1] += nodes[label-clientNum].bandwidth- 4151;//增加至原有带宽
			}
			vector<int> pre2(len, 0);
			//构造费用网络
			vector<vector<int>> cost_net(len, vector<int>(len, 0));
			//初始化费用网络
			for (int row = 0; row < clientNum; ++row) {
				cost_net[clientNum + nodeNum][row] = 1;
				cost_net[row][clientNum + nodeNum] = -1;
			}
			for (int row = 0; row < clientNum; ++row)
			{
				for (int col = clientNum; col < nodeNum + clientNum; ++col)
				{
					if (qosMatrix[col - clientNum][row])
					{
						cost_net[row][col] = 1;
						cost_net[col][row] = -1;
					}
				}
			}
			for (int n = 0; n < nodeNum;++n) {
				int label = priority[n].first + clientNum;
				cost_net[label][clientNum + nodeNum + 1] = n;
				//cost_net[clientNum + nodeNum + 1][label] = -n;//有负环会死循环
			}
			while ((increase = SPFA(cost_net, AdjacencyMatrix, pre2, clientNum, nodeNum)) != 0) {
				;
			}
		}


		//cout << "时刻" << i << ":" << maxflow << endl;
		
		//将分配方案记录下来
		for (int k = 0; k < clientNum; ++k) {
			for (int j = clientNum; j < nodeNum + clientNum; ++j) {
				if (AdjacencyMatrix[j][k] > 0) {
					record[i][k][j - clientNum] = AdjacencyMatrix[j][k];
				}
			}
		}

		////将方案写入文档
		//for (int k = 0; k < clientNum; ++k) {
		//	outfile << clientName[k] << ":";
		//	int flag = 1;
		//	for (int j = clientNum; j < nodeNum + clientNum; ++j) {
		//		if (AdjacencyMatrix[j][k] > 0) {
		//			if (flag == 0) {
		//				outfile << ",";
		//			}
		//			else {
		//				--flag;
		//			}
		//			outfile << "<" << nodes[j - clientNum].name << "," << AdjacencyMatrix[j][k] << ">" ;
		//		}
		//	}
		//	outfile << endl;
		//}
		

		//将该时刻各边缘节点使用的带宽记录下来
		for (int k = clientNum; k < nodeNum + clientNum; ++k) {
			used[k - clientNum].push_back(AdjacencyMatrix[clientNum + nodeNum + 1][k]);//每个边缘节点带宽使用为邻接矩阵中终点到边缘节点的值
			used_copy[k - clientNum].push_back(AdjacencyMatrix[clientNum + nodeNum + 1][k]);
		}
		
		for (auto& vec : used) {
			sort(vec.begin(), vec.end());
		}

		//将每个边缘节点95百分位带宽值记录下来排序，用来下一时刻每个客户节点选择边缘节点时做参考
		for (auto& node : priority) {
			int label = node.first;//边缘节点的标号
			int cur_n95 = (int)ceil((i + 1) * 0.95);
			if (cur_n95 + 1 <= i) {
				node.second = used[label][cur_n95+1];//把该时刻标号为label的边缘节点的n95带宽存入priority数组用来排序
			}
			else {
				node.second = used[label][i];
			}
			
		}
		sort(priority.begin(), priority.end(), cmp2);//重写回调函数实现对带宽排序

		//++turn;//充满0.05Time时刻后，将边缘节点一部分移到最后
		//if (turn == n05) {
		//	for (int j = 0; j < nodeNum/20; ++j) {
		//		auto it = priority.begin();
		//		pair<int, int> temp = { it->first,it->second };
		//		priority.erase(it);
		//		priority.push_back(temp);
		//	}
		//	turn = 0;
		//}
	}

	//已经分配完毕，下面进行后处理
	//后处理的思想是遍历每个边缘节点，找到它的95百分位的值的时刻，考虑能否将他的一部分流量匀给同时刻别的未满的边缘节点
	//重分配方法为遍历该节点分配了哪些客户节点，寻找这些客户其他能沟通的边缘节点，使其他节点获得这部分流量后不会比其自身的的95百分位值高，也不会超过上限
	//sort(priority.begin(), priority.end(), cmp1);//按标号升序排列
	//for (int i = nodeNum - 1; i >= 0; --i) {
	//	int label = priority[i].first;//先处理95百分位大的边缘节点
	//	for (int t = 0; t < Time; ++t) {
	//		if (used_copy[label][t] == priority[i].second) {//找到95百分位时刻，可能不止一个
	//			for (int client = 0; client < clientNum; ++client) {
	//				if (record[t][client][label] > 0) {
	//					for (int node = 0; node < nodeNum; ++node) {
	//						int flow = record[t][client][label]/100;
	//						int turn = 0;
	//						while (turn <100 && node != label && qosMatrix[node][client] > 0 && used_copy[node][t] > priority[node].second && used_copy[node][t] + flow <= nodes[node].bandwidth) {
	//							used_copy[node][t] += flow;
	//							used_copy[label][t] -= flow;
	//							record[t][client][node] += flow;
	//							record[t][client][label] -= flow;
	//							++turn;
	//							//cout << "到达" << endl;
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}
	
	//for (int i = nodeNum - 1; i >= 0; --i) {
	//	int label = priority[i].first;//先处理95百分位大的边缘节点
	//	int cost = priority[i].second;

	//	for (int t = 0; t < Time; ++t) {
	//		if (used_copy[label][t] == cost) {//找到值为cost的95百分位时刻，可能不止一个
	//			for (int c = 0; c < clientNum;++c) {
	//				for (auto& p : record[t][c]) {
	//					if (p.first == label) {//寻找这个时刻下有哪些客户使用了该节点的流量
	//						for (int n = 0; n < nodeNum; ++n) {
	//							if (qosMatrix[n][c] == 1) {
	//								for (auto& temp : priority) {
	//									if (temp.first == n) {
	//										int thisn95 = temp.second;
	//										if (used_copy[n][t] + p.second < thisn95 && used_copy[n][t] + p.second < nodes[n].bandwidth && p.second>0) {
	//											used_copy[n][t] += p.second;
	//											used_copy[label][t] -= p.second;
	//											int find = 0;
	//											for (auto& pp : record[t][c]) {
	//												if (pp.first == n) {
	//													pp.second += p.second;
	//													find = 1;
	//												}
	//											}
	//											if (find == 0) {
	//												p.first = n;
	//											}
	//										}
	//									}
	//								}
	//							}
	//						}
	//					}
	//				}
	//			}
	//		}
	//	}
	//}

	for (int t = 0; t < Time; ++t) {
		for (int k = 0; k < clientNum; ++k) {
			outfile << clientName[k] << ":";
			int flag = 1;
			for (int j = 0; j < nodeNum; ++j) {
				if (record[t][k][j] > 0) {
					if (flag == 0) {
						outfile << ",";
					}
					else {
						--flag;
					}
					outfile << "<" << nodes[j].name << "," << record[t][k][j] << ">";
				}

			}
			outfile << endl;
		}
	}


	outfile.close();

	
	////计算95百分位成本
	//for (auto& vec : used_copy) {
	//	sort(vec.begin(), vec.end());
	//	for (auto& x : vec) {
	//		cout << x << ", ";
	//	}
	//	cout << endl;
	//}
	//int costs = 0;//总成本
	//for (auto& vec : used_copy) {
	//	costs += vec[n95-1];
	//}
	//cout << costs << endl;
	

	return 0;
}
