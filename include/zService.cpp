#include "engine.h"


zService *zService::serviceInst = NULL;

bool zService::init()
{
	Zebra::logger->debug("zService::init");
	//��ʼ�������
	srand(time(NULL));

	return true;

}


void zService::main()
{
	Zebra::logger->debug("zService::main");

	if (init() && validate())
	{
		while (!isTerminate())
		{
			if (!serviceCallback())
			{
				break;
			}
		}
	}

	final();
}