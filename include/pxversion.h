/*
 * pxversion.h : compile-time version informations for the pslib.
 *
 * See Copyright for the status of this software.
 *
 * Uwe@steinmann.cx
 */

#ifndef __PXLIB_VERSION_H__
#define __PXLIB_VERSION_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * use those to be sure nothing nasty will happen if
 * your library and includes mismatch
 */
#define PXLIB_DOTTED_VERSION "@PXLIB_DOTTED_VERSION@"
#define PXLIB_MAJOR_VERSION 0
#define PXLIB_MINOR_VERSION 6
#define PXLIB_MICRO_VERSION 9

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif


