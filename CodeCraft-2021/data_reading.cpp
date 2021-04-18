#include "data_reading.h"
#include <sstream>

int fReadNum(ifstream& file)
{
	string s;
	getline(file, s);
	return stoi(s);
}

int fReadSrvNum(ifstream& file)
{
	return fReadNum(file);
}

int fReadVMNum(ifstream& file)
{
	return fReadNum(file);
}

int fReadDays(ifstream& file)
{
	return fReadNum(file);
}

int fReadReqCnt(ifstream& file)
{
	return fReadNum(file);
}

Server fReadSrv(ifstream& file)
{
	string s;
	getline(file, s);
	vector<string>&& data = split(s);
	return Server(data[0], stoi(data[1]), stoi(data[2]), stoi(data[3]), stoi(data[4]));
}

VM fReadVM(ifstream& file)
{
	string s;
	getline(file, s);
	vector<string>&& data = split(s);
	return VM(data[0], stoi(data[1]), stoi(data[2]), (bool)stoi(data[3]));
}

Request fReadReq(ifstream& file)
{
	string s;
	getline(file, s);
	vector<string>&& data = split(s);
	ReqOpt ro = data[0] == "add" ? AddReq : (data[0] == "del" ? DelReq : (ReqOpt)-1);
	return ro == AddReq ? Request(ro, data[1], uint32_t(stoll(data[2]))) : Request(ro, uint32_t(stoll(data[1])));
}

vector<string> split(const string& s)
{
	string str = s.substr(1, s.size() - 2);
	vector<string> ret;
	int idx = 1;
	do {
		if (str[idx] == ',')	str[idx] = ' ';
	} while (++idx != str.size() - 1);
	string data;
	istringstream strs(str);
	while (strs >> data)
		ret.push_back(data);
	return ret;
} 