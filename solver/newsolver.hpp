#include <iostream> //for cout
//#include <iomanip> //for setprecision
#include <queue> //for queue
//#include <algorithm> //
//#include <vector> //
//#include <cmath> //
#include <string.h> //for strtok
#include <sstream> //for istringstream
#include <fstream> //for ofsteram
#include <unordered_map> //

#include <omp.h> //for omp_set_num_threads
#include <time.h> //for clock

using namespace std;

#define INF 1e+9

/////////
/*クラス*/
/////////
class edge { //辺のクラス

public:
//入力ファイルより
int id; //index
int node_id1; //接続されているFPGAid
int node_id2; //接続されているFPGAid

vector<pair<int, long> > used_net; //枝を使ったネットid first:id second :cost
long cost;

edge() : cost(1){
}

};

class net { //ネットのクラス

public:
//入力ファイルより
int id; //index
int source_sig; //送信元nodeからの信号
vector<int> target_sig; //送信先node

vector<int> included_group; //含まれているグループid
vector<pair<int, int> > T; //解：枝、TDM
long cost;
long max_g_cost;

long priority; //ルーティングの優先順位、高い方から先にルーティングする

net() : cost(0),max_g_cost(0),priority(0){

}

void sum_cost();   //総コスト計算

bool operator<(const net& a) const
{
        return priority < a.priority;
}

};

class group { //ネットグループのクラス

public:
//入力ファイルより
int id; //index
vector<int> net_id; //ネットid

long cost; //TDM総和

group() : cost(0){
}

void sum_cost();   //総コスト計算

};

////////////////
/*グローバル変数*/
////////////////
int nf;  //FPGA数
int ne;  //FPGA接続数
int nw;  //ネット数
int ng;  //ネットグループ数

vector<vector <int> > V; //ノードの集合
vector<edge> E; //辺の集合
vector<net> N; //ネットの集合
vector<group> G; //ネットグループの集合

long max_TDM = 0; //現在時点のスコア
int max_g = 0; //スコアとなっているグループ

//////////////////
/*プロトタイプ宣言*/
//////////////////
void fileload(char *inputfile); //入力

void fileout(char *outputfile); //出力

void routing(); //経路探索

void calc_TDM(); //配線したそれぞれのネットの解枝にTDM割り当て　引数：枝のid

////////////
/*メンバ関数*/
////////////
void net::sum_cost(){ //コスト計算
        cost = 0;
        for(int i = 0; i < T.size(); ++i) cost += T[i].second;
}

void group::sum_cost(){ //コスト計算
        cost = 0;
        for(int i = 0; i < net_id.size(); ++i) cost += N[net_id[i]].cost;
}
