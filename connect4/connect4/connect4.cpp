#include<cstdio>
#include<cmath>
#include<stdlib.h>
#include<time.h>
#include<algorithm>
#include<vector>
#include<stack>
using namespace std;
#define INF 1000000000
#define row_n 6
#define col_n 7
#define BOUND 1000000 //search state ũ�Ⱑ BOUND �̸��϶� limit�� �ø�
#define Resource 300000 //MCTS �˰����� �õ� Ƚ��
#define init_depth 8

struct mct_struct { //MCTS state�� ����ü
	int win, n, child_num; //draw�� ��� 1 win�� ��� 2�� ������. n�� �׻� 2�� ����
	bool terminal;
	mct_struct* child[8];
	int col[8];
};
struct uct_struct { //uct�� ����ϱ� ���� ����ü
	int win, n;
};
int c=2; //UCT�� exploration parameter
int t; //MCTS���� turn�� �����ϴ� ����

int state[8][9]; //0:empty . 1:first-player's stone O 2:second-player's stone X
int h[8]; //i��° column�� ���� ������ ���� ����
int bot_turn,rival_turn; //1:first 2:second
int depth_limit; //depth_limit�� ������ ������ ����(game_tree�� ũ��)
int d[4][2]; //����,����,�� �밢���� ������ �����ϴ� ���� 0:���� 1:���� 2,3:�밢��
int search_count; //search state�� ũ�⸦ ����ϱ����� ����
int stone_n; //���� ������ stone�� ������ ������
bool ban[9];
void init() {
	srand((unsigned int)time(NULL)); //MCTS �˰����� ���� ���� ����
	depth_limit = init_depth; //�ʱ��� depth_limit�� ���� ���������� ������ ���� ����
	search_count = BOUND + 1;
	d[0][0] = 1; d[0][1] = 0;
	d[1][0] = 0; d[1][1] = 1;
	d[2][0] = 1; d[2][1] = 1;
	d[3][0] = -1; d[3][1] = 1;

	for (int i = 0; i <= col_n + 1; i++) {
		state[0][i] = -1; state[row_n + 1][i] = -1;
	}
	for (int i = 0; i <= row_n + 1; i++) {
		state[i][0] = -1; state[i][col_n + 1] = -1;
	}

	printf("���� ���� �Է����ּ���(1:first 2:second) : ");
	scanf_s("%d", &bot_turn);
}
void draw() { //���� ���� ��Ȳ�� ������ִ� �Լ�
	system("cls");
	for (int i = row_n; i >= 1; i--) {
		for (int j = 1; j <= col_n; j++) {
			switch (state[i][j]) {
			case 0:
				printf(" . ");
				break;
			case 1:
				printf(" O ");
				break;
			case 2:
				printf(" X ");
				break;
			default:
				printf(" E "); //error
				break;
			}
		}
		printf("\n");
	}
}
int swt(int turn) {
	if (turn == 1) return 2;
	else return 1;
}
bool push_stone(int turn, int col) { //col��° column�� ���� turn�� �÷��̾��� ���� �д�.
	if (h[col] == row_n) return false; //�������� ���� ���� ���������� �װ��� ������ ������ �˷��ش�.
	state[++h[col]][col] = turn; //���� ����

	//state ���� update
	stone_n++;
	return true;
}
int pop_stone(int col) {
	if (h[col] == 0) return 0;
	int ret = state[h[col]][col];
	state[h[col]][col] = 0;
	h[col]--;

	//state ���� update
	stone_n--;
	return ret;
}
bool chk_win(int turn, int col) {
	int ori_x = h[col],ori_y=col;
	for (int i = 0; i < 4; i++) {
		int x = ori_x, y = ori_y;
		int cnt = -1;
		while (state[x][y] == turn) {
			x += d[i][0]; y += d[i][1]; cnt++;
		}
		x = ori_x; y = ori_y;
		while (state[x][y] == turn) {
			x -= d[i][0]; y -= d[i][1]; cnt++;
		}
		if (cnt >= 4) return true;
	}
	return false;
}
int rule_based(int prev_col) {
	//rule 1
	if (prev_col == -1) return 3;
	int i;
	
	//rule 2
	for (i = 1; i <= col_n; i++) {
		if (push_stone(bot_turn, i)) {
			if (chk_win(bot_turn,i)) {
				pop_stone(i);
				return i;
			}
			pop_stone(i);
		}
	}

	//rule 3
	for (i = 1; i <= col_n; i++) {
		if (push_stone(rival_turn, i)) {
			if (chk_win(rival_turn, i)) {
				pop_stone(i);
				return i;
			}
			pop_stone(i);
		}
	}

	//rule 4
	for (i = 1; i <= col_n; i++) {
		ban[i] = false;
		if (push_stone(bot_turn, i)) {
			for (int j = 1; j <= col_n; j++) {
				if (push_stone(rival_turn, i)) {
					if (chk_win(rival_turn, i)) {
						ban[i] = true;
						pop_stone(i);
						break;
					}
					pop_stone(i);
				}
			}
			pop_stone(i);
		}
	}
	//rule 5
	return -1;
}
mct_struct* init_mct() {
	mct_struct* x = (mct_struct *)malloc(sizeof(mct_struct));
	x->n = 0; x->win = 0; x->terminal = false; x->child_num = 0;
	return x;
}
uct_struct simulasion() { //�յ��� Ȯ���� ���� ���� �����ϸ鼭 simulation ���ش�.
	t = swt(t);
	while (1) {
		int col = rand() % col_n + 1;
		uct_struct ret;
		if (push_stone(t, col)) {
			if (chk_win(t, col)) {
				ret.n = 2;
				if (t == bot_turn) ret.win = 2;
				else ret.win = 0;
			}
			else if (stone_n == row_n * col_n) {
				ret.n = 2; ret.win = 1;
			}
			else ret = simulasion();
			pop_stone(col);
			t = swt(t);
			return ret;
		}
	}
}
uct_struct expansion(mct_struct* x) { //selection �� state���� ������ ��� ��츦 expansion �Ͽ��ش�.
	swt(t);
	uct_struct ret;
	ret.win = 0;
	ret.n = 0;
	for (int i = 1; i <= col_n; i++) {
		if (push_stone(t, i)) {
			mct_struct* child=init_mct();
			x->child[x->child_num] = child;
			x->col[x->child_num++] = i;
			child->n = 2;
			if (chk_win(t, i)) {
				child->terminal = true;
				if (t == bot_turn) child->win = 2;
			}
			else if (stone_n == row_n * col_n) {
				child->terminal = true;
				child->win = 1;
			}
			else {
				uct_struct res=simulasion();
				child->win = res.win;
			}
			ret.n += child->n;
			ret.win += child->win;
			pop_stone(i);
		}
	}
	x->n += ret.n;
	x->win += ret.win;
	swt(t);
	return ret;
}
uct_struct selection(mct_struct* x) {
	uct_struct ret;
	if (x->child_num == 0) { //leaf�� �߰��Ѱ��
		if (x->terminal) {
			ret.win = 0; ret.n = 0;
			return ret; //leaf�� terminal�ΰ�� �ٸ� ������ �����ϰ� ���� iteration���� �Ѿ��.
		}
		ret=expansion(x); //���õ� leaf���� expansion �ϰ� ���� expansion�� state���� simulation �Ͽ��־� ������� �����´�.
		return ret;
	}
	//UCT���� �������� ���õ� Ȯ���� ������ �Ͽ� selection
	int uct[7];
	int sum = 0;
	for (int i = 0; i < x->child_num; i++) {
		mct_struct* child = x->child[i];
		double uct_val = (double)child->win / child->n + (double)c*sqrt(log((double)x->n) / child->n); //uct ����
		uct[i]=uct_val*500;
		sum += uct[i];
	}
	int s = rand()%sum+1;
	sum = 0;
	int next=0;
	int col;
	for (int i = 0; i < x->child_num; i++) {
		sum += uct[i];
		if (s <= sum) {
			next = i;
			col = x->col[i];
			break;
		}
	}
	push_stone(t, col);
	t = swt(t);
	ret=selection(x->child[next]);
	x->n += ret.n; x->win += ret.win; //backpropagation�� ������ �ϴ� ����
	pop_stone(col);
	t = swt(t);
	return ret;
}
void free_mct(mct_struct* x) {
	for (int i = 0; i < x->child_num; i++) {
		free_mct(x->child[i]);
	}
	free(x);
}
double pp[8];
int MCTS(int turn) { //MCTS�� ������ ���� �������� �ۼ��Ͽ���.
	mct_struct* root=init_mct();
	int MCTS_cnt = 0; t = turn; //root�� ���� ����
	while (MCTS_cnt++ < Resource) {
		selection(root);
	}
	int sel = 0;
	double max = 0;
	for (int i = 0; i < root->child_num; i++) { //root�� child(�̹��� ������)�� ���� �̱� Ȯ���� ���� ���� ����. exploitation�� �����.
		mct_struct* child = root->child[i];
		if (child->n == 0) continue;
		double val = (double)child->win / child->n;
		pp[root->col[i]] = val;
		if (max < val) {
			max = val;
			sel = root->col[i];
		}
	}
	printf("\n");
	free_mct(root);
	return sel;
}

int huristic1() { //ù��°�� �õ��� huristic function. line�� ��� huristic ���� ����Ѵ�.
	int point = 0;
	for (int ori_y = 1; ori_y <= col_n; ori_y++) {
		for (int ori_x = 1; ori_x <= h[ori_y]; ori_x++) {
			for (int i = 0; i < 4; i++) {
				int x = ori_x, y = ori_y;
				int cnt = -1;
				while (state[x][y] == state[ori_x][ori_y]) {
					x += d[i][0]; y += d[i][1]; cnt++;
				}
				x = ori_x; y = ori_y;
				while (state[x][y] == state[ori_x][ori_y]) {
					x -= d[i][0]; y -= d[i][1]; cnt++;
				}
				if (state[ori_x][ori_y] == bot_turn) point += cnt * cnt;
				else point -= cnt * cnt;
			}
		}
	}
	return point;
}
int huristic2(int turn) {
	int point = 0;
	int bot_line[5][2],rival_line[5][2];
	int max_bot=0,max_rival=0;
	for (int ori_y = 1; ori_y <= col_n; ori_y++) {
		for (int ori_x = h[ori_y]+1; ori_x <= row_n; ori_x++) {
			for (int i = 0; i < 4; i++) {
				//bot�� ���� ���� ��ġ�� �������� ��������� �������� ������ ����Ѵ�.
				int cur_turn = bot_turn;
				int stone=0,side=0,side_stone=0;
				int x = ori_x+d[i][0], y = ori_y+d[i][1];
				while (state[x][y] == cur_turn) {
					x += d[i][0]; y += d[i][1]; stone++;
				}
				while (state[x][y] == 0) {
					x += d[i][0]; y += d[i][1]; side++;
				}
				while (state[x][y] == cur_turn) {
					x += d[i][0]; y += d[i][1]; side_stone++;
				}
				x = ori_x-d[i][0]; y = ori_y-d[i][1];
				while (state[x][y] == cur_turn) {
					x -= d[i][0]; y -= d[i][1]; stone++;
				}
				while (state[x][y] == 0) {
					x -= d[i][0]; y -= d[i][1]; side++;
				}
				while (state[x][y] == cur_turn) {
					x -= d[i][0]; y -= d[i][1]; side_stone++;
				}
				if (stone + side + side_stone >= 4) { //4������ �� Ȯ���� �������� ������ �ο�
					if (max_bot < stone && ori_x==h[ori_y]+1) max_bot = stone;
					point += stone*stone;
				}

				//rival�� ���� bot�� ���� �Ȱ��� ������ �ο��Ѵ�.
				cur_turn = rival_turn;
				stone = 0; side = 0;
				x = ori_x+d[i][0]; y = ori_y+d[i][1];
				while (state[x][y] == cur_turn) {
					x += d[i][0]; y += d[i][1]; stone++;
				}
				while (state[x][y] == 0) {
					x += d[i][0]; y += d[i][1]; side++;
				}
				while (state[x][y] == cur_turn) {
					x += d[i][0]; y += d[i][1]; side_stone++;
				}
				x = ori_x-d[i][0]; y = ori_y-d[i][1];
				while (state[x][y] == cur_turn) {
					x -= d[i][0]; y -= d[i][1]; stone++;
				}
				while (state[x][y] == 0) {
					x -= d[i][0]; y -= d[i][1]; side++;
				}
				while (state[x][y] == cur_turn) {
					x -= d[i][0]; y -= d[i][1]; side_stone++;
				}
				if (stone + side + side_stone >= 4) {
					if (max_rival < stone && ori_x == h[ori_y] + 1) max_rival = stone;
					point -= stone * stone;
				}
			}
		}
	}
	//�ٷ� ���� �� �ִ� �ڸ��� �߿���. ���� �� ������ ����ų� ����� ������� �������̱� ������ �̰����� ������ �ο�
	if (turn==bot_turn) {
		if (max_bot == 3) point += 100000;
		else if(max_bot>=max_rival) point += (max_bot+1)*(max_bot + 1) * 3 / 4;
		else point += max_rival * max_rival*3/4;
	}
	else {
		if (max_rival == 3) point -= 100000;
		else if(max_rival>=max_bot) point -= (max_rival+1)*(max_rival + 1) * 2 / 3;
		else point -= max_bot * max_bot * 3 / 4;
	}
	return point;
}
int huristic_func(int turn) { //���� state�� ����� ���� ������ �޸���ƽ ���� ���
	return huristic2(turn);
}
int search(int depth, int MIN, int MAX) { //search �Լ� //MIN,MAX�� ���� ����� parent�� MIN,MAX ���̴�. �Լ��� ��ȯ���� huristic��
	search_count++; //depth_limit�� ���������� �����ϱ����� Ž���� state�� ����.
	if (stone_n == row_n * col_n) return 0;
	int cur_turn = bot_turn + depth%2;
	if (cur_turn == 3) cur_turn = 1;

	if (depth == depth_limit) {
		return huristic_func(cur_turn);
	}
	int local_MIN = INF, local_MAX = -INF; //���� ��忡�� MIN,MAX
	for (int i = 1; i <= col_n; i++) {
		if (push_stone(cur_turn, i)) {
			int ret;
			if (chk_win(cur_turn, i)) {
				if (cur_turn == bot_turn) ret = INF;
				else ret = -INF;
			}
			else ret = search(depth + 1, local_MIN, local_MAX); //i��° column�� ���� �������� ���Ǵ� �޸���ƽ ���� ret�� �����´�.

			if (cur_turn != bot_turn) local_MIN = min(local_MIN, ret);
			else local_MAX = max(local_MAX, ret);
			pop_stone(i);
		}
		if (local_MIN <= MAX || local_MAX>=MIN) break; //alpha-beta pruning �˰����� ���� pruning
	}
	if (cur_turn == bot_turn) return local_MAX;
	else return local_MIN;
}
void game() {
	rival_turn = swt(bot_turn);
	int prev_col = -1;
	int cur_turn = 2;
	int local_MAX = -INF;
	int prev_count = -1;
	while (1) {
		draw();
		cur_turn = swt(cur_turn);
		int sel=-1;
		if (cur_turn == bot_turn) {
			//rule-based algorithm
			search_count = BOUND + 1;
			sel = rule_based(prev_col);
			
			//search algorithm
			while (sel == -1) {
				search_count = 0;
				local_MAX = -INF;
				for (int i = 1; i <= col_n; i++) { //ù��° �δ� ���� �������� ���ºκ��� return�Ͽ��� �ϴ� ���� �ٸ��Ƿ� search �Լ� ���� ����
					if (ban[i]) continue; //rule-based�� ���� ���� ���ƾ� �� ���ΰ�� �ǳʶڴ�
					if (push_stone(cur_turn, i)) {
						int ret = search(1, INF, -INF);
						if (ret > local_MAX) {
							local_MAX = ret;
							sel = i;
						}
						pop_stone(i);
					}
				}
				if (sel == -1) { //������ ���� ��� ban�Ȱ�찡 ������ �����Ƿ� �װ�� �Ѽ��ִ� ���߿� �ƹ����� �д�. ��ǻ� ����.
					for (int i = 1; i <= col_n; i++) {
						if (push_stone(cur_turn, i)) {
							sel = i;
							pop_stone(i);
							break;
						}
					}
				}
				while (prev_count*5>=search_count && search_count < BOUND && depth_limit < col_n*row_n - stone_n) { //Ž���� search state�� depth�� �÷��� �ɸ�ŭ ����� ������ depth�� �ø���.
					depth_limit++;
					search_count *= col_n; //depth�� 1�� �þ�� ���� Ž���ϰ� �Ǵ� state�� ���� column�� ���� ����ϰ� �����ϹǷ�, 
					sel = -1; //����� ������ Ž������ ���Ͽ����Ƿ� depth_limit�� �÷��� �ٽ� Ž���ϵ��� �Ѵ�.
				}
			}
			prev_count = search_count;
			//end search algorithm

			//MCTS algorithm
			//if(sel==-1) sel = MCTS(cur_turn);
		}
		else {
			printf("������ : ");
			scanf_s("%d", &sel);
		}
		push_stone(cur_turn, sel);
		if (chk_win(cur_turn,sel)) {
			draw();
			if (cur_turn == bot_turn) printf("bot win!");
			else printf("player win!");
			break;
		}
		if (stone_n == row_n * col_n) {
			draw();
			printf("draw!");
			break;
		}
		prev_col = sel;
	}
}
int main() {
	init();
	game();
	system("pause");
	return 0;
}