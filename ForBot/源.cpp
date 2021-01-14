#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <math.h>
#include <ctime>
#include "jsoncpp/json.h"
using namespace std;

#define TIME_CALCULATE 600000	//计算时长
//注意电脑上单位是ms, botzone上单位是us

#define UCB1		  1		//UCB1 公式中的经验常数

//指定模拟深度的公式
#define dph(a)		  6


//-----------------以下是规则代码---------------

//棋盘的大小为 9 * 9 ,定义11原因是留出棋盘外围，方便使用（数组不越界）
#define BOARD_SIZE 11
//最长对局 81 手
#define MAX_STEPS 81

//----------------------用于数气的全局变量---------------------
bool visited[BOARD_SIZE][BOARD_SIZE];							        //数气时该点是否已经访问过
//注意：每次使用前都要使用以下函数刷新该变量
void InitVisited()
{
	memset(visited, false, sizeof(visited));
}

#define		MAX_DIR			4											//共四个方向
const int	cx[] = { -1,0,1,0 };										// x，y 方向的位移
const int	cy[] = { 0,-1,0,1 };

//棋子颜色
typedef bool COLOR;
#define		C_BLACK			true
#define		C_WHITE			false

//棋盘坐标
typedef short int COORDINATE;
#define		GetX(a)			((a) / BOARD_SIZE)					//获取 x 坐标
#define		GetY(a)			((a) % BOARD_SIZE)					//获取 y 坐标
#define		GetPt(a, b)		(((a) * BOARD_SIZE) + (b))			//组合坐标

//落子状态
typedef char POINTINFO;
//下面表示该点已经落子
#define		BLACK			0b0001
#define		WHITE			0b0010
//下面表示该点尚未落子
#define		AVAILABLE		0b0000		
//下面表示棋盘外的点
#define     OUTSIDE			0b0011

//棋局状态
struct BOARDDATA {
	POINTINFO PointInfo[BOARD_SIZE][BOARD_SIZE];

	short int rounds;							//当前手数

	COLOR whoseTurn;							//当前轮到何方
};

class Game {
public:
	//获取当前手数
	short int getRounds()const
	{
		return BoardData.rounds;
	}

	//获取当前轮到的棋手
	COLOR getWhoseTurn()const
	{
		return BoardData.whoseTurn;
	}

	//获取棋盘上某一点的落子状态
	//参数：pt 该点坐标
	void getPointInfo(COORDINATE pt, POINTINFO* pPtInfo)const
	{
		*pPtInfo = BoardData.PointInfo[GetX(pt)][GetY(pt)];
	}

	//获取当前局面的可行棋步
	//返回值：可行棋步的数目
	short int getLegalStep(COORDINATE* legalSteps)
	{
		short int cnt = 0;
		for (int i = 1; i < BOARD_SIZE - 1; i++)
		{
			for (int j = 1; j < BOARD_SIZE - 1; j++)
			{
				if (CheckStep(GetPt(i, j), getWhoseTurn()))
				{
					legalSteps[cnt] = GetPt(i, j);
					cnt++;
				}
			}
		}
		return cnt;
	}

	//返回某一方的非法棋步数目
	short int getIllegalStep(COLOR col)
	{
		short int cnt = 0;
		for (int i = 1; i < BOARD_SIZE - 1; i++)
		{
			for (int j = 1; j < BOARD_SIZE - 1; j++)
			{
				if (BoardData.PointInfo[i][j] == AVAILABLE && !CheckStep(GetPt(i, j), col))
				{
					cnt++;
				}
			}
		}
		return cnt;
	}

	//棋盘状态初始化
	void InitBoard()
	{
		BoardData.rounds = 0;
		BoardData.whoseTurn = BLACK;

		//设置棋盘内部的点为 AVAILABLE
		for (int i = 1; i < BOARD_SIZE - 1; i++)
		{
			for (int j = 1; j < BOARD_SIZE - 1; j++)
			{
				BoardData.PointInfo[i][j] = AVAILABLE;
			}
		}

		//设置棋盘以外的点为 OUTSIDE
		for (int i = 0; i < BOARD_SIZE; i++)
		{
			BoardData.PointInfo[i][0] = OUTSIDE;
			BoardData.PointInfo[i][BOARD_SIZE - 1] = OUTSIDE;
		}
		for (int j = 1; j < BOARD_SIZE - 1; j++)
		{
			BoardData.PointInfo[0][j] = OUTSIDE;
			BoardData.PointInfo[BOARD_SIZE - 1][j] = OUTSIDE;
		}
	}

	//根据历史棋步设置棋盘状态
	//参数：stepsInfo 表示历史棋步坐标的有序数组的指针，rounds 已进行的手数 
	void SetBoard(COORDINATE* stepsInfo, short int rounds)
	{
		BoardData.rounds = rounds;
		BoardData.whoseTurn = (rounds % 2) ? C_WHITE : C_BLACK;

		for (int i = 0; i < rounds; i++)
		{
			int x = GetX(stepsInfo[i]);
			int y = GetY(stepsInfo[i]);
			BoardData.PointInfo[x][y] = (i % 2) ? WHITE : BLACK;
		}
	}

	//落下一手棋
	//返回值：是否为有效位置有效
	bool Drop(COORDINATE pt)
	{
		if (CheckStep(pt, BoardData.whoseTurn))
		{
			//修改手数
			BoardData.rounds++;

			//修改棋局信息
			BoardData.PointInfo[GetX(pt)][GetY(pt)] = (BoardData.whoseTurn ? BLACK : WHITE);
			BoardData.whoseTurn = !BoardData.whoseTurn;

			return true;
		}
		else
			return false;
	}

	//直接下棋而不检验（仅限用于simulation）
	void put(COORDINATE pt)
	{
		//修改手数
		BoardData.rounds++;

		//修改棋局信息
		BoardData.PointInfo[GetX(pt)][GetY(pt)] = (BoardData.whoseTurn ? BLACK : WHITE);
		BoardData.whoseTurn = !BoardData.whoseTurn;
	}

	//检查该棋子是否有气
	//采用递归方法
	bool HasAir(COORDINATE pt)
	{
		visited[GetX(pt)][GetY(pt)] = true;
		bool flag = false;

		COORDINATE ptNext;
		POINTINFO ptInfo = BoardData.PointInfo[GetX(pt)][GetY(pt)];
		for (int dir = 0; dir < MAX_DIR; dir++)
		{
			ptNext = GetPt(GetX(pt) + cx[dir], GetY(pt) + cy[dir]);
			POINTINFO ptInfoNext = BoardData.PointInfo[GetX(ptNext)][GetY(ptNext)];
			if (ptInfoNext != OUTSIDE)
			{
				if (ptInfoNext == AVAILABLE)
					flag = true;
				else if (ptInfoNext == ptInfo && !visited[GetX(ptNext)][GetY(ptNext)])
					if (HasAir(ptNext))
						flag = true;
			}
		}

		return flag;
	}

	//检查该棋步是否合法
	bool CheckStep(COORDINATE pt, COLOR color)
	{
		POINTINFO ptInfo = BoardData.PointInfo[GetX(pt)][GetY(pt)];

		if (ptInfo == AVAILABLE)
		{
			BoardData.PointInfo[GetX(pt)][GetY(pt)] = (color ? BLACK : WHITE);

			InitVisited();

			if (!HasAir(pt))
			{
				BoardData.PointInfo[GetX(pt)][GetY(pt)] = ptInfo;
				return false;
			}
			else
				for (int dir = 0; dir < MAX_DIR; dir++)
				{
					POINTINFO ptInfoNext = BoardData.PointInfo[GetX(pt) + cx[dir]][GetY(pt) + cy[dir]];
					if (ptInfoNext != OUTSIDE)
					{
						if ((ptInfoNext == BLACK || ptInfoNext == WHITE) && !visited[GetX(pt) + cx[dir]][GetY(pt) + cy[dir]])
							if (!HasAir(GetPt(GetX(pt) + cx[dir], GetY(pt) + cy[dir])))
							{
								BoardData.PointInfo[GetX(pt)][GetY(pt)] = ptInfo;
								return false;
							}
					}
				}

			BoardData.PointInfo[GetX(pt)][GetY(pt)] = ptInfo;
			return true;
		}
		else
			return false;
	}

	//判断游戏是否已经结束并给出获胜方（若有）
	bool EndGame(COLOR* color)
	{
		COLOR col = BoardData.whoseTurn;

		for (int i = 1; i < BOARD_SIZE - 1; i++)
		{
			for (int j = 1; j < BOARD_SIZE - 1; j++)
			{
				if (BoardData.PointInfo[i][j] == AVAILABLE && CheckStep(GetPt(i, j), col))
					return false;
			}
		}

		*color = !col;
		return true;
	}
private:
	BOARDDATA   BoardData;								//棋盘状态
};

//-------------------以下是策略代码----------------------
//-----------------当前采用简单的MCTS--------------------

//输出的计算结果
struct OutPut {
	COORDINATE point;
	int num;
	double winrate;
};

class TreeNode {
public:
	//构造函数：建立新的树节点
	inline explicit TreeNode(Game game, TreeNode* parentNode)
	{
		state = game;
		isFullyExpanded = false;
		isMyAction = parentNode ? (!parentNode->isMyAction) : false;
		parent = parentNode;
		numVisited = 0;
		totalReward = 0;
		numChild = 0;
		move = 0;
		for (int i = 0; i < MAX_STEPS; i++)
			children[i] = NULL;
	}

	//指定该节点落子坐标
	inline void setMove(COORDINATE pt)
	{
		move = pt;
	}

	//主函数：选择最佳子节点
	//返回最佳子节点的落子坐标
	//对象：根节点
	//参数：指定的计算时间
	OutPut getBestAction(double time)
	{
		OutPut output;

		expand();

		//指定模拟深度
		int depth = (int)(dph(state.getRounds()));

		double start = clock();
		int n;
		for (n = 0; clock() - start < time; n++)//clock() - start < time
		{
			TreeNode* leaf = select();
			if (leaf->numVisited)
			{
				leaf->expand();
				if (leaf->numChild)
					leaf = leaf->select();
			}
			bool win = (leaf->simulate(depth) == state.getWhoseTurn());
			leaf->backPropagate(win);
		}
		TreeNode* node = getMostChild();

		output.point = node->move;
		output.num = n;
		output.winrate = node->totalReward / node->numVisited;

		deleteNode();

		return output;
	}

private:
	Game state;
	bool isMyAction;
	bool isFullyExpanded;
	TreeNode* parent;
	COORDINATE move;
	int numVisited;
	double totalReward;
	short int numChild;
	TreeNode* children[MAX_STEPS];

	//返回访问次数最多的子节点
	//对象：父节点
	inline TreeNode* getMostChild()const
	{
		int max = 0;
		int cnt = 0;//随机选择的计数器
		TreeNode* mostChild = NULL;
		for (int i = 0; i < numChild; i++)
		{
			TreeNode* child = children[i];
			if (child->numVisited > max)
			{
				max = child->numVisited;
				mostChild = child;
			}
			else if (child->numVisited == max)
			{
				if (!(rand() % (cnt + 2)))
				{
					max = child->numVisited;
					mostChild = child;
					cnt++;
				}
			}
		}
		return mostChild;
	}

	//返回UCB值最大的子节点
	//对象：父节点
	inline TreeNode* getBestChild()const
	{
		double maxValue = 0;
		int cnt = 0;//随机选择的计数器
		TreeNode* bestChild = children[0];
		for (int i = 0; i < numChild; i++)
		{
			TreeNode* child = children[i];

			double value;
			if (child->numVisited)
				value = child->totalReward / child->numVisited
				+ UCB1 * sqrt(2 * log(numVisited) / child->numVisited);
			else
				value = 0xffffffff;

			if (value - maxValue > 1e-6)
			{
				maxValue = value;
				bestChild = child;
			}
			else if (abs(value - maxValue) <= 1e-6)
			{//若 UCB 值相等则随机选择
				if (!(rand() % (cnt + 2)))
				{
					maxValue = value;
					bestChild = child;
					cnt++;
				}
			}

		}
		return bestChild;
	}

	//选择叶子节点
	//对象：根节点
	inline TreeNode* select()
	{
		TreeNode* bestLeaf = this;
		while (bestLeaf->numChild)
		{
			bestLeaf = bestLeaf->getBestChild();
		}
		return bestLeaf;
	}

	//扩展叶子节点
	//对象：叶子节点
	inline void expand()
	{
		if (!isFullyExpanded)
		{
			int cnt = 0;

			for (int i = 1; i < BOARD_SIZE - 1; i++)
			{
				for (int j = 1; j < BOARD_SIZE - 1; j++)
				{
					Game game = state;
					if (game.Drop(GetPt(i, j)))
					{
						children[cnt] = new TreeNode(game, this);
						children[cnt]->setMove(GetPt(i, j));
						cnt++;
					}
				}
			}

			numChild = cnt;
			isFullyExpanded = true;
		}
	}

	//对叶子节点进行模拟
	//对象：叶子节点
	//返回值：模拟结果
	//参数：depth 模拟深度
	inline COLOR simulate(int depth)const
	{
		COLOR col;

		Game game = state;
		for (int i = 0; i < depth; i++)
		{
			COORDINATE step[MAX_STEPS];
			short int n = game.getLegalStep(step);
			if (!n)break;
			game.put(step[rand() % n]);
		}

		if (!game.EndGame(&col))
		{
			short int a = game.getIllegalStep(C_BLACK);
			short int b = game.getIllegalStep(C_WHITE);
			if (a > b)return C_WHITE;
			else if (a < b)return C_BLACK;
			else return (rand() % 2) ? C_WHITE : C_BLACK;
		}
		else
			return col;
	}

	//反向传输
	//对象：叶子节点
	//参数：模拟结果
	inline void backPropagate(bool win)
	{
		TreeNode* node = this;
		do {
			node->numVisited++;
			if (win == node->isMyAction)
				node->totalReward++;
			node = node->parent;
		} while (node);
	}

	//清除某一结点以下的所有游戏树
	//采用递归方法
	//对象：根节点
	inline void deleteNode()
	{
		//先清除子节点
		for (int i = 0; i < numChild; i++)
			children[i]->deleteNode();
		//再清除自身
		delete this;
	}
};

//-------------------主函数-------------------
int main()
{
	Game game;
	game.InitBoard();
	COORDINATE stepsInfo[81];
	srand((unsigned)time(0));

	string str;
	int x, y;
	// 读入JSON
	getline(cin, str);
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	// 分析自己收到的输入和自己过往的输出，并恢复状态
	int cnt = 0;
	int turnID = input["responses"].size();
	for (int i = 0; i < turnID; i++)
	{
		x = input["requests"][i]["x"].asInt(), y = input["requests"][i]["y"].asInt();
		if (x != -1)
		{
			stepsInfo[cnt] = GetPt(x + 1, y + 1);
			cnt++;
		}
		x = input["responses"][i]["x"].asInt(), y = input["responses"][i]["y"].asInt();
		if (x != -1)
		{
			stepsInfo[cnt] = GetPt(x + 1, y + 1);
			cnt++;
		}
	}
	x = input["requests"][turnID]["x"].asInt(), y = input["requests"][turnID]["y"].asInt();
	if (x != -1)
	{
		stepsInfo[cnt] = GetPt(x + 1, y + 1);
		cnt++;
	}
	game.SetBoard(stepsInfo, cnt);

	// 输出决策JSON
	Json::Value ret;
	Json::Value action;

	TreeNode* root = new TreeNode(game, NULL);
	OutPut output = root->getBestAction(TIME_CALCULATE);

	action["x"] = GetX(output.point) - 1;
	action["y"] = GetY(output.point) - 1;
	ret["response"] = action;
	char de[100];
	sprintf(de, "模拟次数：%d，预期收益：%.3f", output.num, output.winrate);
	ret["debug"] = de;
	Json::FastWriter writer;

	cout << writer.write(ret) << endl;
	return 0;
}