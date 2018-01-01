TEST_GENERIC_SRCS := test.c
TEST_GENERIC_OBJS := ${TEST_GENERIC_SRCS:.c=.c}
TEST_ITERATOR_SRCS := test_iterator.c
TEST_ITERATOR_OBJS := ${TEST_ITERATOR_SRCS:.c=.c}

COMMON_SOURCES := $(filter-out $(TEST_GENERIC_SRCS) $(TEST_ITERATOR_SRCS),$(wildcard *.c))
COMMON_OBJS := ${COMMON_SOURCES:.c=.c}

priqueue_INCLUDE_DIRS := .
priqueue_LIBRARY_DIRS :=
priqueue_LIBRARIES := pthread

CPPFLAGS += $(foreach includedir, $(priqueue_INCLUDE_DIRS),-I$(includedir))
LDFLAGS +=  $(foreach librarydir, $(priqueue_LIBRARY_DIRS),-L$(librarydir))
LDFLAGS +=  $(foreach library, $(priqueue_LIBRARIES),-l$(library))


.PHONY: all clean distclean

all: test_generic test_iterator

test_generic: $(TEST_GENERIC_OBJS) $(COMMON_OBJS)
	$(CC) -Wall -D _BSD_SOURCE $(TEST_GENERIC_OBJS) $(COMMON_OBJS) $(CPPFLAGS) $(LDFLAGS) -o test_generic

test_iterator: $(TEST_ITERATOR_OBJS) $(COMMON_OBJS)
	$(CC) -Wall -D _BSD_SOURCE $(TEST_ITERATOR_OBJS) $(COMMON_OBJS) $(CPPFLAGS) $(LDFLAGS) -o test_iterator

clean:
	@- $(RM) test_generic test_iterator

distclean: clean
