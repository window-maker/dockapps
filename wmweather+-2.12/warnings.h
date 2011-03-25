extern unsigned long current_warnings;

void init_warnings(void);
void update_warnings(int force);
void output_warnings(int all);
void warnings_cleanup(void);
