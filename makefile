uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')

# compilation options
CXX=g++
CC=gcc
CXXFLAGS=-fPIC -mavx -maes -mpclmul -DRDTSC -DTEST=AES128 -O3

INSTALL_INCLUDE_DIR=$(includedir)/ScGarbledCircuitNoFixedKey
INSTALL_LIB_DIR=$(libdir)

ifeq ($(uname_S),Linux)
	INCLUDE_ARCHIVES_START = -Wl,-whole-archive
	INCLUDE_ARCHIVES_END = -Wl,-no-whole-archive -Wl,--no-undefined
	SHARED_LIB_OPT:=-shared
	SHARED_LIB_EXT:=.so
endif

ifeq ($(uname_S),Darwin)
	INCLUDE_ARCHIVES_START=-Wl,-all_load
	INCLUDE_ARCHIVES_END=
	SHARED_LIB_OPT:=-dynamiclib
	SHARED_LIB_EXT:=.dylib
endif

# openssl dependency
SOURCESCPP = GarbledBooleanCircuit.cpp \
	FastGarblingFourToTwoNoAssumptions.cpp\
	FastGarblingFreeXorHalfGatesFixedKeyAssumptions.cpp\
	TedKrovetzAesNiWrapperC.cpp
	
OBJ_FILES = $(SOURCESCPP:.cpp=.o)

SOURCESC =intrin_sequential_ks2_enc2.c ecb.c intrin_sequential_ks4_enc8.c intrin_sequential_ks1_enc1.c intrin_sequential_ks4_enc4.c
OBJ_FILESC = $(SOURCESC:.c=.o)

## targets ##

# main target - linking individual *.o files
libScGarbledCircuitNoFixedKey$(SHARED_LIB_EXT): $(OBJ_FILES) $(OBJ_FILESC)
	$(CXX) $(SHARED_LIB_OPT) -o $@ $(OBJ_FILES) $(OBJ_FILESC)

install:
	@echo "Installing ScGarbledCircuitNoFixedKey..."
	install -d $(INSTALL_INCLUDE_DIR)
	install -d $(INSTALL_LIB_DIR)
	install -m 0644 *.h $(INSTALL_INCLUDE_DIR)
	install -m 0644 libScGarbledCircuitNoFixedKey$(SHARED_LIB_EXT) $(INSTALL_LIB_DIR)
	@echo "Done."

# each source file is compiled seperately before linking
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(SOURCESCPP)
%.o: %.c
	$(CC) $(CXXFLAGS) -c $(SOURCESC)

clean:
	rm -f *~
	rm -f *.o
	rm -f *.so
	rm -f *.dylib