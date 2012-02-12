/* Like copyXPMArea, but only copies non-masked pixels */
void combineWithTrans(int sx, int sy, unsigned w, unsigned h, int dx, int dy);

/* Like combineWithTrans, except it combines pixels by this formula:
 * new = (src * o + dest * (256 - o)) / 256
 */
void combineWithOpacity(int sx, int sy, unsigned w, unsigned h, int dx, int dy, int o);
