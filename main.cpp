#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <set>
#include <vector>
#include <chrono>
#include <map>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;
using LineTypeSet = std::set<std::vector<int>>;
//using LvTypeSetMap = std::map<std::set<std::set<size_t>>,int>;


size_t gNode = 0;
size_t gLine = 0;
size_t gCol = 0;
size_t gLv = 0;//等级
int gCount = 0;
LineTypeSet gLineTypeSet;
std::map<std::vector<size_t>,size_t> gLvTypeMap;

std::string GetMatString(const std::vector<int>& mat){
    std::string data;
    size_t tmp = 0;
    for(size_t i = 0; i<mat.size();++i){
        data.append(std::to_string(mat[i]) + "   ");
        ++tmp;
        if(tmp == gCol){
            data.append("\n");
            tmp = 0;
        }
    }
    return data;
}

int GetLineLv(size_t start, size_t step , const std::vector<int>& mat){
    int startLv = mat[start];
    for(size_t i = 1; i<gCol;++i){
        if(mat[start + step*i] != startLv){
            return -1;
        }
    }
    return startLv;
}

void ClassifyMat(const std::vector<int>& mat){
    //print mat
    //std::cout<<GetMatString(mat)<<std::endl;
    std::vector<int> line(gLine,-1);
    for(size_t i = 0 ;i<gLine ;++i){
        int lv = -1;
        if(i<gCol){
            lv = GetLineLv(i*gCol,1,mat);
        }
        else if(i <gCol*2){
            lv = GetLineLv(i-gCol,gCol,mat);
        }
        else if(i == gLine-2)
        {
            lv = GetLineLv(0,gCol+1,mat);
        }
        else{
            lv = GetLineLv(gCol-1,gCol-1,mat);
        }
        line[i] = lv;
    }
    //print line 
    // for(auto e :line){
    //     printf("%4d",e);
    // }
    // std::cout<<std::endl;
    gLineTypeSet.insert(line);
    //parse lv type
    std::vector<size_t> tmpVec(gLv,0);
    for(size_t i = 0; i <line.size() ; ++i){
        int lv = line[i];
        if (lv < 0){
            continue;
        }
        tmpVec[static_cast<size_t>(lv)] += 1;
    }
    std::sort(tmpVec.begin(),tmpVec.end());
    size_t& num = gLvTypeMap[tmpVec];
    num++;
    if(num < 10){
        //save mat
        std::string dirName("./result/");
        for(auto e:tmpVec){
            dirName.append(std::to_string(e));
        }
        dirName.append("/");
        fs::create_directory(dirName);
        dirName.append(std::to_string(num)+".txt");
        std::fstream fileStream;
        fileStream.open(dirName.c_str(),std::ios::app);
        std::string data = GetMatString(mat);

        fileStream << data;
        fileStream.close();
    }
}

void DoFillMat(size_t index,std::vector<int>& mat){
    if(index>= mat.size()){
        ClassifyMat(mat);
        gCount++;
        return;
    }
    for(size_t lv = 0 ; lv<gLv;++lv){
        mat[index] = static_cast<int>(lv);
        DoFillMat(index+1,mat);
    }
}

int main(int argc, char *args[])
{
    if (argc != 3)
    {
        std::cerr << "argc error" << std::endl;
        return -1;
    }
    int col = atoi(args[1]);
    // gNode = static_cast<int>(pow(col,col));
    gCol = static_cast<size_t>(col);
    gNode = gCol* gCol;
    gLv = static_cast<size_t>(atoi(args[2]));
    gLine = gCol * 2 + 2;
    std::cout << "init gNode = " << gNode
              << " gLv = " << gLv 
              << " gLine = " << gLine
              << std::endl; 
    //create result dir
    fs::remove_all("./result");
    fs::create_directories("./result");
    std::chrono::steady_clock::time_point t1 = std::chrono::steady_clock::now();
    std::vector<int> mat;
    mat.reserve(gNode);
    for(size_t i = 0; i< gNode;++i){
        mat.push_back(-1);
    }
    DoFillMat(0,mat);
    std::chrono::steady_clock::time_point t2 = std::chrono::steady_clock::now();
    std::chrono::duration<double> time_span = duration_cast<std::chrono::duration<double>>(t2 - t1);
    std::cout<<"total count "<<gCount<<std::endl;
    std::cout<<"line type count "<< gLineTypeSet.size()<<std::endl;
    std::cout<<"lv type count " << gLvTypeMap.size() <<std::endl;
    std::cout << "It took " << time_span.count() << " seconds." <<std::endl;
    return 0;
}