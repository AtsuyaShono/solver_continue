#include "newsolver.hpp"

//メイン関数
int main(int argc, char **filename){  //実行コマンド　./a.out 入力ファイル　出力ファイル
        //filename[0]:実行ファイル名　1:入力ファイル名 2:出力ファイル名

        //処理//
        #ifdef _OPENMP
        omp_set_num_threads(8);
        #endif

/*
        /////////////////
        //ファイル読み込み//
        /////////////////
        fileload(filename[1]);

        //////////
        //経路探索//
        //////////
        routing();

        //////////
        //TDM計算//
        //////////
        calc_TDM();

        /////////////////
        //ファイル書き込み//
        /////////////////
        fileout(filename[2]);
 */

        clock_t start = clock();

        /////////////////
        //ファイル読み込み//
        /////////////////
        fileload(filename[1]);

        clock_t end = clock();
        double time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        printf("fileload time %lf[s]\n", time);

        start = clock();

        //////////
        //経路探索//
        //////////
        routing();

        end = clock();
        time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        printf("routing time %lf[s]\n", time);

        start = clock();

        //////////
        //TDM計算//
        //////////
        calc_TDM();

        end = clock();
        time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        printf("calc_TDM time %lf[s]\n", time);

        start = clock();

        /////////////////
        //ファイル書き込み//
        /////////////////
        fileout(filename[2]);

        end = clock();
        time = static_cast<double>(end - start) / CLOCKS_PER_SEC;
        printf("fileout time %lf[s]\n", time);

        ////スコア表示
        max_TDM = 0;
        for(int i = 0; i < nw; ++i) {
                N[i].sum_cost(); //ネットごとのTDMを計算
        }
        for(int i = 0; i < ng; ++i) {
                G[i].sum_cost(); //グループごとのTDMを計算
                if(max_TDM < G[i].cost) {
                        max_TDM = G[i].cost;
                        max_g = i;
                }
        }

        cout << "Max group ID is: " << max_g << " and maximum total TDM ratio of all net groups is: " << max_TDM << endl;

        return 0;
}

void fileload(char *inputfile){    //入力

        int size = 1024*1024;
        int data1, data2; //読み取り変数
        char line[size]; //行文字列記憶用
        char *str; //文字列記憶用
        int i;

        FILE *fp = fopen(inputfile, "r");

        fscanf(fp, "%d %d %d %d", &nf, &ne, &nw, &ng); //ノード,枝,ネット,ネットグループ

        //領域確保
        V.resize(nf);
        E.resize(ne);
        N.resize(nw);
        G.resize(ng);

        for(i = 0; i < ne; ++i) {
                fscanf(fp, "%d %d\n", &data1, &data2);

                E[i].id = i;
                E[i].node_id1 = data1;
                E[i].node_id2 = data2;
                V[data1].emplace_back(E[i].id);
                V[data2].emplace_back(E[i].id);
        }

        for(i = 0; i < nw; ++i) {
                fgets(line, size, fp);
                str = strtok(line, " ");
                data1 = atoi(str);

                N[i].id = i;
                N[i].source_sig = data1;
                while(1) {
                        str = strtok(NULL, " ");
                        if(str == NULL) break;
                        data1 = atoi(str);

                        N[i].target_sig.emplace_back(data1);
                }
        }

        for(i = 0; i < ng; ++i) {
                fgets(line, size, fp);
                str = strtok(line, " ");
                data1 = atoi(str);

                G[i].id = i;
                G[i].net_id.emplace_back(data1);
                N[data1].included_group.emplace_back(i);
                while(1) {
                        str = strtok(NULL, " ");
                        if(str == NULL) break;
                        data1 = atoi(str);

                        G[i].net_id.emplace_back(data1);
                        N[data1].included_group.emplace_back(i);
                }
        }
}

void fileout(char *outputfile){ //出力

        int i,j;
        string line;

        ofstream ofs(outputfile);
        ostringstream stream(line);

        for(i = 0; i < nw; ++i) {
                stream.str("");
                stream << N[i].T.size() << endl;
                for(j = 0; j < N[i].T.size(); ++j) {
                        stream << N[i].T[j].first << " " << N[i].T[j].second << endl; //first: 枝番号 second: TDM
                }
                ofs << stream.str();
        }
}

void routing(){ //経路探索

        vector<net> priority; //ネットをルーティングする順番

        //ネットが使われているグループのネットの数順にネットをルーティングしていく
        //その優先順位決め
        for (int i = 0; i < ng; ++i)
                for (int j = 0; j < G[i].net_id.size(); ++j)
                        N[G[i].net_id[j]].priority += G[i].net_id.size();

        for(int i = 0; i < nw; ++i) {
                N[i].priority += N[i].target_sig.size();
                priority.emplace_back(N[i]);                 //優先順位順にキューにpushする
        }

        sort(priority.begin(), priority.end(),greater<net>());

        //経路探索
        #pragma omp parallel for
        for(int i = 0; i < nw; ++i) {
                const int id = priority[i].id; //ルーティングするネットid
                vector<int> T; //解枝
                unordered_map<int, bool> target; //送信先のノードならtrue
                unordered_map<int, bool> includes;
                int v; //現在地点のノード番号

                for(int j = 0; j < N[id].target_sig.size(); ++j)
                        target[N[id].target_sig[j]] = true; //送信先にフラグたて

                for(int j = 0; j < N[id].target_sig.size(); ++j) {
                        vector<int> dis(nf,INF); //dis[行き先のノード] = 出発地点から行き先までのコスト
                        unordered_map<int, int> route; //経路記憶 //キー:ノード 値:経路で使われる直近の枝
                        priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > que; //キュー：昇順　//first:最短距離コスト　second:ノード番号

                        //初期化
                        dis[N[id].source_sig] = 0; //出発地点のコストは0
                        que.push({0, N[id].source_sig}); //キューに出発地点を追加
                        v = N[id].source_sig; //現在地点初期化

                        //探索
                        while(!que.empty()) { //キューが空になるまでループ
                                const pair<int, int> p = que.top(); //キューの最短距離最小値を取り出す
                                que.pop(); //キューから取り出したものを削除
                                v = p.second; //現在地

                                if(target[v] == true) { //まだ繋がっていないターゲットを見つけたら終了
                                        target[v] = false;
                                        int node = v;
                                        while(1) {
                                                //T.emplace_back(route[node]); //解の枝を記憶
                                                N[id].T.push_back({route[node], 2}); //解を代入
                                                includes[node] = true;
                                                if(E[route[node]].node_id1 != node) node = E[route[node]].node_id1; //枝の接続先を記憶
                                                else node = E[route[node]].node_id2; //枝の接続先を記憶
                                                if(includes[node] == true || node == N[id].source_sig) break;
                                        }
                                        break;
                                }

                                if(dis[v] < p.first) continue; //startからvまでのコストが現在時点の最短距離より小さい場合スキップ（枝刈り）

                                for(int k = 0; k < V[v].size(); ++k) { //vの枝を全て参照
                                        const edge e = E[V[v][k]]; //vのk番目のedgeを記憶
                                        int to; //記憶用接続先のnodeid
                                        if(e.node_id1 == v) to = e.node_id2; //node_id1がvなら接続先はnode_id2
                                        else to = e.node_id1; //node_id2がvなら接続先はnode_id1

                                        if(dis[to] > dis[v] + e.cost * !includes[to]) { //現在の最短距離よりV[v][i]のエッジを使ったほうが短い時
                                                dis[to] = dis[v] + e.cost * !includes[to]; //更新
                                                que.push({dis[to], to}); //追加
                                                route[to] = e.id; //toまでの経路で最後に使用したedge.idを格納
                                        }
                                }
                        }
                }

                #pragma omp critical
                {
                        for(int j = 0; j < N[id].T.size(); ++j)
                                ++E[N[id].T[j].first].cost;         //コスト更新
                }
                //}

        }

}

void calc_TDM(){ //配線したそれぞれのネットの解枝にTDM割り当て　引数：枝のid

        for(int i = 0; i < nw; ++i)
                for(int j = 0; j < N[i].T.size(); ++j)
                        E[N[i].T[j].first].used_net.push_back({N[i].id,E[N[i].T[j].first].cost}); //使った枝にネットidを記憶させる

        #pragma omp parallel
        {
                #pragma omp parallel for
                for(int i = 0; i < nw; ++i)
                        N[i].T.clear();     //解をクリア

                #pragma omp parallel for
                for(int i = 0; i < ne; ++i)
                        for(int j = 0; j < E[i].used_net.size(); ++j)
                                N[E[i].used_net[j].first].cost += E[i].used_net[j].second;               //ネットのコスト更新

                //全グループのコスト計算
                #pragma omp parallel for
                for(int i = 0; i < ng; ++i)
                        G[i].sum_cost(); //グループごとのTDMを計算

                #pragma omp parallel for
                for(int i = 0; i < nw; ++i)
                        for (int j = 0; j < N[i].included_group.size(); j++)
                                if(N[i].max_g_cost < G[N[i].included_group[j]].cost) N[i].max_g_cost = G[N[i].included_group[j]].cost;


                #pragma omp parallel for
                for (int i = 0; i < ne; i++) {
                        long sum = 0;
                        for (int j = 0; j < E[i].used_net.size(); j++)
                                sum += N[E[i].used_net[j].first].max_g_cost;

                        for (int j = 0; j < E[i].used_net.size(); j++) {
                                long sum_ = N[E[i].used_net[j].first].max_g_cost * 2;
                                E[i].used_net[j].second = (sum + (sum_ - 1)) / (sum_);
                        }
                }
        }

        //解（枝、TDM）代入
        for(int i = 0; i < ne; ++i)
                for(int j = 0; j < E[i].used_net.size(); ++j)
                        N[E[i].used_net[j].first].T.push_back({E[i].id, 2*E[i].used_net[j].second});

}
