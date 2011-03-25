/* Returns 0 if the files are identical
 *         1 if they differ
 *        -1 if fopen or fstat fails, or if files are not both regular files
 */
int diff(char *file1, char *file2);
