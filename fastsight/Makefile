CC       = gcc
CXX      = g++
CFLAGS   = -Wall -O3 -funroll-loops -DUSE_XSHM
CXXFLAGS = -Wall -O3 -funroll-loops `fltk-config --cxxflags` -DUSE_XSHM
LDFLAGS  = -lm `fltk-config --ldstaticflags`
OBJS     = main.o interface.o j2kcodec.o connect.o \
           Fl_VideoArea.o filters/filters.o \
           filters/grayscale.o filters/average.o \
           filters/filtercfg.o filters/range.o \
           linux/get_ip.o linux/capcfg.o \
           linux/capture.o linux/cfgmanager.o \
           libj2k/bio.o libj2k/cio.o libj2k/dwt.o \
           libj2k/fix.o libj2k/int.o libj2k/j2k.o \
           libj2k/log.o libj2k/mct.o libj2k/mqc.o \
           libj2k/pi.o libj2k/t1.o libj2k/t2.o \
           libj2k/tcd.o libj2k/tgt.o
FSVOBJS  = fsvplay.o j2kcodec.o Fl_VideoArea.o \
           linux/cfgmanager.o \
           libj2k/bio.o libj2k/cio.o libj2k/dwt.o \
           libj2k/fix.o libj2k/int.o libj2k/j2k.o \
           libj2k/log.o libj2k/mct.o libj2k/mqc.o \
           libj2k/pi.o libj2k/t1.o libj2k/t2.o \
           libj2k/tcd.o libj2k/tgt.o
INCLUDE  = -I. -Ifilters -Ilibj2k -Ilinux

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $< -o $*.o

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $*.o

all: fastsight fsvplay

fastsight: $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o fastsight

fsvplay: $(FSVOBJS)
	$(CXX) $(FSVOBJS) $(LDFLAGS) -o fsvplay
  
clean: 
	rm -f $(OBJS) $(FSVOBJS) fastsight fsvplay
