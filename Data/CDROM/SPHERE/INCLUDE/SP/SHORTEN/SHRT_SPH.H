/***********************************************************************/
/*    File: shrt_sph.h                                                 */
/*    Desc: Sphere to Shorten interface header file                    */
/*    History:                                                         */
/*        Mar 29, 1993:  Creation                                      */
/***********************************************************************/

# define FORMAT_VERSION		1
# define BUGFIX_RELEASE         "09"

extern void shorten_set_flags();
extern void shorten_reset_flags();
extern void shorten_set_ftype();
extern void shorten_set_channel_count();
extern void shorten_init();
extern int shorten_compress();
extern int shorten_uncompress();


