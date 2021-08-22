

#define command1 "svn log -q -r HEAD http://svn/svn | grep -v -- ------------------------------------------------------------------------ | gawk -F'|' ' {print $1}' "

#define command2 "svn log -q -r HEAD http://svn/svn | grep -v -- ------------------------------------------------------------------------ | gawk -F'|' ' {print $2}' " 

#define command3 "svn log --incremental -r HEAD http://svn/svn |  tail -n 1 "

#define DATA_REFRESH_RATIO 1000
#define DISPLAY_REFRESH_RATIO 10



#define CMDCOUNT 3
#define BUFFERLENGTH 1000//hope it'll suffice
#define min(x, y)   ((x)<(y))?x:y

void trim( char **buffer);
short unsigned int isWhitespace( char c );
void BlitString(const char *name, int x, int y);
void DisplayScroll (const char *label, const int position, const int level);
void removeEolns(char** buffer);
void freeBuffer(char** buffer, unsigned int length);
void executeCommand( const char* cmd, char**buffer, const unsigned int length );
