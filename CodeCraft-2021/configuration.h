// 服务器和虚拟机类
// 用户购买的服务器类和购买的虚拟机类
// 请求指令类
#ifndef _CONFIGURATION_H
#define _CONFIGURATION_H

#include <string>
using std::string;
/* 部署方式: 双节点部署, A节点部署, B节点部署 */
typedef enum { DblNode, ANode, BNode } DeployedNode;

/* 请求指令的操作类别 */
typedef enum { AddReq, DelReq } ReqOpt;

class Server
{
public:
	Server() {}
	~Server() {}
	Server(const std::string& _name, const int& _cores, const int& _mem, const int& _hwCost, const int& _dCost);
	std::string Name() { return this->name; }
	inline int	Cores() { return this->cores; }
	inline int	Memory() { return this->memory; }
	inline int	HwCost() { return this->hardwareCost; }
	inline int	DayCost() { return this->dailyCost; }

private:
	std::string name;			// 服务器名称
	int			cores;			// 服务器核心数
	int			memory;			// 服务器内存容量
	int			hardwareCost;	// 服务器硬件成本
	int			dailyCost;		// 服务器每日运行开销
};

class VM
{
public:
	VM() {}
	~VM() {}
	VM(const string& _name, const int& _cores, const int& _mem, const bool _isDouble);
	inline string	Name() { return this->name; }
	inline int		Cores() { return this->cores; }
	inline int		Memory() { return this->memory; }
	inline bool		isDouNode() { return this->isDouble; }

private:
	string		name;
	int			cores;
	int			memory;
	bool		isDouble;
};

class ProcuredServer
{
public:
	ProcuredServer() {}
	~ProcuredServer() {}
	ProcuredServer(Server _server);
	inline int	ServerID() { return this->serverID; }
	inline int	RmnCoresA() { return this->ARemainCores; }
	inline int	RmnCoresB() { return this->BRemainCores; }
	inline int	RmnMemA() { return this->ARemainMemory; }
	inline int	RmnMemB() { return this->BRemainMemory; }
	double		SpecificA() { return this->ASpecific; }
	double		SpecificB() { return this->BSpecific; }

	void		setServerID(int id);
	bool		AddVM(VM& vm, const DeployedNode& dn);
	bool		DelVM(VM& vm, const DeployedNode& dn);
private:
	inline void	ChangeSpecific() {
		this->ASpecific = (double)this->ARemainCores / (double)this->ARemainMemory;
		this->BSpecific = (double)this->BRemainCores / (double)this->BRemainMemory;
	}

public:
	Server		server;			// 所属服务器种类
private:
	int		    serverID;		// 服务器的ID号
	int			ARemainCores;
	int			BRemainCores;
	int		    ARemainMemory;
	int		    BRemainMemory;
	double		ASpecific;		// A结点核心内存之比
	double		BSpecific;		// B结点核心内存之比
};

class UserVM
{
public:
	UserVM() {}
	~UserVM() {}
	UserVM(VM _vm, uint32_t _vmID);
	bool Deploy(ProcuredServer* ps, DeployedNode dpn);
	bool DelVM();

public:
	VM				vm;			// 所属虚拟机种类
	ProcuredServer* deployedServer;	// 部署的服务器
private:
	uint32_t		vmID;		// 虚拟机ID
	DeployedNode    nodePos;	// 部署的方式
};

class Request
{
public:
	Request() {}
	~Request() {}
	Request(const ReqOpt& _opt, const string& _vmName, const uint32_t _vmID);
	Request(const ReqOpt& _opt, const uint32_t _vmID);
	inline ReqOpt	Opt() { return this->opt; }
	inline string	VmName() { return this->vmName; }
	inline uint32_t	VmID() { return this->vmID; }

private:
	ReqOpt			opt;
	string			vmName;
	uint32_t		vmID;
};




#endif // !_CONFIGURATION_H 