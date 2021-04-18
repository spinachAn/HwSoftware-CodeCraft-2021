#include "configuration.h"
#include <iostream>
Server::Server(const std::string& _name, const int& _cores, const int& _mem, const int& _hwCost, const int& _dCost) :
	name(_name), cores(_cores), memory(_mem), hardwareCost(_hwCost), dailyCost(_dCost)
{
}


VM::VM(const std::string& _name, const int& _cores, const int& _mem, const bool _isDouble) :
	name(_name), cores(_cores), memory(_mem), isDouble(_isDouble)
{
}


ProcuredServer::ProcuredServer(Server _server) : server(_server)
{
	this->serverID = -1;
	this->ARemainCores = _server.Cores() / 2;
	this->BRemainCores = _server.Cores() / 2;
	this->ARemainMemory = _server.Memory() / 2;
	this->BRemainMemory = _server.Memory() / 2;
	this->ChangeSpecific();
}

void ProcuredServer::setServerID(int id)
{
	this->serverID = id;
}

bool ProcuredServer::AddVM(VM& vm, const DeployedNode& dn)
{
	int ac = 0, bc = 0, am = 0, bm = 0;
	switch (dn) {
	case DblNode:
		ac = vm.Cores() / 2;
		bc = vm.Cores() / 2;
		am = vm.Memory() / 2;
		bm = vm.Memory() / 2;
		break;
	case ANode:
		ac = vm.Cores();
		am = vm.Memory();
		break;
	case BNode:
		bc = vm.Cores();
		bm = vm.Memory();
		break;
	default:
		return false;
	}
	if (this->ARemainCores - ac >= 0 && this->ARemainMemory - am >= 0) {
		this->ARemainCores -= ac;
		this->ARemainMemory -= am;
	}
	else {
		return false;
	}
	if (this->BRemainCores - bc >= 0 && this->BRemainMemory - bm >= 0) {
		this->BRemainCores -= bc;
		this->BRemainMemory -= bm;
	}
	else {
		return false;
	}
	this->ChangeSpecific();
	return true;
}

bool ProcuredServer::DelVM(VM& vm, const DeployedNode& dn)
{
	int ac = 0, bc = 0, am = 0, bm = 0;
	switch (dn) {
	case DblNode:
		ac = vm.Cores() / 2;
		bc = vm.Cores() / 2;
		am = vm.Memory() / 2;
		bm = vm.Memory() / 2;
		break;
	case ANode:
		ac = vm.Cores();
		am = vm.Memory();
		break;
	case BNode:
		bc = vm.Cores();
		bm = vm.Memory();
		break;
	default:
		return false;
	}
	this->ARemainCores += ac;
	this->ARemainMemory += am;
	this->BRemainCores += bc;
	this->BRemainMemory += bm;
	this->ChangeSpecific();
	return true;
}


UserVM::UserVM(VM _vm, uint32_t _vmID) : vm(_vm), vmID(_vmID)
{
}

bool UserVM::Deploy(ProcuredServer* ps, DeployedNode dpn)
{
	this->deployedServer = ps;
	this->nodePos = dpn;
	return this->deployedServer->AddVM(this->vm, this->nodePos);
}

bool UserVM::DelVM()
{
	string s = this->vm.Name();
	return this->deployedServer->DelVM(this->vm, this->nodePos);
}

Request::Request(const ReqOpt& _opt, const std::string& _vmName, const uint32_t _vmID) :
	opt(_opt), vmName(_vmName), vmID(_vmID)
{
}

Request::Request(const ReqOpt& _opt, const uint32_t _vmID) :
	opt(_opt), vmID(_vmID)
{
	vmName = "";
} 