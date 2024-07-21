#include "json.hpp"
#include <iostream>
#include <vector>
#include <map>
using json = nlohmann::json;
using namespace std;

string func1(){
    json js;
    js["id"] = 2;
    js["name"] = "张三";
    js["from"] = "中国";
    js["phone"] = "12234566";
    string buf = js.dump();
    return buf;
}
void func2(){
    json js;
    vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    js["list"] = vec;
    map<int,string> map;
    map.insert({1,"黄山"});
    map.insert({2,"泰山"});
    map.insert({3,"华山"});
    js["map"] = map;
    cout << js << endl;
}
int main(){
    string buf = func1();
    json jsbuf = json::parse(buf);
    int a = jsbuf["id"].get<int>();
    cout << a << endl
         << jsbuf["name"] << endl
         << jsbuf["from"] << endl
         << jsbuf["phone"];
    return 0;
}