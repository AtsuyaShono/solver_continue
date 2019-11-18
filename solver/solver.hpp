#include <iostream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <vector>
#include <cmath>
#include <string.h>
#include <omp.h>

#include <sstream>
#include <fstream>

#include <time.h>

using namespace std;

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
vector<pair<int, long> > used_net; //枝を使ったネットid first:id second :cost
double sum; //各ネットの1/TDMの合計

edge() : cost(1) {
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

long priority; //ルーティングの優先順位、高い方から先にルーティングする
long cost;   //最終的なネットのTDM
vector<pair<int, long> > T;   //ダイクストラ用の最短経路edgeid first:edgeid,second:TDM

bool max; //スコアとなるグループに属している場合true
int max_his; //過去何回かで何回以上最大グループに属している場合正の値
bool max_once; //過去に一度でも最大グループになっていたらtrue

net() : cost(0),max_his(1){
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

unsigned int max_TDM = 0; //現在時点のスコア
unsigned int max_g = 0; //スコアとなっているグループ

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

inline int
digitBinary(int n)
{
        if (n < 100000) {
                if (n < 1000) {
                        if (n < 10)
                                return 1;
                        else if (n < 100)
                                return 2;
                        else
                                return 3;
                } else {
                        if (n < 10000)
                                return 4;
                        else
                                return 5;
                }
        } else {
                if (n < 10000000) {
                        if (n < 1000000)
                                return 6;
                        else
                                return 7;
                } else {
                        if (n < 100000000)
                                return 8;
                        else if (n < 1000000000)
                                return 9;
                        else
                                return 10;
                }
        }
}

////////////
/*メンバ関数*/
////////////
void edge::sum_forrestriction(){ //制約判定のためのTDM逆数の総和
        sum = 0;
        for(int i = 0; i < used_net.size(); ++i) sum += rcp(used_net[i].second);
}

void edge::increase_TDM(){ // 非常事態の時、適当に増やす

        if(sum > 2.0) {   //まだ制約を満たしていない場合
                bool debug = true;   //もし全てのネットが最大グループで計算が進まない時に計算を実行するためのフラグ

                for(int i = 0; i < used_net.size(); ++i) {
                        if(!N[used_net[i].first].max) {                   //最大グループのネットではない場合
                                debug = false;           //改善できるので非常用の計算回避
                                const long dumy = used_net[i].second * (sum-1) * 0.01 * rcp(digitBinary(used_net[i].second)) + 2;
                                if(N[used_net[i].first].max_his == 0) {
                                        if(!N[used_net[i].first].max_once) {
                                                used_net[i].second += 2.0*dumy; //TDM変更
                                        }
                                        else{
                                                used_net[i].second += 1.5*dumy; //TDM変更
                                        }
                                }
                                else {
                                        used_net[i].second += dumy;     //TDM変更
                                }
                        }
                }

                if(debug) {           //非常事態の計算
                        for(int i = 0; i < used_net.size(); ++i) {
                                const int dumy = used_net[i].second >> 1;
                                used_net[i].second += dumy;           //TDM変更
                        }
                }

        }
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
