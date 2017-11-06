CPP = g++
CARGS = 

ifdef DEBUG_FLAG
PATH_LUA=./lua_dbglib/
else
PATH_LUA=/usr/local/lua51/
endif

INC_LUA=-I$(PATH_LUA)/include/
LIB_LUA=$(PATH_LUA)/lib/liblua.a -ldl

OBJS_DIR = TemporaryObjs/gamesvr

ALL_MODULE = $(shell find ./ -type d)
INC_ALL += $(addprefix -I, $(ALL_MODULE))

INC = $(INC_ALL) $(INC_LUA)
LIB = $(LIB_LUA)

LIB_THREAD = -lpthread

ifdef DEBUG_FLAG
BINARY = gamesvr.dbg
C_ARGS = -g -Wall -Wformat-security -Werror -rdynamic -D_FILE_OFFSET_BITS=64 $(INC) -fPIC -D_DEBUG
else
BINARY = gamesvr
C_ARGS = -g -Wall -Wformat-security -Werror -rdynamic -D_FILE_OFFSET_BITS=64 $(INC) -fPIC -O3
endif


ALL_CPP_LIST = $(shell find ./ -maxdepth 2 -name "*.cpp")
ALL_C_LIST = $(shell find ./ -maxdepth 2 -name "*.c")

ALLOBJS = $(patsubst %.cpp, $(OBJS_DIR)/%.o, $(ALL_CPP_LIST))
ALLOBJS += $(patsubst %.c, $(OBJS_DIR)/%.o, $(ALL_C_LIST))

$(OBJS_DIR)/%.o: %.cpp
	@mkdir -p $(OBJS_DIR)/$(shell dirname $<)
	$(CPP) $(C_ARGS) $(INC) -o $@ -c $<
$(OBJS_DIR)/%.o: %.c
	@mkdir -p $(OBJS_DIR)/$(shell dirname $<)
	$(CPP) $(C_ARGS) $(INC) -o $@ -c $<


#------------------------------------
.PHONY: all clean

all: $(BINARY)

$(BINARY): $(ALLOBJS)
	@rm -f $@
	$(CPP) -Wl -Bstatic $(C_ARGS) -o $@ $(ALLOBJS) $(LIB)

clean:
	rm -f $(BINARY) $(ALLOBJS)
