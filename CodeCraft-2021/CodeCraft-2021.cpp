#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>
#include <cmath>
#include "configuration.h"
#include "data_reading.h"
using namespace std;

const string FILENAME = "training-1.txt";

unordered_map<string, Server>	servers;	    // 服务器种类
unordered_map<string, VM>		vms;		    // 虚拟机种类
unordered_map<uint32_t, UserVM>	userVms;    	// 用户创建的虚拟机
vector<ProcuredServer*>			bgtServers;	    // 已购买的服务器

unordered_map<string, int>		newServers;	    // 每日新购入的服务器
list<Request>					requests;	    // 每日的请求序列
vector<pair<int, DeployedNode>> newVMs;
vector<string> newVMsInfo;
int                             curLastSrvID;   // 当前最新购买的服务器的编号

long long money;
long long eachday;
void cal() {
    money += eachday;
}


void TempVarInit()
{
	newServers.clear();
	requests.clear();
    newVMs.clear();
    newVMsInfo.clear();
}

int addVMReq(vector<int>& info);
void creatServerID();
void newVMsInfoToStr();
pair<string, vector<int>>			buyServer(int day);
pair<ProcuredServer*, DeployedNode>	DeployRequest(Request& req);
pair<ProcuredServer*, DeployedNode> dblSrvChoice(vector<pair<int, DeployedNode>>& temp);
pair<ProcuredServer*, DeployedNode> sglSrvChoice(vector<pair<int, DeployedNode>>& temp, double ratio);

int main()
{
    curLastSrvID = 0;
    money = 0, eachday = 0;
	// Open the file.
	ifstream file(FILENAME);
	// Load servers info.
	int serverKinds = fReadSrvNum(file);
	while (serverKinds--) {
		Server&& srv = fReadSrv(file);
		servers[srv.Name()] = srv;
	}
	// Load vms info.
	int vmKinds = fReadVMNum(file);
	while (vmKinds--) {
		VM&& vm = fReadVM(file);
		vms[vm.Name()] = vm;
	}
	// Get days;
	int T = fReadDays(file);
	while (T--) {
		// Requests count in one day.
		int R = fReadReqCnt(file), _R = R;
		// Temp var init and load request info.
		TempVarInit();
		while (_R--) {
			requests.push_back(fReadReq(file));
		}
		while (R--) {
			Request req = requests.front();
			if (req.Opt() == AddReq) {
                if (req.VmName() == "vmMXNSZ")
                    int aa = 1;
				pair<ProcuredServer*, DeployedNode> dplWay = DeployRequest(req);
				if (dplWay.first == NULL) {
					// Buy new server
					auto purchaseInfo = buyServer(T + 1);
                    if (purchaseInfo.second[0] == -2) {
                        R++;
                        continue;
                    }
                    else {
                        string buySrvName = purchaseInfo.first;
                        ProcuredServer* ps = new ProcuredServer(servers[buySrvName]);
                        bgtServers.push_back(ps);
                        newServers[ps->server.Name()]++;
                        int reqNum = addVMReq(purchaseInfo.second);
                        R++;
                        R -= reqNum;

                        eachday += servers[ps->server.Name()].DayCost();
                        money += servers[ps->server.Name()].HwCost();
                    }
				}
				else {
					// Add vm to the current server
					userVms[req.VmID()] = UserVM(vms[req.VmName()], req.VmID());
					userVms[req.VmID()].Deploy(dplWay.first, dplWay.second);
					requests.pop_front();
				}
			}
			else {
                auto it = userVms.find(req.VmID());
				// Delete it !
                UserVM uv = userVms[req.VmID()];
				userVms[req.VmID()].DelVM();
				auto _it = userVms.find(req.VmID());
				userVms.erase(_it);
				requests.pop_front();
			}
		}
        // 对新购入的服务器进行编号赋值
        creatServerID();
        cout << "(purchase, " << newServers.size() << ")" << endl;
        for (auto& newServer : newServers) {
            cout << "(" << newServer.first << ", " << newServer.second << ")" << endl;
        }
        cout << "(migration, 0)" << endl;
        newVMsInfoToStr();
        for (int i = 0; i < newVMsInfo.size(); i++) {
            cout << newVMsInfo[i] << endl;
        }
        cal();
	}// 输出购买成本和服务器数量
    // cout << money << " " << bgtServers.size();
	return 0;
}


void newVMsInfoToStr() {
    for (auto& vm : newVMs) {
        int s_id = bgtServers[vm.first]->ServerID();
        string str = "(" + to_string(s_id);
        str += vm.second == DblNode ? ")" : (vm.second == ANode ? ", A)" : ", B)");
        newVMsInfo.push_back(str);
    }
}

int addVMReq(vector<int>& info)
{
    int retReqNum = 0;  // 处理的请求数量
    ProcuredServer* aimServer = bgtServers[bgtServers.size() - 1]; // 放入的目标服务器
    int aimSrvIdx = bgtServers.size() - 1;
    for (int num = 0; num < info.size(); num++) {
        auto req = requests.front();
        // 若当前操作为删除操作, 执行del()
        if (req.Opt() == DelReq) {
            userVms[req.VmID()].DelVM();
            auto it = userVms.find(req.VmID());
            if (it != userVms.end()) {
                userVms.erase(it);
                requests.pop_front();
            }
            retReqNum++;
            num--;
            continue;
        }
        // add请求处理
        VM vm = vms[req.VmName()];
        DeployedNode dplNode;
        if (vm.isDouNode()) dplNode = DblNode;
        else if (info[num] == -1)   dplNode = ANode;
        else dplNode = BNode;
        userVms[req.VmID()] = UserVM(vm, req.VmID());
        userVms[req.VmID()].Deploy(aimServer, dplNode);
        newVMs.push_back(make_pair(aimSrvIdx, dplNode)); // 存入vm在server配置的信息
        requests.pop_front();
        retReqNum++;
    }
    return retReqNum;
}

void creatServerID()
{
    int num = 0;
    for (auto& ns : newServers) {
        num = ns.second;
        for (int i = bgtServers.size() - 1; i >= 0; i--) {
            // 从尾部向前找对应名字的服务器
            while (bgtServers[i]->server.Name() != ns.first)    i--;
            bgtServers[i]->setServerID(curLastSrvID);
            curLastSrvID++; num--;
            if (num == 0) {
                break;
            }
        }
    }
}

bool isFit(int remainCores, int remainMem) {
    remainCores++; remainMem++;
    // 判定函数
    if ((remainCores < 5 && remainMem > 45) || (remainCores > 45 && remainMem < 5))  return false;
    if ((remainCores < 10 && remainMem > 150) || (remainCores > 150 && remainMem < 10))  return false;
    return true;
}

bool canPutIn(ProcuredServer* bs, VM& vm, DeployedNode dpn) {
    int cEach = vm.Cores() / (dpn == DblNode ? 2 : 1),
        mEach = vm.Memory() / (dpn == DblNode ? 2 : 1);
    int A_rc = bs->RmnCoresA(), A_rm = bs->RmnMemA(),
        B_rc = bs->RmnCoresB(), B_rm = bs->RmnMemB();
    int ac = A_rc - cEach, am = A_rm - mEach,
        bc = B_rc - cEach, bm = B_rm - mEach;
    // 根据不同部署方式判断部署的合理性
    switch (dpn) {
    case DblNode:
        if (A_rc >= cEach && B_rc >= cEach && A_rm >= mEach && B_rm >= mEach && isFit(ac, am) && isFit(bc, bm)) {
            return true;
        }
        break;
    case ANode:
        if (A_rc >= cEach && A_rm >= mEach && isFit(ac, am)) {
            return true;
        }
        break;
    case BNode:
        if (B_rc >= cEach && B_rm >= mEach && isFit(bc, bm)) {
            return true;
        }
        break;
    }
    return false;
}

pair<ProcuredServer*, DeployedNode> DeployRequest(Request& req)
{
    VM vm = vms[req.VmName()];
    bool isDouble = vm.isDouNode();
    vector<pair<int, DeployedNode>> temp;    // 可放入的服务器<serverIdx, way>
    for (int i = 0; i < bgtServers.size(); i++) {
        if (vm.isDouNode()) {
            if (canPutIn(bgtServers[i], vm, DblNode)) {
                temp.push_back(make_pair(i, DblNode));
            }
        }
        else {
            if (canPutIn(bgtServers[i], vm, ANode)) {
                temp.push_back(make_pair(i, ANode));
            }
            if (canPutIn(bgtServers[i], vm, BNode)) {
                temp.push_back(make_pair(i, BNode));
            }
        }
    }

    if (isDouble)   return dblSrvChoice(temp);
    else            return sglSrvChoice(temp, (double)vm.Cores() / (double)vm.Memory());
}

pair<ProcuredServer*, DeployedNode> dblSrvChoice(vector<pair<int, DeployedNode>>& temp)
{
    pair<ProcuredServer*, DeployedNode> ret(NULL, DblNode);
    // 算法部分, 在所有能放下当前vm的servers中选择最合适的
    if (temp.size() > 0) {
        // The most suitable choice
        int pos = temp[0].first;
        int c = min(bgtServers[temp[0].first]->RmnCoresA(), bgtServers[temp[0].first]->RmnCoresB());
        int m = min(bgtServers[temp[0].first]->RmnMemA(), bgtServers[temp[0].first]->RmnMemB());
        for (int i = 1; i < temp.size(); i++) {
            int cp = min(bgtServers[temp[i].first]->RmnCoresA(), bgtServers[temp[i].first]->RmnCoresB());
            int mp = min(bgtServers[temp[i].first]->RmnMemA(), bgtServers[temp[i].first]->RmnMemB());
            if (mp + cp < m + c) {
                pos = temp[i].first;
                c = cp;
                m = mp;
            }
        }
        ret.first = bgtServers[pos];
        newVMs.push_back(make_pair(pos, DblNode));  // 存入vm在server配置的信息
    }
    return ret;
}

bool cmp(const pair<int, int>& a, const pair<int, int>& b)
{
    double cmpA, cmpB;
    cmpA = a.second == 0 ? bgtServers[a.first]->SpecificA() : bgtServers[a.first]->SpecificB();
    cmpB = b.second == 0 ? bgtServers[a.first]->SpecificA() : bgtServers[a.first]->SpecificB();
    return cmpA < cmpB;
}
pair<ProcuredServer*, DeployedNode> sglSrvChoice(vector<pair<int, DeployedNode>>& temp, double ratio)
{
    pair<ProcuredServer*, DeployedNode> ret(NULL, DblNode);
    // 如果没有合适的位置，返回false
    if (temp.size() == 0)   return ret;
    sort(temp.begin(), temp.end(), cmp);

    double cha = 10000;
    int idx;
    for (int i = 0; i < temp.size(); i++) {
        double _cha = cha;
        if (temp[i].second == ANode)
            cha = min(cha, fabs(ratio - bgtServers[temp[i].first]->SpecificA()));
        else
            cha = min(cha, fabs(ratio - bgtServers[temp[i].first]->SpecificB()));
        if (cha != _cha)    idx = i;
    }
    auto aimNode = temp[idx];
    ret.first = bgtServers[aimNode.first];
    ret.second = aimNode.second;
    newVMs.push_back(make_pair(aimNode.first, ret.second)); // 存入vm在server配置的信息
    return ret;
}

/*
  服务器选择算法  
*/
string _buy() {
    int n = servers.size();
    int r_n = rand() % (0 - n);
    for (auto& server : servers) {
        if (r_n == 0) {
            return server.first;
        }
        r_n--;
    }
    return "";
}

pair<string, vector<int>> buyServer(int day) {
    pair<string, vector<int>> ans;
    int lc = 0;
    int lm = 0;
    int rc = 0;
    int rm = 0;
    vector<int> pos;
    vector<string> ser;
    ser.clear();
    vector<string> temp;
    int tempPos;
    vector<Request> tempReq;
    for (list<Request>::iterator it = requests.begin(); it != requests.end(); ++it) {
        if (it->Opt() == DelReq)
            continue;
        tempReq.push_back(*it);
        int c = vms[it->VmName()].Cores();
        int m = vms[it->VmName()].Memory();
        bool t = vms[it->VmName()].isDouNode();
        if (t) {
            tempPos = 0;
            lc += (c / 2);
            rc += (c / 2);
            lm += (m / 2);
            rm += (m / 2);
        }
        else if (lc <= rc && lm <= rm) {
            tempPos = -1;
            lc += c;
            lm += m;
        }
        else  if (lc > rc && lm > rm) {
            tempPos = 1;
            rc += c;
            rm += m;
        }
        else {
            break;
        }
        for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
            if (it->second.Cores() / 2 < max(lc, rc) || it->second.Memory() / 2 < max(lm, rm)
                || !isFit(it->second.Cores() / 2 - lc, it->second.Memory() / 2 - lm)
                || !isFit(it->second.Cores() / 2 - rc, it->second.Memory() / 2 - rm))
                continue;
            temp.push_back(it->second.Name());
        }
        if (temp.size() == 0)
            break;
        ser = temp;
        temp.clear();
        pos.push_back(tempPos);
    }
    if (pos.size() <= 3) {
        string ansSer;
        if (ser.size() == 0) {
            string srtname = _buy();
            ans.first = srtname;
            pos.push_back(1);
            pos[0] = -2;
            ans.second = pos;
            return ans;
        }
        for (int minCost = INT32_MAX, i = 0; i < ser.size(); i++) {
            if (minCost > servers[ser[i]].HwCost() + servers[ser[i]].DayCost() * day) {
                minCost = servers[ser[i]].HwCost() + servers[ser[i]].DayCost() * day;
                ansSer = ser[i];
            }
        }
        ans.first = ansSer;
        ans.second = pos;
        return ans;
    }
    else {//////////////////////
        string ansSer;
        int n = pos.size();
        vector<int> pos1;
        vector<int> pos2;
        string s1;
        string s2;
        int minc = INT32_MAX;
        int minc1 = INT32_MAX;
        int minc2 = INT32_MAX;
        string mins;
        vector<int> minp;
        vector<string> temp1;
        vector<string> temp2;
        for (int i = 2; i <= n / 2; i++) {
            int lc1 = 0;
            int lm1 = 0;
            int rc1 = 0;
            int rm1 = 0;
            int lc2 = 0;
            int lm2 = 0;
            int rc2 = 0;
            int rm2 = 0;
            for (int j = 0; j < i; j++) {
                int c = vms[tempReq[j].VmName()].Cores();
                int m = vms[tempReq[j].VmName()].Memory();
                bool t = vms[tempReq[j].VmName()].isDouNode();
                if (t) {
                    tempPos = 0;
                    lc1 += (c / 2);
                    rc1 += (c / 2);
                    lm1 += (m / 2);
                    rm1 += (m / 2);
                }
                else if (lc <= rc && lm <= rm) {
                    tempPos = -1;
                    lc1 += c;
                    lm1 += m;
                }
                else {
                    tempPos = 1;
                    rc1 += c;
                    rm1 += m;
                }
                pos1.push_back(tempPos);
            }
            for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
                if (it->second.Cores() / 2 < max(lc1, rc1) || it->second.Memory() / 2 < max(lm1, rm1)
                    || !isFit(it->second.Cores() / 2 - lc1, it->second.Memory() / 2 - lm1)
                    || !isFit(it->second.Cores() / 2 - rc1, it->second.Memory() / 2 - rm1))
                    continue;
                temp1.push_back(it->second.Name());
            }
            for (int minc1 = INT32_MAX, i = 0; i < temp1.size(); i++) {
                if (minc1 > servers[temp1[i]].HwCost() + servers[temp1[i]].DayCost() * day) {
                    minc1 = servers[temp1[i]].HwCost() + servers[temp1[i]].DayCost() * day;
                    s1 = temp1[i];
                }
            }
            for (int j = i; j < n; j++) {
                int c = vms[tempReq[j].VmName()].Cores();
                int m = vms[tempReq[j].VmName()].Memory();
                bool t = vms[tempReq[j].VmName()].isDouNode();
                if (t) {
                    tempPos = 0;
                    lc2 += (c / 2);
                    rc2 += (c / 2);
                    lm2 += (m / 2);
                    rm2 += (m / 2);
                }
                else if (lc <= rc && lm <= rm) {
                    tempPos = -1;
                    lc2 += c;
                    lm2 += m;
                }
                else {
                    tempPos = 1;
                    rc2 += c;
                    rm2 += m;
                }
                pos2.push_back(tempPos);
            }
            for (unordered_map<string, Server>::iterator it = servers.begin(); it != servers.end(); it++) {
                if (it->second.Cores() / 2 < max(lc1, rc1) || it->second.Memory() / 2 < max(lm1, rm1)
                    || !isFit(it->second.Cores() / 2 - lc1, it->second.Memory() / 2 - lm1)
                    || !isFit(it->second.Cores() / 2 - rc1, it->second.Memory() / 2 - rm1))
                    continue;
                temp2.push_back(it->second.Name());
            }
            for (int minc2 = INT32_MAX, i = 0; i < temp2.size(); i++) {
                if (minc1 > servers[temp2[i]].HwCost() + servers[temp2[i]].DayCost() * day) {
                    minc1 = servers[temp2[i]].HwCost() + servers[temp2[i]].DayCost() * day;
                    s2 = temp2[i];
                }
            }
            if (minc > minc1 + minc2) {
                minc = minc1 + minc2;
                mins = s1;
                minp = pos1;
            }
        }
        ans.first = mins;
        ans.second = minp;
        return ans;
    }
} 