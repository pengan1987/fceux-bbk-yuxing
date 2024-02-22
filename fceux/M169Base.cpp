#include "M169Base.h"

static BMAPPINGLocal* createMapper(int num)
{
	if (MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = new YuXingCreate();
	static BMAPPINGLocal ss;
	ss.init = MapperBase::s_Mapper_Init;
	ss.number = 169;
	ss.name = "YuXing";
	return &ss;
}
static MAP_Mapper& d = AddMapper(169, createMapper);
