#==============================================================================
# Makefile
#==============================================================================

#Name of the package
PKG = 

# ---- compiler options ----
CC = g++
LD = g++
CFLAGS = -W -Wall -fexceptions -fno-builtin -O2 -fpic -D_REENTRANT -c
LDFLAGS = -fexceptions -O2 -o
INCLUDEHEADERS = -I ../ -I /usr/local/include/putils

os = linux
ifeq ($(os),qnx)
	INCLUDELIBS = -L ../ -lputils -lm -lsocket
else 
	INCLUDELIBS = -L ../ -lputils -lm -lc -lrt
endif

OBJ = 
TARGET = ErrnoException.t RecursiveMutex.t StatusReport.t ShMem.t \
         MessageQueue.t PtBarrier.t RWLock.t TCPClientServer.t \
         UDPClientServer.t Thread.t
CLEAN = rm -rf *.o lib* *.dat $(TARGET)


# ========== Targets ==========
targets: $(TARGET)

# ----- ErrnoException -----
ErrnoException.t: ErrnoException.t.cpp
	$(CC) $(CFLAGS) ErrnoException.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) ErrnoException.t ErrnoException.t.o $(INCLUDELIBS)

# ----- RecursiveMutex -----
RecursiveMutex.t: RecursiveMutex.t.cpp
	$(CC) $(CFLAGS) RecursiveMutex.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) RecursiveMutex.t RecursiveMutex.t.o $(INCLUDELIBS)

# ----- PtBarrier -----
PtBarrier.t: PtBarrier.t.cpp
	$(CC) $(CFLAGS) PtBarrier.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) PtBarrier.t PtBarrier.t.o $(INCLUDELIBS)

# ----- RWLock -----
RWLock.t: RWLock.t.cpp
	$(CC) $(CFLAGS) RWLock.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) RWLock.t RWLock.t.o $(INCLUDELIBS)

# ----- StatusReport -----
StatusReport.t: StatusReport.t.cpp
	$(CC) $(CFLAGS) StatusReport.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) StatusReport.t StatusReport.t.o $(INCLUDELIBS)

# ----- ShMem -----
ShMem.t: ShMem.t.cpp
	$(CC) $(CFLAGS) ShMem.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) ShMem.t ShMem.t.o $(INCLUDELIBS)

# ----- MessageQueue -----
MessageQueue.t: MessageQueue.t.cpp
	$(CC) $(CFLAGS) MessageQueue.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) MessageQueue.t MessageQueue.t.o $(INCLUDELIBS)

# ----- TCPClientServer -----
TCPClientServer.t: TCPClientServer.t.cpp
	$(CC) $(CFLAGS) TCPClientServer.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) TCPClientServer.t TCPClientServer.t.o $(INCLUDELIBS)

# ----- UDPClientServer -----
UDPClientServer.t: UDPClientServer.t.cpp
	$(CC) $(CFLAGS) UDPClientServer.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) UDPClientServer.t UDPClientServer.t.o $(INCLUDELIBS)

# ----- Thread -----
Thread.t: Thread.t.cpp
	$(CC) $(CFLAGS) Thread.t.cpp $(INCLUDEHEADERS)
	$(LD) $(LDFLAGS) Thread.t Thread.t.o $(INCLUDELIBS)

clean:
	$(CLEAN)

