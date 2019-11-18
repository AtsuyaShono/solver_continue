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

        //スコア表示
        max_TDM = 0;
        for(int i = 0; i < nw; ++i) {
                N[i].sum_cost(); //ネットごとのTDMを計算
        }
        for(int i = 0; i < ng; ++i) {
                G[i].sum_cost(); //グループごとのTDMを計算
        }
        sort(G.begin(), G.end());
        max_g = G[ng-1].id;
        max_TDM = G[ng-1].cost;
        cout << "Max group ID is: " << max_g << " and maximum total TDM ratio of all net groups is: " << G[ng-1].cost << endl;

        return 0;
}
///////
/*関数*/
///////
/*
   void fileload(char *inputfile){ //入力

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
 ++targets;
                }
        }

        //group
        for(i = 0; i < ng; ++i) {
                G[i].id = i;         //id格納
                getline(ifs, line); //1行読み込む
                istringstream stream(line);
                for (j = 0; stream >> data; ++j) { // 1個ずつ読み込み
                        G[i].net_id.emplace_back(data); //格納
 ++nets_in_group;
                }
        }
   }
 */

void fileload(char *inputfile){

        int size = 1024*1024;
        int data1, data2;  //読み取り変数
        char line[size];  //文字列記憶用
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
                data1 = atoi(strtok(line, " "));
                N[i].source_sig = data1;
                while(1) {
                        const char *str = strtok(NULL, " ");
                        if(str == NULL) break;
                        data1 = atoi(str);
                        N[i].target_sig.emplace_back(data1);
                }
        }

        for(i = 0; i < ng; ++i) {
                G[i].id = i;
                fgets(line, size, fp);
                data1 = atoi(strtok(line, " "));
                G[i].net_id.emplace_back(data1);
                while(1) {
                        const char *str = strtok(NULL, " ");
                        if(str == NULL) break;
                        data1 = atoi(str);
                        G[i].net_id.emplace_back(data1);
                }
        }
}



void fileout(char *outputfile){ //出力

        int i,j;
        char out[1024];

        FILE *fp = fopen(outputfile, "w");

        for(i = 0; i < nw; ++i) {
                sprintf(out, "%lu\n", N[i].T.size());
                for(j = 0; j < N[i].T.size(); ++j) {
                        sprintf(out, "%s%d %ld\n", out, N[i].T[j].first, N[i].T[j].second);
                }
                fprintf(fp, "%s", out);
        }
        //int i,j;
        //string line;

        //ofstream ofs(outputfile);
        //ostringstream stream(line);

        //for(i = 0; i < nw; ++i) {
        //        stream.str("");
        //        stream << N[i].T.size() << endl;
        //        for(j = 0; j < N[i].T.size(); ++j) {
        //                stream << N[i].T[j].first << " " << N[i].T[j].second << endl;
        //        }
        //        ofs << stream.str();
        //}
}

void routing(){ //経路探索

        int i,j;
        priority_queue<net, vector<net>, less<net> > que; //キュー：降順

        //ネットが使われているグループのネットの数順にネットをルーティングしていく
        //その優先順位決め
        for ( i = 0; i < ng; ++i)
                for ( j = 0; j < G[i].net_id.size(); ++j)
                        N[G[i].net_id[j]].priority += G[i].net_id.size();

        //ダイクストラ用のnodeクラスに情報を格納
        for(i = 0; i < ne; ++i) {
                V[E[i].node_id1].edge.emplace_back(E[i].id);         //最後尾にedgeID格納
                V[E[i].node_id2].edge.emplace_back(E[i].id);         //最後尾にedgeID格納
        }

        for(i = 0; i < nw; ++i) {
                N[i].priority += N[i].target_sig.size();
                que.push(N[i]); //優先順位順にキューにpushする
        }

        //経路探索
        #pragma omp parallel for
        for(i = 0; i < nw; ++i) {
                int id; //処理するnetID
                vector<bool> included_sig(nf); //ノードnum が信号を含んでいればtrue :included_sig[num] = true
                vector<bool> target_sig(nf); //ノードnum がN[id]のtargetならtrue :target_sig[num] = true
                vector<int> T; //解枝記憶
                vector<bool> penalty_cost(ne,1); //すでに通過済みの枝numなら0 :penalty_cost[num] = 0

                //#pragma omp ordered
                #pragma omp critical
                {
                        id = que.top().id;         //ルーティングするネットidを記憶
                        que.pop();         //ルーティングしたネットidを削除
                }

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
                                        //target_sig[v] = false; //target訪問済みなのでfalse
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
                                //E[T[loop]].cost += 1 + (double)(nw - i) / nw;
                        }
                }
        }
}

void calc_TDM(){

        int i,j;
        //queue<edge> q_edge;
        priority_queue<edge, vector<edge>, less<edge> > q_edge;

        int top = nw * 0.01;         //グループコスト順に上からtop分のネットをmax,max_hisと設定する
        int count = 0; //max_his用のカウント
        int top_g = 0; //上位のグループの数

        for(i = 0; i < nw; ++i)
                for(j = 0; j < N[i].T.size(); ++j)
                        E[N[i].T[j].first].used_net.push_back({N[i].id,E[N[i].T[j].first].cost * 0.01 + 2});         //使った枝にネットidを記憶させる

        for(i = 0; i < ne; ++i) {         //ネットごとのTDMを計算
                for(j = 0; j < E[i].used_net.size(); ++j)
                        N[E[i].used_net[j].first].cost += E[i].used_net[j].second;
                q_edge.push(E[i]);         //キューにedge id をpush
        }

        //だいたいでTDMを増やしていく
        while(1) {
                #pragma omp parallel
                {
                        #pragma omp for private(j)
                        for(i = 0; i < top_g; ++i)
                                for(j = 0; j < G[ng-1-i].net_id.size(); ++j)
                                        N[G[ng-1-i].net_id[j]].unmax(); //最大グループ所属のネットフラグ初期化

                        #pragma omp for
                        for(i = 0; i < nw; ++i) N[i].cost = 0;

                        #pragma omp for private(j)
                        for(i = 0; i < ne; ++i)
                                for(j = 0; j < E[i].used_net.size(); ++j)
                                        N[E[i].used_net[j].first].cost += E[i].used_net[j].second;                                   //ネットのコスト更新

                        //全グループのコスト計算
                        #pragma omp for
                        for(i = 0; i < ng - top_g; ++i)
                                G[i].sum_cost(); //グループごとのTDMを計算

                        #pragma omp for
                        for(i = 0; i < ne; ++i)
                                E[i].sum_forrestriction(); //TDM逆数総和の計算
                }


                sort(G.begin(), G.end());         //cost順にsort

                //top個のネットにフラグたて
                int count_net = 0; //フラグをたてたネットの数
                i = 0;
                while(count_net < top) {
                        #pragma omp parallel for
                        for(j = 0; j < G[ng-1-i].net_id.size(); ++j)
                                if(N[G[ng-1-i].net_id[j]].max == false) //フラグが立っていないなら
                                        N[G[ng-1-i].net_id[j]].max_flag(); //フラグたて

                        count_net += G[ng-1-i].net_id.size(); //カウント
                        ++i;
                }
                top_g = i; //最大のグループの数

                ++count; //何周したか
                if(count >= 10)         //max_hisを一定周期でリセット
                {
                        for(i = 0; i < nw; ++i) {
                                N[i].max_his -= 7; //40回中30回以上最大グループに属していたならフラグは立ったまま
                                if(N[i].max_his < 0) N[i].max_his = 0;
                        }
                        count = 0; //リセット
                }

                int loop = q_edge.size();
                #pragma omp parallel for
                for(int count = 0; count < loop; ++count) {
                  #pragma omp critical
                        {
                                i = q_edge.top().id; //TDM操作するedgeを記憶
                                q_edge.pop(); //一旦消す
                        }
                        E[i].increase_TDM();
                }
                for(i = 0; i < ne; ++i)
                        if(E[i].sum > 2.0) q_edge.push(E[i]);

                if(q_edge.empty()) break;         //全ての枝が制約を満たせば終了
        }

        for(i = 0; i < nw; ++i)
                N[i].T.clear();         //解をクリア

        //解（枝、TDM）代入
        for(i = 0; i < ne; ++i) {
                for(j = 0; j < E[i].used_net.size(); ++j) {
                        N[E[i].used_net[j].first].T.push_back({E[i].id,2*E[i].used_net[j].second});
                }
        }
}
