program_NAME := test
program_C_SRCS := $(wildcard *.c)
program_C_OBJS := ${program_C_SRCS:.c=.c}
program_OBJS := $(program_C_OBJS)
program_INCLUDE_DIRS :=
program_LIBRARY_DIRS :=
program_LIBRARIES := -lpthread


CPPFLAGS += $(foreach includedir, $(program_INCLUDE_DIRS),-I$(includedir))
LDFLAGS +=  $(foreach librarydir, $(program_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS +=  $(foreach library, $(program_LIBRARIES), -l(library))

.PHONY: all clean distclean

all: $(program_NAME)

$(program_NAME): $(program_OBJS)
	$(CC) $(program_OBJS) $(program_LIBRARIES) -o $(program_NAME) -std=c99 -D _BSD_SOURCE

clean:
	@- $(RM) $(program_NAME)

distclean: clean
