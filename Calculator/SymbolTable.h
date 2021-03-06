#ifndef _SYMBOLTABLE_H_
#define _SYMBOLTABLE_H_
#include <map>
#include <string>
#include "Serial.h"
class SymbolTable : public Serializable
{
public:
	enum
	{
		ID_NOT_FIND = 0xffffffff
	};

public:
	SymbolTable() :curId_(0){}
	~SymbolTable();
	unsigned int Add(const std::string& str);
	unsigned int Find(const std::string& str) const;
	void Clear();
	std::string GetSymbolName(unsigned int id) const;
	unsigned int GetCurId() const;

public:
	void Serialize(Serializer& out) const;
	void DeSerialize(DeSerializer& in);

private:
	std::map<const std::string, unsigned int> dictionary_;
	unsigned int curId_;
};

#endif _SYMBOLTABLE_H_
