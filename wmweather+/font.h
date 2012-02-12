int DrawString(int x, int y, char *str, int font);
int GetStringWidth(char *str);
int DrawNumber(int x, int y, int n, int font);
int DrawChar(int x, int y, char c, int font);

/* This is called automatically if necessary */
void init_font(int i);
