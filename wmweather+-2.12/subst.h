#ifndef SUBST_H
#define SUBST_H

struct subst_val {
    char id;         /* if id=='X', %X will be substituted */
    enum {
        INT     ='i', /* val => signed int */
        UINT    ='u', /* val => unsigned int */
        OCTAL   ='o', /* val => unsigned int */
        HEX     ='x', /* val => unsigned int */
        FLOAT_E ='e', /* val => double */
        FLOAT_F ='f', /* val => double */
        FLOAT_G ='g', /* val => double */
        FLOAT_A ='a', /* val => double */
        CHAR    ='c', /* val => char */
        STRING  ='s'  /* val => char * */
    } type;
    void *val;
};

char *subst(const char *s, struct subst_val *substitutes);

#endif
