RSYM_BASE = $(TOOLS_BASE)$(SEP)rsym
RSYM_BASE_ = $(RSYM_BASE)$(SEP)
LOG2LINES_BASE = $(TOOLS_BASE_)log2lines
LOG2LINES_BASE_ = $(LOG2LINES_BASE)$(SEP)

LOG2LINES_INT = $(INTERMEDIATE_)$(LOG2LINES_BASE)
LOG2LINES_INT_ = $(LOG2LINES_INT)$(SEP)
LOG2LINES_OUT = $(OUTPUT_)$(LOG2LINES_BASE)
LOG2LINES_OUT_ = $(LOG2LINES_OUT)$(SEP)

$(LOG2LINES_INT): | $(TOOLS_INT)
	$(ECHO_MKDIR)
	${mkdir} $@

ifneq ($(INTERMEDIATE),$(OUTPUT))
$(LOG2LINES_OUT): | $(TOOLS_OUT)
	$(ECHO_MKDIR)
	${mkdir} $@
endif

LOG2LINES_TARGET = \
	$(LOG2LINES_OUT_)log2lines$(EXEPOSTFIX)

LOG2LINES_SOURCES = \
	$(LOG2LINES_BASE_)list.c \
	$(LOG2LINES_BASE_)util.c \
	$(LOG2LINES_BASE_)options.c \
	$(LOG2LINES_BASE_)help.c \
	$(LOG2LINES_BASE_)cache.c \
	$(LOG2LINES_BASE_)image.c \
	$(LOG2LINES_BASE_)stat.c \
	$(LOG2LINES_BASE_)revision.c \
	$(LOG2LINES_BASE_)cmd.c \
	$(LOG2LINES_BASE_)log2lines.c \
	$(RSYM_BASE_)rsym_common.c

LOG2LINES_OBJECTS = \
	$(addprefix $(INTERMEDIATE_), $(LOG2LINES_SOURCES:.c=.o))

LOG2LINES_HOST_CFLAGS = $(TOOLS_CFLAGS) -I $(RSYM_BASE)

LOG2LINES_HOST_LFLAGS = $(TOOLS_LFLAGS)

.PHONY: log2lines
log2lines: $(LOG2LINES_TARGET)

$(LOG2LINES_TARGET): $(LOG2LINES_OBJECTS) | $(LOG2LINES_OUT)
	$(ECHO_HOSTLD)
	${host_gcc} $(LOG2LINES_OBJECTS) $(LOG2LINES_HOST_LFLAGS) -o $@

$(LOG2LINES_INT_)log2lines.o: $(LOG2LINES_BASE_)log2lines.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)util.o: $(LOG2LINES_BASE_)util.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)list.o: $(LOG2LINES_BASE_)list.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)options.o: $(LOG2LINES_BASE_)options.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)help.o: $(LOG2LINES_BASE_)help.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)cache.o: $(LOG2LINES_BASE_)cache.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)image.o: $(LOG2LINES_BASE_)image.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)stat.o: $(LOG2LINES_BASE_)stat.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)revision.o: $(LOG2LINES_BASE_)revision.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

$(LOG2LINES_INT_)cmd.o: $(LOG2LINES_BASE_)cmd.c | $(LOG2LINES_INT)
	$(ECHO_HOSTCC)
	${host_gcc} $(LOG2LINES_HOST_CFLAGS) -c $< -o $@

.PHONY: log2lines_clean
log2lines_clean:
	-@$(rm) $(LOG2LINES_TARGET) $(LOG2LINES_OBJECTS) 2>$(NUL)
clean: log2lines_clean
