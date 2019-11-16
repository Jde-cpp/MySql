CCACHE_EXISTS := $(shell ccache -V)
ifdef CCACHE_EXISTS
    CC := ccache $(CC)
    CXX := ccache g++-8
endif
DEBUG ?= 1
CXXFLAGS_BASE = -c -g -pthread -fPIC -std=c++17 -Wall -Wno-unknown-pragmas -DJDE_MYSQL_EXPORTS
BIN_DIR_BASE:=../../bin/
OUT_DIR_BASE := .obj
ifeq ($(DEBUG), 1)
	OUT_DIR=$(OUT_DIR_BASE)/debug
	CXXFLAGS= $(CXXFLAGS_BASE) -O0 -I$(OUT_DIR)
	BIN_DIR=$(BIN_DIR_BASE)debug
else
	OUT_DIR=$(OUT_DIR_BASE)/release
	CXXFLAGS= $(CXXFLAGS_BASE) -march=native -DNDEBUG -O3 -I$(OUT_DIR)
	BIN_DIR=$(BIN_DIR_BASE)release
endif

OUTPUT_FILE = libMySql.so
OUTPUT = $(BIN_DIR)/$(OUTPUT_FILE)
LIBS = -lstdc++fs -lMy -lmysqlcppconn8

all: $(OUTPUT)
	$(NOECHO) $(NOOP)

OBJECTS = $(OUT_DIR)/MySqlDataSource.o $(OUT_DIR)/MySqlRow.o $(OUT_DIR)/MySqlSchemaProc.o  $(OUT_DIR)/MySqlStatements.o
LDFLAGS = -pthread -shared -Wl,-z,origin -Wl,-rpath='$$ORIGIN' 

$(OUTPUT): $(OUT_DIR)/stdafx.h.gch $(OBJECTS)
	$(CXX) -L$(BIN_DIR) $(LDFLAGS) $(OBJECTS) -o$(OUTPUT) $(LIBS)

$(OUT_DIR)/stdafx.h.gch: pc.h
	$(CXX) $(CXXFLAGS) pc.h -o$(OUT_DIR)/stdafx.h.gch -I/home/duffyj/code/libraries/spdlog/include -I/home/duffyj/code/libraries/mysql/include/mysqlx -I$$BOOST_ROOT

$(OUT_DIR)/%.o: ./%.cpp ./%.h pc.h
	$(CXX) $(CXXFLAGS) $(INCLUDE) -Wno-int-in-bool-context -Wno-ignored-attributes ./$< -o$@ -c #-I/home/duffyj/code/libraries/eigen  -I/home/duffyj/code/libraries/json/include -I/home/duffyj/code/libraries/LightGBM/include

clean:
	rm -rf -d $(OUT_DIR)/*.*
	rm -rf -d $(OUTPUT)
