
CFLAGS = -g

all:	nat-client nat-server
	@echo "All done."

nat-client:	nat-client.o nat-reg.o 
	gcc -o $@ $^ -lstdc++

nat-server:	nat-server.o nat-reg.o
	gcc -o $@ $^ -lstdc++

%.o:	%.cpp
	gcc -MMD -c $< -o $@ $(CFLAGS)

clean:
	rm -f *.o *~ *.d nat-client nat-server

-include $(patsubst %.o,%.d,$(wildcard *.o)) Make-extra
