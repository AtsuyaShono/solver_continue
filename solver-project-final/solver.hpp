#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <vector>
#include <omp.h>

using namespace std;

//#include "RCP.cpp" //逆数の配列ファイル

#ifdef _OPENMP
omp_set_num_threads(4);
#endif

#define INF 1e+9

/////////
/*クラス*/
/////////
class node { //ノードクラス

public:
vector<int> edge;       //つながっているedgeID

node(){
}
};

class edge { //辺のクラス

public:
//入力ファイルより
int id;         //辺のid
int node_id1;         //接続されているFPGAid
int node_id2;         //接続されているFPGAid

int cost;   //TDM比
vector<pair<int, int> > used_net; //枝を使ったネットid first:id second :cost
double sum; //各ネットの1/TDMの合計

edge() : cost(0) {
}

bool restriction(); //TDM制約を満たしているか
void sum_forrestriction(); //制約確認のためのTDM逆数計算
void increase_TDM(); //TDMを２ずつ増やす関数

bool operator<(const edge& a) const
{
        return sum < a.sum;
}

};

class net { //ネットのクラス

public:
//入力ファイルより
int id;         //ネットid
int source_sig;         //送信元nodeからの信号
vector<int> target_sig;         //送信先node

int priority; //ルーティングの優先順位、高い方から先にルーティングする
int cost;   //最終的なネットのTDM
vector<pair<int, int> > T;   //ダイクストラ用の最短経路edgeid first:edgeid,second:TDM

bool max; //スコアとなるグループに属している場合true
int max_his; //過去何回かで何回以上最大グループに属している場合正の値
bool max_once; //過去に一度でも最大グループになっていたらtrue

net() : cost(0){
}

void sum_cost();   //総コスト計算
void sum_forrestriction(); //TDM制約のために1/TDMをedge.sumにたす
void max_flag(); //最大グループに属しているnetにフラグを立てる
void unmax(); //フラグ初期化

bool operator<(const net& a) const
{
        return priority < a.priority;
}
};

class group { //ネットグループのクラス

public:
//入力ファイルより
int id;         //ネットグループid
vector<int> net_id;         //ネットid

long cost;   //グループの総TDM

group() : cost(0){
}

void sum_cost();   //総コスト計算

bool operator<(const group& a) const
{
        return cost < a.cost;
}
};

class P {    //ダイクストラ計算用クラス

public:
int path;        //最短距離
int node;        //ノード番号

// ">" のオーバーロード pathを基準に大小比較を行う
bool operator>(const P& a) const
{
        return path > a.path;
}
};

////////////////
/*グローバル変数*/
////////////////
int nf;  //FPGA数
int ne;  //FPGA接続数
int nw;  //ネット数
int ng;  //ネットグループ数

vector<node> V; //ノードの集合
vector<edge> E; //辺の集合
vector<net> N; //ネットの集合
vector<group> G; //ネットグループの集合

float paramator = 1.0;
float paramator_x = 1.0;

long max_TDM = 0; //現在時点のスコア
long max_g = 0; //スコアとなっているグループ

long targets = 0;
long nets_in_group = 0;

//////////////////
/*プロトタイプ宣言*/
//////////////////
void fileload(char *inputfile); //入力

void fileout(char *outputfile); //出力

void routing(); //経路探索

void calc_TDM(); //配線したそれぞれのネットの解枝にTDM割り当て　引数：枝のid
//変更することによって改善する場合がある場所あり

//int rcp_size = RCP.size();

inline double rcp(const int num){
        //if(num < rcp_size) {
        //        return RCP[num];
        //}
        //else
        return (double)1/num;
}

////////////
/*メンバ関数*/
////////////
void edge::sum_forrestriction(){ //制約判定のためのTDM逆数の総和
        sum = 0;
        for(int i = 0; i < used_net.size(); ++i) sum += rcp(used_net[i].second);
}

void edge::increase_TDM(){ // 非常事態の時、適当に増やす
        for(int i = 0; i < used_net.size(); ++i) used_net[i].second += 2;
}

void net::sum_cost(){ //コスト計算
        cost = 0;
        for(int i = 0; i < T.size(); ++i) cost += T[i].second;
}

void net::max_flag(){ //フラグたて
        max = true;
        max_once = true;
        ++max_his;
}

void net::unmax(){ //フラグリセット
        max = false;
}

void group::sum_cost(){ //コスト計算
        cost = 0;
        for(int i = 0; i < net_id.size(); ++i) cost += N[net_id[i]].cost;
}
