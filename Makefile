CROSS_COMPILER = /home/cfan5/DNTC_SDK/bin/arm-linux-gnueabihf
CC = $(CROSS_COMPILER)-gcc
AR = $(CROSS_COMPILER)-ar
ARFLAG = -rcs
CFLAGS = -g -O0 -march=armv7-a -marm -mfpu=neon -mfloat-abi=hard
CLIBS = -L./lib/ -ldrm -ldrm_omap -lGAL -lDrmWrapper -lGc320Wrapper  -lLog -lFbMng -lImageWrapper -pthread
INCLUDE_DIRS =  -I./include -I./log_mng -I./drm_mng -I./gc320_mng -I./fb_mng -I./image_mng
SUBDIRS = ./log_mng ./image_mng ./drm_mng ./gc320_mng ./fb_mng

export CC CFLAGS AR ARFLAG

TARGET = test_main
OBJECTS = test_main.o

$(TARGET) : $(OBJECTS) $(SUBDIRS)
	$(CC) $< -o $@ $(CLIBS)
	
$(OBJECTS) : %.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE_DIRS)

$(SUBDIRS):ECHO
	+$(MAKE) -C $@

ECHO:
	@echo $(SUBDIRS)
	@echo begin compile

.PHONY : clean
clean:
	for dir in $(SUBDIRS);\
	do $(MAKE) -C $$dir clean||exit 1;\
	done
	rm -rf $(TARGET) $(OBJECTS) ./lib/*.a