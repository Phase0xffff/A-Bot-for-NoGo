#include <iostream>
#include <cstdio>
#include <cstring>
#include <string>
#include <cmath>
#include <math.h>
#include <ctime>
#include "jsoncpp/json.h"
using namespace std;

#define TIME_CALCULATE 600000	//����ʱ��
//ע������ϵ�λ��ms, botzone�ϵ�λ��us

#define UCB1		  1		//UCB1 ��ʽ�еľ��鳣��

//ָ��ģ����ȵĹ�ʽ
#define dph(a)		  6


//-----------------�����ǹ������---------------

//���̵Ĵ�СΪ 9 * 9 ,����11ԭ��������������Χ������ʹ�ã����鲻Խ�磩
#define BOARD_SIZE 11
//��Ծ� 81 ��
#define MAX_STEPS 81

//----------------------����������ȫ�ֱ���---------------------
bool visited[BOARD_SIZE][BOARD_SIZE];							        //����ʱ�õ��Ƿ��Ѿ����ʹ�
//ע�⣺ÿ��ʹ��ǰ��Ҫʹ�����º���ˢ�¸ñ���
void InitVisited()
{
	memset(visited, false, sizeof(visited));
}

#define		MAX_DIR			4											//���ĸ�����
const int	cx[] = { -1,0,1,0 };										// x��y �����λ��
const int	cy[] = { 0,-1,0,1 };

//������ɫ
typedef bool COLOR;
#define		C_BLACK			true
#define		C_WHITE			false

//��������
typedef short int COORDINATE;
#define		GetX(a)			((a) / BOARD_SIZE)					//��ȡ x ����
#define		GetY(a)			((a) % BOARD_SIZE)					//��ȡ y ����
#define		GetPt(a, b)		(((a) * BOARD_SIZE) + (b))			//�������

//����״̬
typedef char POINTINFO;
//�����ʾ�õ��Ѿ�����
#define		BLACK			0b0001
#define		WHITE			0b0010
//�����ʾ�õ���δ����
#define		AVAILABLE		0b0000		
//�����ʾ������ĵ�
#define     OUTSIDE			0b0011

//���״̬
struct BOARDDATA {
	POINTINFO PointInfo[BOARD_SIZE][BOARD_SIZE];

	short int rounds;							//��ǰ����

	COLOR whoseTurn;							//��ǰ�ֵ��η�
};

class Game {
public:
	//��ȡ��ǰ����
	short int getRounds()const
	{
		return BoardData.rounds;
	}

	//��ȡ��ǰ�ֵ�������
	COLOR getWhoseTurn()const
	{
		return BoardData.whoseTurn;
	}

	//��ȡ������ĳһ�������״̬
	//������pt �õ�����
	void getPointInfo(COORDINATE pt, POINTINFO* pPtInfo)const
	{
		*pPtInfo = BoardData.PointInfo[GetX(pt)][GetY(pt)];
	}

	//��ȡ��ǰ����Ŀ����岽
	//����ֵ�������岽����Ŀ
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

	//����ĳһ���ķǷ��岽��Ŀ
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

	//����״̬��ʼ��
	void InitBoard()
	{
		BoardData.rounds = 0;
		BoardData.whoseTurn = BLACK;

		//���������ڲ��ĵ�Ϊ AVAILABLE
		for (int i = 1; i < BOARD_SIZE - 1; i++)
		{
			for (int j = 1; j < BOARD_SIZE - 1; j++)
			{
				BoardData.PointInfo[i][j] = AVAILABLE;
			}
		}

		//������������ĵ�Ϊ OUTSIDE
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

	//������ʷ�岽��������״̬
	//������stepsInfo ��ʾ��ʷ�岽��������������ָ�룬rounds �ѽ��е����� 
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

	//����һ����
	//����ֵ���Ƿ�Ϊ��Чλ����Ч
	bool Drop(COORDINATE pt)
	{
		if (CheckStep(pt, BoardData.whoseTurn))
		{
			//�޸�����
			BoardData.rounds++;

			//�޸������Ϣ
			BoardData.PointInfo[GetX(pt)][GetY(pt)] = (BoardData.whoseTurn ? BLACK : WHITE);
			BoardData.whoseTurn = !BoardData.whoseTurn;

			return true;
		}
		else
			return false;
	}

	//ֱ������������飨��������simulation��
	void put(COORDINATE pt)
	{
		//�޸�����
		BoardData.rounds++;

		//�޸������Ϣ
		BoardData.PointInfo[GetX(pt)][GetY(pt)] = (BoardData.whoseTurn ? BLACK : WHITE);
		BoardData.whoseTurn = !BoardData.whoseTurn;
	}

	//���������Ƿ�����
	//���õݹ鷽��
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

	//�����岽�Ƿ�Ϸ�
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

	//�ж���Ϸ�Ƿ��Ѿ�������������ʤ�������У�
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
	BOARDDATA   BoardData;								//����״̬
};

//-------------------�����ǲ��Դ���----------------------
//-----------------��ǰ���ü򵥵�MCTS--------------------

//����ļ�����
struct OutPut {
	COORDINATE point;
	int num;
	double winrate;
};

class TreeNode {
public:
	//���캯���������µ����ڵ�
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

	//ָ���ýڵ���������
	inline void setMove(COORDINATE pt)
	{
		move = pt;
	}

	//��������ѡ������ӽڵ�
	//��������ӽڵ����������
	//���󣺸��ڵ�
	//������ָ���ļ���ʱ��
	OutPut getBestAction(double time)
	{
		OutPut output;

		expand();

		//ָ��ģ�����
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

	//���ط��ʴ��������ӽڵ�
	//���󣺸��ڵ�
	inline TreeNode* getMostChild()const
	{
		int max = 0;
		int cnt = 0;//���ѡ��ļ�����
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

	//����UCBֵ�����ӽڵ�
	//���󣺸��ڵ�
	inline TreeNode* getBestChild()const
	{
		double maxValue = 0;
		int cnt = 0;//���ѡ��ļ�����
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
			{//�� UCB ֵ��������ѡ��
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

	//ѡ��Ҷ�ӽڵ�
	//���󣺸��ڵ�
	inline TreeNode* select()
	{
		TreeNode* bestLeaf = this;
		while (bestLeaf->numChild)
		{
			bestLeaf = bestLeaf->getBestChild();
		}
		return bestLeaf;
	}

	//��չҶ�ӽڵ�
	//����Ҷ�ӽڵ�
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

	//��Ҷ�ӽڵ����ģ��
	//����Ҷ�ӽڵ�
	//����ֵ��ģ����
	//������depth ģ�����
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

	//������
	//����Ҷ�ӽڵ�
	//������ģ����
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

	//���ĳһ������µ�������Ϸ��
	//���õݹ鷽��
	//���󣺸��ڵ�
	inline void deleteNode()
	{
		//������ӽڵ�
		for (int i = 0; i < numChild; i++)
			children[i]->deleteNode();
		//���������
		delete this;
	}
};

//-------------------������-------------------
int main()
{
	Game game;
	game.InitBoard();
	COORDINATE stepsInfo[81];
	srand((unsigned)time(0));

	string str;
	int x, y;
	// ����JSON
	getline(cin, str);
	Json::Reader reader;
	Json::Value input;
	reader.parse(str, input);

	// �����Լ��յ���������Լ���������������ָ�״̬
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

	// �������JSON
	Json::Value ret;
	Json::Value action;

	TreeNode* root = new TreeNode(game, NULL);
	OutPut output = root->getBestAction(TIME_CALCULATE);

	action["x"] = GetX(output.point) - 1;
	action["y"] = GetY(output.point) - 1;
	ret["response"] = action;
	char de[100];
	sprintf(de, "ģ�������%d��Ԥ�����棺%.3f", output.num, output.winrate);
	ret["debug"] = de;
	Json::FastWriter writer;

	cout << writer.write(ret) << endl;
	return 0;
}