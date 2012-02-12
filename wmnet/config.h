#if defined (__FreeBSD__) || defined (__OpenBSD__)

/* Our only FreeBSD driver, this goes straight into kernel memory 
 * and reads the raw structures from right underneath the kernel using the
 * kvm library.  This made the code a require a little more thought, but 
 * the end result is a statistics driver thats faster than the linux ones 
 * (with the possible exception of the LINUX_PPP driver).  However, none
 * of them really vary by any appreciable amount. You can monitor whole
 * interfaces only with this driver. 
 */
# define USE_KVM

#endif



#ifdef linux

/* this driver uses a socket ioctl() to get stats from a ppp type interface
 * Define this if you will be using wmnet mostly to watch your ppp stats.
 * The advantage to this driver is that you don't need to mess around with
 * IP accounting rules. 
 */
#define USE_LINUX_PPP






/* This driver uses the 2.0 kernel's IP accounting rules to gather data 
 * You set two rules up using the ipfwadm command and wmnet will watch
 * them.  You DO need a 2.0 kernel and IP accounting enabled in your kernel
 */
#define USE_IPFWADM





/* If you have a 2.1 kernel, you've probably noticed that IP accounting
 * and the ipfwadm command won't work anymore.  These have been superceded
 * by the ipchains mechanism.  Define this if you want IP chains support,
 * you have a 2.1 kernel, and you have set up two chains (read README)
 */
#define USE_IPCHAINS

/* Define this if you have a 2.1 kernel and wish to use the stats from
 * /proc/net/dev.  In the 2.1 kernels, /proc/net/dev now has a bytes field
 * (2.0 only had a packets field) which enables us to use this driver as
 * a generic interface watcher.  With this, you don't have to fool around with
 * ipchains if you don't want to or haven't it enabled
 */
#define USE_2_1_DEV
#endif /* linux */

