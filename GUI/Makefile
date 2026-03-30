CC = gcc
RM = rm -rfv
EXEC +=\
main

CFLAGS +=\
-Wextra \
-Wall

RAY_FLAGS += 	\
-lraylib	\
-lGL		\
-lm		\
-lpthread	\
-ldl		\
-lrt		\
-lX11 		\


.PHONY: all clean

all: $(EXEC)

%: %.c
	$(CC) $< -o $@ $(CFLAGS) $(RAY_FLAGS)

clean:
	@$(RM) $(EXEC)
