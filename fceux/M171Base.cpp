#include "M171Base.h"


static BMAPPINGLocal* createMapper(int num)
{
	if (MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = new BBK10Create();
	static BMAPPINGLocal ss;
	ss.init = MapperBase::s_Mapper_Init;
	ss.number = 171;
	ss.name = "BBK";
	return &ss;
}
static MAP_Mapper& d = AddMapper(171, createMapper);
