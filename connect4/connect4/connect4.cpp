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
#define BOUND 1000000 //search state 크기가 BOUND 미만일때 limit를 늘림
#define Resource 300000 //MCTS 알고리즘의 시도 횟수
#define init_depth 8

struct mct_struct { //MCTS state의 구조체
	int win, n, child_num; //draw일 경우 1 win의 경우 2씩 증가함. n은 항상 2씩 증가
	bool terminal;
	mct_struct* child[8];
	int col[8];
};
struct uct_struct { //uct를 계산하기 위한 구조체
	int win, n;
};
int c=2; //UCT의 exploration parameter
int t; //MCTS에서 turn을 저장하는 변수

int state[8][9]; //0:empty . 1:first-player's stone O 2:second-player's stone X
int h[8]; //i번째 column에 현재 놓여진 돌의 높이
int bot_turn,rival_turn; //1:first 2:second
int depth_limit; //depth_limit의 정도를 저장할 변수(game_tree의 크기)
int d[4][2]; //가로,세로,두 대각선의 방향을 저장하는 변수 0:세로 1:가로 2,3:대각선
int search_count; //search state의 크기를 계산하기위한 변수
int stone_n; //현재 놓여진 stone의 개수를 저장함
bool ban[9];
void init() {
	srand((unsigned int)time(NULL)); //MCTS 알고리즘을 위한 난수 생성
	depth_limit = init_depth; //초기의 depth_limit의 정도 경험적으로 적절한 값을 넣음
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

	printf("봇의 턴을 입력해주세요(1:first 2:second) : ");
	scanf_s("%d", &bot_turn);
}
void draw() { //현재 게임 상황을 출력해주는 함수
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
bool push_stone(int turn, int col) { //col번째 column에 현재 turn의 플레이어의 돌을 둔다.
	if (h[col] == row_n) return false; //놓으려는 곳에 돌이 꽉차있으면 그곳에 넣을수 없음을 알려준다.
	state[++h[col]][col] = turn; //돌을 놓음

	//state 정보 update
	stone_n++;
	return true;
}
int pop_stone(int col) {
	if (h[col] == 0) return 0;
	int ret = state[h[col]][col];
	state[h[col]][col] = 0;
	h[col]--;

	//state 정보 update
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
uct_struct simulasion() { //균등한 확률로 다음 돌을 선택하면서 simulation 해준다.
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
uct_struct expansion(mct_struct* x) { //selection 된 state에서 가능한 모든 경우를 expansion 하여준다.
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
	if (x->child_num == 0) { //leaf를 발견한경우
		if (x->terminal) {
			ret.win = 0; ret.n = 0;
			return ret; //leaf가 terminal인경우 다른 과정을 생략하고 다음 iteration으로 넘어간다.
		}
		ret=expansion(x); //선택된 leaf에서 expansion 하고 각각 expansion된 state에서 simulation 하여주어 결과값을 가져온다.
		return ret;
	}
	//UCT값이 높을수록 선택될 확률이 높도록 하여 selection
	int uct[7];
	int sum = 0;
	for (int i = 0; i < x->child_num; i++) {
		mct_struct* child = x->child[i];
		double uct_val = (double)child->win / child->n + (double)c*sqrt(log((double)x->n) / child->n); //uct 공식
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
	x->n += ret.n; x->win += ret.win; //backpropagation의 역할을 하는 구문
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
int MCTS(int turn) { //MCTS를 이해한 것을 바탕으로 작성하였다.
	mct_struct* root=init_mct();
	int MCTS_cnt = 0; t = turn; //root의 턴을 저장
	while (MCTS_cnt++ < Resource) {
		selection(root);
	}
	int sel = 0;
	double max = 0;
	for (int i = 0; i < root->child_num; i++) { //root의 child(이번에 놓을수)중 가장 이길 확률이 높은 것을 고른다. exploitation을 고려함.
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

int huristic1() { //첫번째로 시도한 huristic function. line을 세어서 huristic 값을 계산한다.
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
				//bot의 점수 현재 위치에 놓았을때 만들어지는 라인으로 점수를 계산한다.
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
				if (stone + side + side_stone >= 4) { //4라인이 될 확률이 있을때만 점수를 부여
					if (max_bot < stone && ori_x==h[ori_y]+1) max_bot = stone;
					point += stone*stone;
				}

				//rival의 점수 bot의 경우와 똑같이 점수를 부여한다.
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
	//바로 놓을 수 있는 자리가 중요함. 가장 긴 라인을 만들거나 상대의 긴라인을 막을것이기 때문에 이것으로 점수를 부여
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
int huristic_func(int turn) { //현재 state에 저장된 돌의 정보로 휴리스틱 값을 계산
	return huristic2(turn);
}
int search(int depth, int MIN, int MAX) { //search 함수 //MIN,MAX는 현재 노드의 parent의 MIN,MAX 값이다. 함수의 반환값은 huristic값
	search_count++; //depth_limit를 유동적으로 조절하기위해 탐색한 state를 센다.
	if (stone_n == row_n * col_n) return 0;
	int cur_turn = bot_turn + depth%2;
	if (cur_turn == 3) cur_turn = 1;

	if (depth == depth_limit) {
		return huristic_func(cur_turn);
	}
	int local_MIN = INF, local_MAX = -INF; //현재 노드에서 MIN,MAX
	for (int i = 1; i <= col_n; i++) {
		if (push_stone(cur_turn, i)) {
			int ret;
			if (chk_win(cur_turn, i)) {
				if (cur_turn == bot_turn) ret = INF;
				else ret = -INF;
			}
			else ret = search(depth + 1, local_MIN, local_MAX); //i번째 column에 돌을 놓았을때 기대되는 휴리스틱 값을 ret에 가져온다.

			if (cur_turn != bot_turn) local_MIN = min(local_MIN, ret);
			else local_MAX = max(local_MAX, ret);
			pop_stone(i);
		}
		if (local_MIN <= MAX || local_MAX>=MIN) break; //alpha-beta pruning 알고리즘을 통해 pruning
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
				for (int i = 1; i <= col_n; i++) { //첫번째 두는 수의 선택지를 고르는부분은 return하여야 하는 값이 다르므로 search 함수 전에 구현
					if (ban[i]) continue; //rule-based에 의해 두지 말아야 할 수인경우 건너뛴다
					if (push_stone(cur_turn, i)) {
						int ret = search(1, INF, -INF);
						if (ret > local_MAX) {
							local_MAX = ret;
							sel = i;
						}
						pop_stone(i);
					}
				}
				if (sel == -1) { //가능한 수가 모두 ban된경우가 있을수 있으므로 그경우 둘수있는 수중에 아무데나 둔다. 사실상 졌다.
					for (int i = 1; i <= col_n; i++) {
						if (push_stone(cur_turn, i)) {
							sel = i;
							pop_stone(i);
							break;
						}
					}
				}
				while (prev_count*5>=search_count && search_count < BOUND && depth_limit < col_n*row_n - stone_n) { //탐색한 search state가 depth를 늘려도 될만큼 충분히 작으면 depth를 늘린다.
					depth_limit++;
					search_count *= col_n; //depth가 1이 늘어남에 따라 탐색하게 되는 state의 수가 column의 수에 비례하게 증가하므로, 
					sel = -1; //충분한 공간을 탐색하지 못하였으므로 depth_limit를 늘려서 다시 탐색하도록 한다.
				}
			}
			prev_count = search_count;
			//end search algorithm

			//MCTS algorithm
			//if(sel==-1) sel = MCTS(cur_turn);
		}
		else {
			printf("착수점 : ");
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