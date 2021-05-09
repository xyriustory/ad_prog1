CC			:= gcc
CFLAGS		:= -O2 -Wall -MP -MMD -Iinclude -DGL_SILENCE_DEPRECATION
LDFLAGS		:= -Llib -lmatrix
SRC			:= final.c
OBJS		:= $(patsubst %.c,%.o,$(SRC))
DEPS		:= $(patsubst %.c,%.d,$(SRC))
OUTPUT		:= final

ifeq ($(shell uname),Darwin)
FRAMEWORKS	:= -framework OpenGL -framework GLUT
else
FRAMEWORKS	:= -lopengl32 -lglu32 -lglut
endif

.PHONY: all
all: $(OUTPUT)

-include $(DEPS)

$(OUTPUT): $(OBJS)
	$(CC) $^ $(LDFLAGS) $(FRAMEWORKS) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	$(RM) -f $(DEPS) $(OBJS) $(OUTPUT) $(addsuffix .exe,$(OUTPUT))
