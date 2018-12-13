/* cheatcoin main, T13.654-T14.582 $DVS:time$ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#if !defined(_WIN32) && !defined(_WIN64)
#include <signal.h>
#endif
#include "system.h"
#include "address.h"
#include "block.h"
#include "crypt.h"
#include "global.h"
#include "transport.h"
#include "version.h"
#include "wallet.h"
#include "netdb.h"
#include "init.h"
#include "sync.h"
#include "mining_common.h"
#include "commands.h"
#include "terminal.h"
#include "memory.h"
#include "miner.h"
#include "pool.h"
#include "network.h"
#include "utils/log.h"
#include "utils/utils.h"
#include "xdag_config.h"
#include "json-rpc/rpc_service.h"
#include "../dnet/dnet_crypt.h"

#define ARG_EQUAL(a,b,c) strcmp(c, "") == 0 ? strcmp(a, b) == 0 : (strcmp(a, b) == 0 || strcmp(a, c) == 0)

struct startup_parameters {
	const char *addr_ports[256];
	int addrports_count;
	char *bind_to;
	char *pool_address;
	char *miner_address;
	int transport_flags;
	int transport_threads;
	int mining_threads_count;
	int is_pool;
	int is_miner;
	int is_rpc;
	int rpc_port;
	struct pool_configuration pool_configuration;
};

time_t g_xdag_xfer_last = 0;

int(*g_xdag_show_state)(const char *state, const char *balance, const char *address) = 0;

int parse_startup_parameters(int argc, char **argv, struct startup_parameters *parameters);
int pre_init(void);
int setup_miner(struct startup_parameters *parameters);
int setup_pool(struct startup_parameters *parameters);
int dnet_key_init(void);
void printUsage(char* appName);
void set_xdag_name(void);

int xdag_init(int argc, char **argv, int isGui)
{
	xdag_init_path(argv[0]);

#if !defined(_WIN32) && !defined(_WIN64)
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGWINCH, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#endif

	set_xdag_name();

	if(!isGui) {
		printf("%s client/server, version %s.\n", g_progname, XDAG_VERSION);
	}

	g_xdag_run = 1;
	xdag_show_state(0);

	struct startup_parameters parameters;
	int res = parse_startup_parameters(argc, argv, &parameters);
	if(res <= 0) {
		return res;
	}

	if(pre_init() < 0) {
		return -1;
	}

	if(is_wallet()) {
		if(setup_miner(&parameters) < 0) {
			return -1;
		}
	} else {
		if(setup_pool(&parameters) < 0) {
			return -1;
		}
	}

	if(!isGui) {
		if(is_pool() || (parameters.transport_flags & XDAG_DAEMON) > 0) {
			xdag_mess("Starting terminal server...");
			pthread_t th;
			const int err = pthread_create(&th, 0, &terminal_thread, 0);
			if(err != 0) {
				printf("create terminal_thread failed, error : %s\n", strerror(err));
				return -1;
			}
		}

		startCommandProcessing(parameters.transport_flags);
	}

	return 0;
}

int parse_startup_parameters(int argc, char **argv, struct startup_parameters *parameters)
{
	memset(parameters, 0, sizeof(struct startup_parameters));
	parameters->transport_threads = -1;
	int log_level;

	for(int i = 1; i < argc; ++i) {
		if(argv[i][0] != '-') {
			if((!argv[i][1] || argv[i][2]) && strchr(argv[i], ':')) {
				parameters->pool_address = argv[i];
			} else {
				printUsage(argv[0]);
				return 0;
			}
			continue;
		}
		if(ARG_EQUAL(argv[i], "-f", "")) { /* configuration file */
			if(parameters->pool_configuration.node_address != NULL || parameters->pool_configuration.mining_configuration != NULL) {
				printUsage(argv[0]);
				return 0;
			}
			const char *config_path = NULL;
			if(i + 1 < argc && argv[i + 1] != NULL && argv[i + 1][0] != '-') {
				config_path = argv[++i];
			}
			if(get_pool_config(config_path, &parameters->pool_configuration) < 0) return -1;
		} else if(ARG_EQUAL(argv[i], "-a", "")) { /* miner address */
			if(++i < argc) {
				parameters->miner_address = argv[i];
			}
		} else if(ARG_EQUAL(argv[i], "-c", "")) { /* another full node address */
			if(++i < argc && parameters->addrports_count < 256) {
				parameters->addr_ports[parameters->addrports_count++] = argv[i];
			}
		} else if(ARG_EQUAL(argv[i], "-d", "")) { /* daemon mode */
#if !defined(_WIN32) && !defined(_WIN64)
			parameters->transport_flags |= XDAG_DAEMON;
#endif
		} else if(ARG_EQUAL(argv[i], "-h", "")) { /* help */
			printUsage(argv[0]);
			return 0;
		} else if(ARG_EQUAL(argv[i], "-i", "")) { /* interactive mode */
			return terminal();
		} else if(ARG_EQUAL(argv[i], "-z", "")) { /* memory map  */
			if(++i < argc) {
				xdag_mem_tempfile_path(argv[i]);
			}
		} else if(ARG_EQUAL(argv[i], "-t", "")) { /* connect test net */
			g_xdag_testnet = 1;
			g_block_header_type = XDAG_FIELD_HEAD_TEST; //block header has the different type in the test network
		} else if(ARG_EQUAL(argv[i], "-m", "")) { /* mining thread number */
			if(++i < argc) {
				sscanf(argv[i], "%d", &parameters->mining_threads_count);
				if(parameters->mining_threads_count < 0) parameters->mining_threads_count = 0;
			}
		} else if(ARG_EQUAL(argv[i], "-p", "")) { /* public address & port */
			if(parameters->pool_configuration.node_address != NULL) {
				printUsage(argv[0]);
				return 0;
			}
			if(++i < argc) {
				parameters->is_pool = 1;
				parameters->pool_configuration.node_address = argv[i];
			}
		} else if(ARG_EQUAL(argv[i], "-P", "")) { /* pool config */
			if(++i < argc) {
				parameters->pool_configuration.mining_configuration = argv[i];
			}
		} else if(ARG_EQUAL(argv[i], "-r", "")) { /* load blocks and wait for run command */
			g_xdag_run = 0;
		} else if(ARG_EQUAL(argv[i], "-s", "")) { /* address of this node */
			if(++i < argc)
				parameters->bind_to = argv[i];
		} else if(ARG_EQUAL(argv[i], "-v", "")) { /* log level */
			if(++i < argc && sscanf(argv[i], "%d", &log_level) == 1) {
				xdag_set_log_level(log_level);
			} else {
				printf("Illevel use of option -v\n");
				return -1;
			}
		} else if(ARG_EQUAL(argv[i], "", "-rpc-enable")) { /* enable JSON-RPC service */
			parameters->is_rpc = 1;
		} else if(ARG_EQUAL(argv[i], "", "-rpc-port")) { /* set JSON-RPC service port */
			if(!(++i < argc && sscanf(argv[i], "%d", &parameters->rpc_port) == 1)) {
				printf("rpc port not specified.\n");
				return -1;
			}
		} else if(ARG_EQUAL(argv[i], "", "-threads")) { /* number of transport layer threads */
			if(!(++i < argc && sscanf(argv[i], "%d", &parameters->transport_threads) == 1))
				printf("Number of transport threads is not given.\n");
		} else if(ARG_EQUAL(argv[i], "", "-dm")) { /* disable mining */
			g_disable_mining = 1;
			//} else if(ARG_EQUAL(argv[i], "", "-tag")) { /* pool tag */
			//	if(i + 1 < argc) {
			//		if(validate_remark(argv[i + 1])) {
			//			memcpy(g_pool_tag, argv[i + 1], strlen(argv[i + 1]));
			//			g_pool_has_tag = 1;
			//			++i;
			//		} else {
			//			printf("Pool tag exceeds 32 chars or is invalid ascii.\n");
			//			return -1;
			//		}
			//	} else {
			//		printUsage(argv[0]);
			//		return -1;
			//	}
		} else if(ARG_EQUAL(argv[i], "", "-disable-refresh")) { /* disable auto refresh white list */
			g_prevent_auto_refresh = 1;
		} else if(ARG_EQUAL(argv[i], "-l", "")) { /* list balance */
			return out_balances();
		} else {
			printUsage(argv[0]);
			return 0;
		}
	}

	if(parameters->pool_address != NULL && (parameters->pool_configuration.node_address != NULL || parameters->pool_configuration.mining_configuration != NULL)) {
		printf("{0} must be started either as pool or as wallet.\n", argv[0]);
		return -1;
	}

	if(parameters->pool_configuration.node_address != NULL || parameters->pool_configuration.mining_configuration != NULL) {
		if(parameters->pool_configuration.node_address == NULL || parameters->pool_configuration.mining_configuration == NULL) {
			printf("You have to specify both node address and mining configuration.\n");
			return -1;
		}
		g_xdag_type = XDAG_POOL;
	} else {
		g_xdag_type = XDAG_WALLET;
	}

	if(g_disable_mining && is_wallet()) {
		g_disable_mining = 0;   // this option is only for pools
	}

	return 1;	// 1 - continue execution
}

int pre_init(void)
{
	if(!xdag_time_init()) {
		printf("Cannot initialize time module\n");
		return -1;
	}
	if(!xdag_network_init()) {
		printf("Cannot initialize network\n");
		return -1;
	}
	if(xdag_log_init()) return -1;

	xdag_mess("Initializing addresses...");
	if(xdag_address_init()) return -1;

	memset(&g_xdag_stats, 0, sizeof(g_xdag_stats));
	memset(&g_xdag_extstats, 0, sizeof(g_xdag_extstats));

	//TODO: future xdag.wallet
	xdag_mess("Reading wallet...");
	if(dnet_key_init() < 0) return -1;
	xdag_mess("Initializing cryptography...");
	if(xdag_crypt_init(1)) return -1;
	if(xdag_wallet_init()) return -1;

	return 0;
}

int setup_miner(struct startup_parameters *parameters)
{
	static char pool_address_buf[50] = { 0 };
	if(parameters->pool_address == NULL) {
		if(!xdag_pick_pool(pool_address_buf)) {
			return -1;
		}
		parameters->pool_address = pool_address_buf;
	}

	//TODO: think of combining the logic with pool-initialization functions (after decision what to do with xdag_blocks_start)
	if(parameters->is_rpc) {
		xdag_mess("Initializing RPC service...");
		if(!!xdag_rpc_service_start(parameters->rpc_port)) return -1;
	}

	xdag_mess("Starting blocks engine...");
	if(xdag_blocks_start(parameters->mining_threads_count, !!parameters->miner_address)) return -1;	//TODO: rewrite

	if(!g_disable_mining) {
		xdag_mess("Starting mining engine...");
		if(xdag_initialize_mining(parameters->pool_address, parameters->miner_address)) return -1;
	}

	return 0;
}

int setup_pool(struct startup_parameters *parameters)
{
	if(parameters->pool_configuration.node_address != NULL && parameters->bind_to == NULL) {
		char str[64] = { 0 }, *p = strchr(parameters->pool_configuration.node_address, ':');
		if(p) {
			sprintf(str, "0.0.0.0%s", p);
			parameters->bind_to = strdup(str);
		}
	}

	xdag_mess("Starting synchonization engine...");
	if(xdag_sync_init()) return -1;
	xdag_mess("Starting dnet transport...");
	printf("Transport module: ");
	if(xdag_transport_start(parameters->transport_flags, parameters->transport_threads, parameters->bind_to, parameters->addrports_count, parameters->addr_ports)) return -1;

	xdag_mess("Reading hosts database...");
	if(xdag_netdb_init(parameters->pool_configuration.node_address, parameters->addrports_count, parameters->addr_ports)) return -1;

	if(parameters->is_rpc) {
		xdag_mess("Initializing RPC service...");
		if(!!xdag_rpc_service_start(parameters->rpc_port)) return -1;
	}
	xdag_mess("Starting blocks engine...");
	if(xdag_blocks_start(parameters->mining_threads_count, !!parameters->miner_address)) return -1;

	if(!g_disable_mining) {
		xdag_mess("Starting pool engine...");
		if(xdag_initialize_mining(parameters->pool_configuration.mining_configuration, parameters->miner_address)) return -1;
	}

	return 0;
}

int dnet_key_init(void)
{
	int err = dnet_crypt_init();
	if(err < 0) {
		printf("Password incorrect.\n");
		return err;
	}
	return 0;
}

void set_xdag_name(void)
{
	g_progname = "xdag";
	g_coinname = "XDAG";
}

int xdag_set_password_callback(int(*callback)(const char *prompt, char *buf, unsigned size))
{
	return xdag_user_crypt_action((uint32_t *)(void *)callback, 0, 0, 6);
}

void printUsage(char* appName)
{
	printf("Usage: %s flags [pool_ip:port]\n"
		"If pool_ip:port argument is given, then the node operates as a miner.\n"
		"Flags:\n"
		"  -a address     - specify your address to use in the miner\n"
		"  -c ip:port     - address of another xdag full node to connect\n"
		"  -d             - run as daemon (default is interactive mode)\n"
		"  -h             - print this help\n"
		"  -i             - run as interactive terminal for daemon running in this folder\n"
		"  -l             - output non zero balances of all accounts\n"
		"  -m N           - use N CPU mining threads (default is 0)\n"
		"  -f             - configuration file path (pools only). Default value is pool.config\n"
		"  -p ip:port     - (obsolete) public address of this node\n"
		"  -P ip:port:CFG - (obsolete) run the pool, bind to ip:port, CFG is miners:maxip:maxconn:fee:reward:direct:fund\n"
		"                     miners - maximum allowed number of miners,\n"
		"                     maxip - maximum allowed number of miners connected from single ip,\n"
		"                     maxconn - maximum allowed number of miners with the same address,\n"
		"                     fee - pool fee in percent,\n"
		"                     reward - reward to miner who got a block in percent,\n"
		"                     direct - reward to miners participated in earned block in percent,\n"
		"                     fund - community fund fee in percent\n"
		"  -r             - load local blocks and wait for 'run' command to continue\n"
		"  -s ip:port     - address of this node to bind to\n"
		"  -t             - connect to test net (default is main net)\n"
		"  -v N           - set loglevel to N\n"
		"  -z <path>      - path to temp-file folder\n"
		"  -z RAM         - use RAM instead of temp-files\n"
		"  -rpc-enable    - enable JSON-RPC service\n"
		"  -rpc-port      - set HTTP JSON-RPC port (default is 7667)\n"
		"  -threads N     - create N transport layer threads for pool (default is 6)\n"
		"  -dm            - disable mining on pool (-P option is ignored)\n"
		"  -tag           - tag for pool to distingush pools. Max length is 32 chars\n"
		, appName);
}
