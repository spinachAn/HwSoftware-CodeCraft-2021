#ifndef _DATA_READING_H_
#define	_DATA_READING_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "configuration.h"
using namespace std;

int fReadNum(ifstream& file);

int fReadSrvNum(ifstream& file);

int fReadVMNum(ifstream& file);

int fReadDays(ifstream& file);

int fReadReqCnt(ifstream& file);

Server	fReadSrv(ifstream& file);

VM		fReadVM(ifstream& file);

Request fReadReq(ifstream& file);

vector<string> split(const string& s);

#endif // !_DATA_READING_H_