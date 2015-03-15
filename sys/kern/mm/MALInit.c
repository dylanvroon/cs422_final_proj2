static unsigned int NPS_LOC;

struct A {
	unsigned int isnorm;
	unsigned int allocated;
  unsigned int c;
};

static struct A AT_LOC[1 << 20];

unsigned int
get_nps(void)
{
	return NPS_LOC;
}

void
set_nps(unsigned int nps)
{
	NPS_LOC = nps;
}

unsigned int
is_norm(unsigned int t_is_norm_index)
{
	unsigned int tisnorm;

	tisnorm = AT_LOC[t_is_norm_index].isnorm;

	if (tisnorm == 0) {
		tisnorm = 0;
	} else {
		if (tisnorm == 1)
			tisnorm = 0;
		else
			tisnorm = 1;
	}

	return tisnorm;
}

void
set_norm(unsigned int set_norm_index, unsigned int norm_val)
{
	AT_LOC[set_norm_index].isnorm = norm_val;
	AT_LOC[set_norm_index].allocated = 0;
	AT_LOC[set_norm_index].c = 0;
}

unsigned int
at_get(unsigned int at_get_index)
{
	unsigned int allocated;

	allocated = AT_LOC[at_get_index].allocated;
	if (allocated == 0)
		allocated = 0;
	else
		allocated = 1;

	return allocated;
}

void
at_set(unsigned int at_set_index, unsigned int allocated_val)
{
	AT_LOC[at_set_index].allocated = allocated_val;
}

unsigned int
at_get_c (unsigned int at_get_c_index)
{
    unsigned int c;

    c = AT_LOC[at_get_c_index].c;

    return c;
}

void
at_set_c (unsigned int at_set_c_index, unsigned int c_val)
{
    AT_LOC[at_set_c_index].c = c_val;
}
