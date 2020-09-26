

include ../inc.mk

TARGET = ${LIB_PATH}/libcgi.a

SRC = $(wildcard *.cpp)

OBJS = ${patsubst %.cpp, %.o, ${SRC}}

CXX_INCLUDE += -I${DEPS_ROOT}/libconfig/lib 
CXX_INCLUDE += -I${DEPS_ROOT}/hiredis
CXX_INCLUDE += -I${HUIBASE_INCLUDE_PATH}
CXX_INCLUDE += -I${HUIBASE_INCLUDE_PATH}/nosql
CXX_INCLUDE += -I${HRPC_INCLUDE_PATH}/client
CXX_INCLUDE += -I${DEPS_ROOT}/cgicc
CXX_INCLUDE += -I${HUIBASE_INCLUDE_PATH}/crypto
CXX_INCLUDE += -I${DEPS_ROOT}/jsoncpp/include


all: def

def: ${TARGET}


include ../route.mk






