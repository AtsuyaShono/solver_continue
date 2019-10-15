#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <queue>
#include <algorithm>

using namespace std;

int main(void){

        std::cout<<std::fixed<<std::setprecision(10);

        vector<double> rcp(10000000);

        for(int i = 1; i < rcp.size(); ++i) {
                rcp[i] = (double)1/i;
        }

        ofstream ofs("RCP.cpp");

        ofs << "vector<double> RCP{0," << endl;
        for(int i = 1; i < rcp.size(); ++i) {
                if(i%30 == 0) ofs << endl;
                ofs << setprecision(10) << rcp[i];
                if(i != rcp.size()-1) ofs << ",";
        }
        ofs << "};";
}
