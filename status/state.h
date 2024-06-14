#define STATE_FILE "STATE_FILE"
#define MODULES_N 2
#define MODULE_NAME_LEN 16

void initalize_state_file(void);
int read_state(const char*);
int write_state(const char*,int);
