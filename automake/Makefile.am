# Makefile.am
# created by cz.worker@gmail.com on 2018-03-29
# modify by reymondtu@gmx.com on 2018-05-23

SRCROOT             = ..
client              = $(SRCROOT)/client
dnet                = $(SRCROOT)/dnet
dfslib              = $(SRCROOT)/dfslib
ldusinc             = $(SRCROOT)/ldus
utils               = $(SRCROOT)/client/utils
jsonrpc             = $(SRCROOT)/client/json-rpc
http                = $(SRCROOT)/client/http
moving_statistics   = $(SRCROOT)/client/utils/moving_statistics
algorithms          = $(SRCROOT)/client/algorithms
secp256k1           = $(SRCROOT)/secp256k1
websocket           = $(SRCROOT)/client/websocket

sources = \
	$(algorithms)/sha256-mb-x86_64$(HOST_SUFFIX).s  \
	$(client)/x86_64cpuid$(HOST_SUFFIX).s           \
	$(client)/address.c                             \
	$(client)/block.c                               \
	$(client)/crypt.c                               \
	$(client)/hash.c                                \
	$(client)/init.c                                \
	$(client)/main.c                                \
	$(client)/memory.c                              \
	$(client)/miner.c                               \
	$(client)/netdb.c                               \
	$(client)/pool.c                                \
	$(client)/mining_common.c                       \
	$(client)/storage.c                             \
	$(client)/sync.c                                \
	$(client)/transport.c                           \
	$(client)/wallet.c                              \
	$(client)/commands.c                            \
	$(client)/terminal.c                            \
	$(client)/network.c                             \
	$(dnet)/dnet_crypt.c                            \
	$(dnet)/dnet_xdag.c                             \
	$(dfslib)/dfslib_crypt.c                        \
	$(dfslib)/dfslib_random.c                       \
	$(dfslib)/dfslib_string.c                       \
	$(dfslib)/dfsrsa.c                              \
	$(algorithms)/sha256.c                          \
	$(algorithms)/crc.c                             \
	$(utils)/log.c                                  \
	$(utils)/utils.c                                \
	$(utils)/linenoise.c                            \
	$(utils)/dirname.c                              \
	$(utils)/string_utils.c                         \
	$(utils)/random.c                               \
	$(moving_statistics)/moving_average.c           \
	$(jsonrpc)/cJSON.c                              \
	$(jsonrpc)/cJSON_Utils.c                        \
	$(jsonrpc)/rpc_procedures.c                     \
	$(jsonrpc)/rpc_procedure.c                      \
	$(jsonrpc)/rpc_service.c                        \
	$(jsonrpc)/rpc_wrapper.c                        \
	$(jsonrpc)/rpc_commands.c                       \
	$(http)/url.c                                   \
	$(http)/http.c                                  \
	$(secp256k1)/src/secp256k1.c                    \
	$(websocket)/websocket.c                        \
	$(websocket)/wslay/wslay_event.c                \
	$(websocket)/wslay/wslay_frame.c                \
	$(websocket)/wslay/wslay_net.c                  \
	$(websocket)/wslay/wslay_queue.c                \
	$(websocket)/wslay/wslay_stack.c                \

headers = \
	$(client)/address.h                             \
	$(client)/block.h                               \
	$(client)/crypt.h                               \
	$(client)/hash.h                                \
	$(client)/init.h                                \
	$(client)/memory.h                              \
	$(client)/netdb.h                               \
	$(client)/pool.h                                \
	$(client)/miner.h                               \
	$(client)/mining_common.h                       \
	$(client)/state.h                               \
	$(client)/storage.h                             \
	$(client)/sync.h                                \
	$(client)/transport.h                           \
	$(client)/wallet.h                              \
	$(client)/commands.h                            \
	$(client)/terminal.h                            \
	$(dnet)/dnet_crypt.h                            \
	$(dnet)/dnet_history.h                          \
	$(dnet)/dnet_main.h                             \
	$(dnet)/dnet_packet.h                           \
	$(dnet)/dnet_system.h                           \
	$(dfslib)/dfslib_crypt.h                        \
	$(dfslib)/dfslib_random.h                       \
	$(dfslib)/dfslib_string.h                       \
	$(dfslib)/dfslib_types.h                        \
	$(dfslib)/dfsrsa.h                              \
	$(ldusinc)/atomic.h                             \
	$(ldusinc)/list.h                               \
	$(ldusinc)/rbtree.h                             \
	$(algorithms)/sha256.h                          \
	$(algorithms)/include/crc.h                     \
	$(utils)/log.h                                  \
	$(utils)/utils.h                                \
	$(utils)/linenoise.h                            \
	$(utils)/string_utils.h                         \
	$(utils)/random.h                               \
	$(moving_statistics)/moving_average.h           \
	$(jsonrpc)/cJSON.h                              \
	$(jsonrpc)/cJSON_Utils.h                        \
	$(jsonrpc)/rpc_procedures.h                     \
	$(jsonrpc)/rpc_procedure.h                      \
	$(jsonrpc)/rpc_service.h                        \
	$(jsonrpc)/rpc_wrapper.h                        \
	$(jsonrpc)/rpc_commands.h                       \
	$(http)/url.h                                   \
	$(http)/http.h                                  \
	$(websocket)/websocket.h                        \
	$(websocket)/wslay/config.h                     \
	$(websocket)/wslay/wslay.h                      \
	$(websocket)/wslay/wslayver.h                   \
	$(websocket)/wslay/wslay_event.h                \
	$(websocket)/wslay/wslay_frame.h                \
	$(websocket)/wslay/wslay_net.h                  \
	$(websocket)/wslay/wslay_queue.h                \
	$(websocket)/wslay/wslay_stack.h                

#program
bin_PROGRAMS = xdag

# macro defines
_DEFS =                                             \
	-DDFSTOOLS                                      \
	-DCHEATCOIN                                     \
	-DNDEBUG                                        \
	-DSHA256_USE_OPENSSL_TXFM                       \
	-DSHA256_OPENSSL_MBLOCK

_INCLUDES =                                         \
	-I$(SRCROOT)                                    \
	-I$(dnet)                                       \
	-I$(dfslib)                                     \
	-I$(ldusinc)                                    \
	-I$(utils)                                      \
	-I$(moving_statistics)                          \
	-I$(jsonrpc)                                    \
	-I$(http)                                       \
	-I$(secp256k1)/src                              \
	-I$(secp256k1)/                                 \
	-I$(websocket)/wslay                            \
	-I$(websocket)/                                 \
	

xdag_SOURCES = $(sources)
#xdag_LDADD = -lpthread -lcrypto -lssl -lm
xdag_CFLAGS = -std=gnu11 -O3 -g $(_DEFS) $(LIBCRYPTO_CFLAGS) $(_INCLUDES) -Wall -Wmissing-prototypes -Wno-unused-result

