#include "solver.hpp"

//メイン関数
int main(int argc, char **filename){  //実行コマンド　./a.out 入力ファイル　出力ファイル
        //filename[0]:実行ファイル名　1:入力ファイル名 2:出力ファイル名

        //処理//
        #ifdef _OPENMP
        omp_set_num_threads(8);
        #endif

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

        //TDM　ratio 確認
        //for(int i = 0; i < ne; ++i) E[i].sum = 0;
        //for(int i = 0; i < nw; ++i)
        //        N[i].sum_forrestriction();
        //for(int i = 0; i < ne; ++i) {
        //        cout << "  " << setprecision(5) << (float)E[i].sum;
        //        if(i % 20 == 0) cout << endl;
        //}
        //cout << endl;

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
///////
/*関数*/
///////
void fileload(char *inputfile){    //入力

        int size = 1024*1024;
        int data1, data2;    //読み取り変数
        char line[size];    //文字列記憶用
        char *str;
        int i;

        FILE *fp = fopen(inputfile, "r");

        fscanf(fp, "%d %d %d %d", &nf, &ne, &nw, &ng);
        V.resize(nf);
        E.resize(ne);
        N.resize(nw);
        G.resize(ng);

        for(i = 0; i < ne; ++i) {
                E[i].id = i;
                fscanf(fp, "%d %d\n", &data1, &data2);
                E[i].node_id1 = data1;
                E[i].node_id2 = data2;
        }

        for(i = 0; i < nw; ++i) {
                N[i].id = i;
                fgets(line, size, fp);
                str = strtok(line, " ");
                data1 = atoi(str);
                N[i].source_sig = data1;
                while(1) {
                        str = strtok(NULL, " ");
                        if(str == NULL) break;
                        data1 = atoi(str);
                        N[i].target_sig.emplace_back(data1);
                }
        }

        for(i = 0; i < ng; ++i) {
                G[i].id = i;
                fgets(line, size, fp);
                str = strtok(line, " ");
                data1 = atoi(str);
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

/*
        int data; //読み取り変数
        string line; //文字列記憶用
        int i,j;

        ifstream ifs(inputfile);
        if (!ifs)
        {
                cout << "Cannot Read File" << endl; //エラー
                exit(0);
        }
        getline(ifs, line); //行合わせ
        istringstream stream(line);

        //nf
        stream >> nf; //読み込み

        //ne
        stream >> ne; //読み込み

        //nw
        stream >> nw; //読み込み

        //ng
        stream >> ng; //読み込み

        V.resize(nf);
        E.resize(ne);
        N.resize(nw);
        G.resize(ng);

        //edge
        for(i = 0; i < ne; ++i) {
                E[i].id = i; //id格納
                getline(ifs, line); //1行読み込む
                istringstream stream(line);
                stream >> E[i].node_id1; //読み込み
                stream >> E[i].node_id2; //読み込み
        }

        //net
        for(i = 0; i < nw; ++i) {
                N[i].id = i; //id格納
                getline(ifs, line); // 1行読み込む
                istringstream stream(line);
                stream >> N[i].source_sig; //読み込み
                for (j = 0; stream >> data; ++j) { // 1個ずつ読み込み
                        N[i].target_sig.emplace_back(data); //格納
                }
        }

        //group
        for(i = 0; i < ng; ++i) {
                G[i].id = i;         //id格納
                getline(ifs, line); //1行読み込む
                istringstream stream(line);
                for (j = 0; stream >> data; ++j) { // 1個ずつ読み込み
                        G[i].net_id.emplace_back(data); //格納
                        N[data].included_group.emplace_back(i);
                }
        }
 */
}


void fileout(char *outputfile){ //出力
/*
        char out[1024*1024];

        FILE *fp = fopen(outputfile, "w");

        for(int i = 0; i < nw; ++i) {
                sprintf(out, "%lu\n", N[i].T.size());
                for(int j = 0; j < N[i].T.size(); ++j) {
                        sprintf(out, "%s%d %ld\n", out, N[i].T[j].first, N[i].T[j].second);
                }
                fprintf(fp, "%s", out);
        }
 */
        string line;

        ofstream ofs(outputfile);
        ostringstream stream(line);

        for(int i = 0; i < nw; ++i) {
                stream.str("");
                stream << N[i].T.size() << endl;
                for(int j = 0; j < N[i].T.size(); ++j) {
                        stream << N[i].T[j].first << " " << N[i].T[j].second << endl;
                }
                ofs << stream.str();
        }

}

void routing(){ //経路探索

        int i,j;
        priority_queue<net, vector<net>, less<net> > que; //キュー：降順
        //vector<net> priority;

        //ネットが使われているグループのネットの数順にネットをルーティングしていく
        //その優先順位決め
        for (i = 0; i < ng; ++i)
                for ( j = 0; j < G[i].net_id.size(); ++j)
                        N[G[i].net_id[j]].priority += G[i].net_id.size();

        //ダイクストラ用のnodeクラスに情報を格納
        for(i = 0; i < ne; ++i) {
                V[E[i].node_id1].edge.emplace_back(E[i].id);         //最後尾にedgeID格納
                V[E[i].node_id2].edge.emplace_back(E[i].id);         //最後尾にedgeID格納
        }

        for(i = 0; i < nw; ++i) {
                //N[i].priority += N[i].target_sig.size();
                //priority.emplace_back(N[i]); //優先順位順にキューにpushする
                que.push(N[i]);
        }

        //for(i = 0; i < nw; ++i) {
        //        priority.emplace_back(que.top()); //優先順位順にキューにpushする
        //        que.pop();
        //}

        //sort(priority.begin(), priority.end());

        //経路探索
        #pragma omp parallel for
        for(i = 0; i < nw; ++i) {
                int id; //処理するnetID
                vector<bool> included_sig(nf); //ノードnum が信号を含んでいればtrue :included_sig[num] = true
                vector<bool> target_sig(nf); //ノードnum がN[id]のtargetならtrue :target_sig[num] = true
                vector<int> T; //解枝記憶
                vector<bool> penalty_cost(ne,1); //すでに通過済みの枝numなら0 :penalty_cost[num] = 0

                #pragma omp critical
                {
                        id = que.top().id; //ルーティングするネットidを記憶
                        que.pop(); //ルーティングしたネットidを削除
                }

                //id = priority[nw - i].id;

                included_sig[N[id].source_sig] = true; //送信元にフラグたて
                for(int loop = 0; loop < N[id].target_sig.size(); ++loop) {
                        target_sig[N[id].target_sig[loop]] = true; //送信先にフラグたて
                }

                for(int loop = 0; loop < N[id].target_sig.size(); ++loop) {         //ターゲットの数だけルーティング
                        vector<int> dis(nf,INF); //d[行き先]　出発地点から行き先までのコスト
                        vector<int> route(nf); //経路記憶,node i までの最短経路で最後に使われたedgeid：route[i] = edgeid
                        int v; //現在地点のノード番号
                        priority_queue<P, vector<P>, greater<P> > que; //キュー：昇順

                        //初期化
                        dis[N[id].source_sig] = 0;         //出発地点のコストは0
                        que.push(P{0, N[id].source_sig});         //キューに出発地点を追加
                        v = N[id].source_sig;         //現在地点初期化

                        //探索
                        while(!que.empty()) {         //キューが空になるまでループ
                                const P p = que.top();         //キューの最短距離最小値を取り出す
                                que.pop();         //キューから取り出したものを削除
                                v = p.node;         //現在地

                                if(target_sig[v] && !included_sig[v]) { //まだ繋がっていないターゲットを見つけたら終了
                                        break;
                                }

                                if(dis[v] < p.path) continue;         //startからvまでのコストが現在時点の最短距離より小さい場合スキップ（枝刈り）

                                for(int k = 0; k < V[v].edge.size(); ++k) {         //vの枝を全て参照
                                        const edge e = E[V[v].edge[k]];         //vのi番目のedgeを記憶
                                        int to;        //記憶用接続先のnodeid
                                        if(e.node_id1 == v) to = e.node_id2;         //node_id1がvなら接続先はnode_id2
                                        else to = e.node_id1;         //node_id2がvなら接続先はnode_id1

                                        if(dis[to] > dis[v] + e.cost * penalty_cost[e.id]) {         //現在の最短距離よりV[v][i]のエッジを使ったほうが短い時
                                                dis[to] = dis[v] + e.cost * penalty_cost[e.id];         //更新
                                                que.push(P{dis[to], to});         //追加
                                                route[to] = e.id;         //toまでの経路で最後に使用したedge.idを格納
                                        }
                                }
                        }

                        //ルート格納
                        while(1) {
                                included_sig[v] = true;         //信号を含むことになるノードにフラグを立てる
                                if(v == N[id].source_sig || penalty_cost[route[v]] == 0) break;         //探索を始めた元のノードに戻ってくれば終了 //コストが0になっているところはすでに訪問済みなので終了
                                else T.emplace_back(route[v]);         //解の枝を記憶
                                penalty_cost[route[v]] = 0; //もう使っているので次からコストは0
                                if(E[route[v]].node_id1 != v) v = E[route[v]].node_id1;         //枝の接続先を記憶
                                else v = E[route[v]].node_id2;         //枝の接続先を記憶
                        }
                }

                #pragma omp critical
                {
                        for(int loop = 0; loop < T.size(); ++loop) {
                                N[id].T.push_back({T[loop], 2});         //解を代入
                                ++E[T[loop]].cost; //コスト更新
                        }
                }
        }
}

void calc_TDM(){

        for(int i = 0; i < nw; ++i) {
                for(int j = 0; j < N[i].T.size(); ++j)
                        E[N[i].T[j].first].used_net.push_back({N[i].id,E[N[i].T[j].first].cost}); //使った枝にネットidを記憶させる
        }

        #pragma omp parallel
        {
                #pragma omp parallel for
                for(int i = 0; i < nw; ++i) {
                        N[i].T.clear();           //解をクリア
                }

                #pragma omp parallel for
                for(int i = 0; i < ne; ++i)
                        for(int j = 0; j < E[i].used_net.size(); ++j) {
                                N[E[i].used_net[j].first].cost += E[i].used_net[j].second;                     //ネットのコスト更新
                        }

                //全グループのコスト計算
                #pragma omp parallel for
                for(int i = 0; i < ng; ++i)
                        G[i].sum_cost(); //グループごとのTDMを計算

                #pragma omp parallel for
                for(int i = 0; i < nw; ++i) {
                        long max = 0;
                        for (int j = 0; j < N[i].included_group.size(); j++) {
                                if(max < G[N[i].included_group[j]].cost) max = G[N[i].included_group[j]].cost;
                        }
                        N[i].sum = max;
                }

                #pragma omp parallel for
                for (int i = 0; i < ne; i++) {
                        long sum = 0;
                        for (int j = 0; j < E[i].used_net.size(); j++) {
                                sum += N[E[i].used_net[j].first].sum;
                        }

                        for (int j = 0; j < E[i].used_net.size(); j++) {
                                long sum_ = N[E[i].used_net[j].first].sum;
                                E[i].used_net[j].second = ((sum + (sum_ - 1)) / (sum_) + 1) * 0.5;
                        }
                }
        }

        //解（枝、TDM）代入
        for(int i = 0; i < ne; ++i) {
                for(int j = 0; j < E[i].used_net.size(); ++j) {
                        N[E[i].used_net[j].first].T.push_back({E[i].id, 2*E[i].used_net[j].second});
                }
        }
}
