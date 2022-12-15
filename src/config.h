#define ArgsX 32
#define ArgsY 8

#define ULDM_CFG_SUCCESS 0;
#define ULDM_CFG_FILEERROR -1;

struct ConfigStruct{
	int ErrorCode;
	int Daemonize;
	char *XServerPath;
	char **XServerArgs;
	char *XSessionPath;
	char **XSessionArgs;
};

struct ConfigStruct configure( char* ConfigPath );