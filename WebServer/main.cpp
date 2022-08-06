#include "cofig.h"

int main (int argc,char *argv[]) {


	string user ="root";
	string passwd ="9638527410";
	string databasename ="lydb";


	Config config;
	config.parse_arg(argc,argv);

	WebServer server;

	server.init(config.PORT,user,passwd,databasename,config.LOGWrite,
				config.OPT_LINGER,config.TRIGMode,config.sql_num,config.thread_num,
				config.close_log,config.actor_model);

	server.log_wirte();

	server.sql_pool();

	server.thread_pool();

	server.trig_mode();

	server.eventListen();

	server.eventLoop();

	return 0;
}