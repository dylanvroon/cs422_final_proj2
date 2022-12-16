# -*-Makefile-*-

OBJDIRS		+= $(USER_OBJDIR)/memtest

USER_MEMTEST_SRC	+= $(USER_DIR)/memtest/memtest.c
USER_MEMTEST_OBJ	:= $(patsubst %.c, $(OBJDIR)/%.o, $(USER_MEMTEST_SRC))
USER_MEMTEST_OBJ	:= $(patsubst %.S, $(OBJDIR)/%.o, $(USER_MEMTEST_OBJ))
KERN_BINFILES	+= $(USER_OBJDIR)/memtest/memtest

memtest: $(USER_OBJDIR)/memtest/memtest

$(USER_OBJDIR)/memtest/memtest: $(USER_LIB_OBJ) $(USER_MEMTEST_OBJ)
	@echo + ld[USER/memtest] $@
	$(V)$(LD) -o $@ $(USER_LDFLAGS) $(USER_LIB_OBJ) $(USER_MEMTEST_OBJ) $(GCC_LIBS)
	mv $@ $@.bak
	$(V)$(OBJCOPY) --remove-section .note.gnu.property $@.bak $@
	$(V)$(OBJDUMP) -S $@ > $@.asm
	$(V)$(NM) -n $@ > $@.sym

$(USER_OBJDIR)/memtest/%.o: $(USER_DIR)/memtest/%.c
	@echo + cc[USER/memtest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<

$(USER_OBJDIR)/memtest/%.o: $(USER_DIR)/memtest/%.S
	@echo + as[USER/memtest] $<
	@mkdir -p $(@D)
	$(V)$(CC) $(USER_CFLAGS) -c -o $@ $<
