#ifndef STRINGLIST_H
#define STRINGLIST_H

typedef struct string_list_t LIST;

LIST * create_list ();
void   delete_list (LIST *);
int    get_item    (LIST *, unsigned int index);
int    add_item    (LIST *, unsigned int);
int    add_list    (LIST *, LIST *);

#endif
