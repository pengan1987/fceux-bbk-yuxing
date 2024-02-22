#include "M179Base.h"

static BMAPPINGLocal* createMapper(int num)
{
	if (MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = new DrPCCreate();
	static BMAPPINGLocal ss;
	ss.init = MapperBase::s_Mapper_Init;
	ss.number = 179;
	ss.name = "YuXing";
	return &ss;
}
static MAP_Mapper& d = AddMapper(179, createMapper);
