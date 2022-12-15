#include <stdlib.h>
#include <string.h>
#include <confuse.h>

#include "config.h"
struct ConfigStruct configure( char* ConfigPath ) {
	static cfg_bool_t Daemonize = cfg_true;
	static char *XServerPath = "/usr/bin/Xorg";
	static char *XSessionPath = NULL;
	
	cfg_opt_t ConfuseOptions[] = {
		CFG_SIMPLE_BOOL( "Daemonize", &Daemonize ),
		CFG_SIMPLE_STR( "XServerPath" , &XServerPath ),
		CFG_STR_LIST( "XServerArgs", "", CFGF_NONE ),
		CFG_SIMPLE_STR( "XSessionPath" , &XSessionPath ),
		CFG_STR_LIST( "XSessionArgs", "", CFGF_NONE ),
		CFG_END()
	};
	
	cfg_t *config;
	config = cfg_init(ConfuseOptions, 0);
	if( cfg_parse( config, ConfigPath ) != CFG_SUCCESS ) {
		struct ConfigStruct ErrorConfig;
		ErrorConfig.ErrorCode = ULDM_CFG_FILEERROR;
		return ErrorConfig;
	}
	struct ConfigStruct Configuration;
	
	Configuration.Daemonize = (int)Daemonize;
	
	Configuration.XServerPath = malloc( strlen(XServerPath)+1 );
	strcpy( Configuration.XServerPath, XServerPath );
	
	Configuration.XSessionPath = malloc( strlen(XSessionPath)+1 );
	strcpy( Configuration.XSessionPath, XSessionPath );
	
	int XServerArgIncrementor;
	char XServerArgvPrep[ArgsY][ArgsX];
	strncpy( XServerArgvPrep[0], XServerPath, ArgsX );
	for( XServerArgIncrementor = 1; XServerArgIncrementor < cfg_size(config, "XServerArgs") && XServerArgIncrementor <= ArgsY; XServerArgIncrementor++) {
		strncpy( XServerArgvPrep[XServerArgIncrementor], cfg_getnstr(config, "targets", XServerArgIncrementor), ArgsX );
	}
	
	int XSessionArgIncrementor;
	char XSessionArgvPrep[ArgsY][ArgsX];
	strncpy( XSessionArgvPrep[0], XSessionPath, ArgsX );
	for( XSessionArgIncrementor = 1; XSessionArgIncrementor < cfg_size(config, "XServerArgs") && XSessionArgIncrementor <= ArgsY; XSessionArgIncrementor++) {
		strncpy( XSessionArgvPrep[XServerArgIncrementor], cfg_getnstr(config, "targets", XSessionArgIncrementor), ArgsX );
	}



	Configuration.XServerArgs = malloc( XServerArgIncrementor * sizeof( Configuration.XServerArgs ) );
	int XServerIncrementor;
	for ( int XServerIncrementor = 0; XServerIncrementor < XServerArgIncrementor; XServerIncrementor++ ) {

		strcpy( Configuration.XServerArgs[XServerIncrementor], XServerArgvPrep[XServerIncrementor] );
	}
	
	Configuration.XSessionArgs = malloc( XSessionArgIncrementor * sizeof( Configuration.XSessionArgs ) );
	int XSessionIncrementor;
	for ( int XSessionIncrementor = 0; XSessionIncrementor < XSessionArgIncrementor; XSessionIncrementor++ ) {
		//sausage code
		strcpy( Configuration.XSessionArgs[XSessionIncrementor], XSessionArgvPrep[XSessionIncrementor] );
	};
	
	Configuration.ErrorCode = ULDM_CFG_SUCCESS;
		
	return Configuration;
}