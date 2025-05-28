obj-m += main.o 
obj-m += SocketHandler.o 

PWD := $(CURDIR) 

all: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules 
clean: 
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean