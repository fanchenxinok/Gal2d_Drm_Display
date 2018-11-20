LIB_DIR = ../lib/
TOP_DIR = ..

SOURCES = $(wildcard ./*.c)
INCLUDE_DIRS = -I$(TOP_DIR)/include -I./ -I$(TOP_DIR)/log_mng
TARGET = libImageWrapper.a
OBJECTS = $(patsubst %.c,%.o,$(SOURCES))

$(TARGET) : $(OBJECTS)
	$(AR) $(ARFLAG) $@ $^
	cp $@ $(LIB_DIR)
	
$(OBJECTS) : %.o : %.c 
	$(CC) -c $(CFLAGS) $< -o $@ $(INCLUDE_DIRS)

.PHONY : clean
clean:
	rm -rf $(TARGET) $(OBJECTS) 