/*
** THIS SOFTWARE IS SUBJECT TO COPYRIGHT PROTECTION AND IS OFFERED ONLY
** PURSUANT TO THE 3DFX GLIDE GENERAL PUBLIC LICENSE. THERE IS NO RIGHT
** TO USE THE GLIDE TRADEMARK WITHOUT PRIOR WRITTEN PERMISSION OF 3DFX
** INTERACTIVE, INC. A COPY OF THIS LICENSE MAY BE OBTAINED FROM THE 
** DISTRIBUTOR OR BY CONTACTING 3DFX INTERACTIVE INC(info@3dfx.com). 
** THIS PROGRAM IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER 
** EXPRESSED OR IMPLIED. SEE THE 3DFX GLIDE GENERAL PUBLIC LICENSE FOR A
** FULL TEXT OF THE NON-WARRANTY PROVISIONS.  
** 
** USE, DUPLICATION OR DISCLOSURE BY THE GOVERNMENT IS SUBJECT TO
** RESTRICTIONS AS SET FORTH IN SUBDIVISION (C)(1)(II) OF THE RIGHTS IN
** TECHNICAL DATA AND COMPUTER SOFTWARE CLAUSE AT DFARS 252.227-7013,
** AND/OR IN SIMILAR OR SUCCESSOR CLAUSES IN THE FAR, DOD OR NASA FAR
** SUPPLEMENT. UNPUBLISHED RIGHTS RESERVED UNDER THE COPYRIGHT LAWS OF
** THE UNITED STATES.  
** 
** COPYRIGHT 3DFX INTERACTIVE, INC. 1999, ALL RIGHTS RESERVED
**
** Revision 1.1.2.5  2006/01/16 21:22:45  jwrdegoede
** Fix gcc 4.1 new type-punned ptr breaks antialias. warnings
**
** Revision 1.1.2.4  2005/06/09 18:32:35  jwrdegoede
** Fixed all warnings with gcc4 -Wall -W -Wno-unused-parameter, except for a couple I believe to be a gcc bug. This has been reported to gcc.
**
** Revision 1.1.2.3  2005/05/25 08:51:52  jwrdegoede
** Add #ifdef GL_X86 around x86 specific code
**
** Revision 1.1.2.2  2004/10/04 09:36:00  dborca
** second cut at Glide3x for Voodoo1/Rush (massive update):
** delayed validation, vertex snapping, clip coordinates, strip/fan_continue, bugfixes.
** and probably a bunch of other crap that I forgot
**
** Revision 1.1.2.1  2004/03/02 07:55:30  dborca
** Bastardised Glide3x for SST1
**
** Revision 1.1.1.1  1999/12/07 21:48:53  joseph
** Initial checkin into SourceForge.
**
 * 
 * 36    9/19/97 12:38p Peter
 * asm rush trisetup vs alt fifo
 * 
 * 35    9/10/97 10:13p Peter
 * fifo logic from GaryT, non-normalized fp first cut
 * 
 * 34    8/19/97 8:55p Peter
 * lots of stuff, hopefully no muckage
 * 
 * 33    8/01/97 11:49a Dow
 * Added conventional FIFO accounting
 * 
 * 32    7/07/97 3:43p Jdt
 * Fixes to trisetup_nogradients
 * 
 * 31    7/07/97 2:14p Jdt
 * Mods to drawtriangle for parity with asm
 * 
 * 30    7/07/97 8:33a Jdt
 * New tracing macros.
 * 
 * 29    7/04/97 12:08p Dow
 * Fixed chip field muckage
 * 
 * 28    7/01/97 7:44a Jdt
 * Separated SST-1 and SST-96 trisetup implementations.
 * 
 * 27    6/30/97 3:30p Jdt
 * Fixed Watcom Wart.
 * 
 * 26    6/30/97 3:23p Jdt
 * rollback
 * 
 * 24    6/29/97 11:49p Jdt
 * First pass at triangle setup optimization for SST96
 * 
 * 23    6/20/97 5:53p Dow
 * Change for subtle chip field bug
 * 
 * 22    6/18/97 5:54p Dow
 * P6 adjustments
 * 
 * 21    5/27/97 11:37p Pgj
 * Fix for Bug report 545
 * 
 * 20    4/13/97 2:06p Pgj
 * eliminate all anonymous unions (use hwDep)
 * 
 * 19    3/21/97 12:42p Dow
 * Made STWHints not send the Bend Over Baby Packet to FBI Jr.
 * 
 * 18    3/04/97 9:45p Dow
 * More neutering of the multiheaded...
 * 
 * 17    3/04/97 9:08p Dow
 * Neutered multiplatform multiheaded monster
 * 
 * 16    2/14/97 12:55p Dow
 * moved vg96 fifo wrap into init code
 * 
 * 15    12/23/96 1:37p Dow
 * chagnes for multiplatform glide
**
*/

#include <3dfx.h>
#include <glidesys.h>

#define FX_DLL_DEFINITION
#include <fxdll.h>
#include <glide.h>
#include "fxglide.h"

/* access a floating point array with a byte index */
#define FARRAY(p,i) (*(float *)((i)+(unsigned long)(p)))
/* access a byte array with a byte index and convert to float */
#define FbARRAY(p,i) (float)(((unsigned char *)p)[i])

#ifdef GDBG_INFO_ON
  /* Some debugging information */
static char *indexNames[] = {  
  "GR_VERTEX_X_OFFSET",         /* 0 */
  "GR_VERTEX_Y_OFFSET",         /* 1 */
  "GR_VERTEX_Z_OFFSET",         /* 2 */
  "GR_VERTEX_R_OFFSET",         /* 3 */
  "GR_VERTEX_G_OFFSET",         /* 4 */
  "GR_VERTEX_B_OFFSET",         /* 5 */
  "GR_VERTEX_OOZ_OFFSET",       /* 6 */
  "GR_VERTEX_A_OFFSET",         /* 7 */
  "GR_VERTEX_OOW_OFFSET",       /* 8 */
  "GR_VERTEX_SOW_TMU0_OFFSET",  /* 9 */
  "GR_VERTEX_TOW_TMU0_OFFSET",  /* 10 */
  "GR_VERTEX_OOW_TMU0_OFFSET",  /* 11 */
  "GR_VERTEX_SOW_TMU1_OFFSET",  /* 12 */
  "GR_VERTEX_TOW_TMU1_OFFSET",  /* 13 */
  "GR_VERTEX_OOW_TMU1_OFFSET"  /* 14 */
};  
#endif

/*
 **  _trisetup
 **
 **  This routine does all the setup needed for drawing a triangle.  It
 **  is intended to be an exact specification for the mechanisim used
 **  to pass vertices to the assembly language triangle setup code, and
 **  as such has no optimizations at all.  Whenever a 'shortcut'
 **  routine to draw triangles (such as for antialiasing, fast lines,
 **  fast spans, polygons, etc) is needed, this code should be used as
 **  the starting point.
**
*/  

#if ( GLIDE_PLATFORM & GLIDE_HW_SST96 )
GR_DDFUNC(_trisetup, FxI32, ( const void *va, const void *vb, const void *vc ))
{
  GR_DCL_GC;
  FxI32 xindex = (gc->state.vData.vertexInfo.offset >> 2);
  FxI32 yindex = xindex + 1;
  const float *fa = (const float *)va + xindex;
  const float *fb = (const float *)vb + xindex;
  const float *fc = (const float *)vc + xindex;
  float ooa, dxAB, dxBC, dyAB, dyBC;
  int i,culltest;
  union { float f; int i; } ay;
  union { float f; int i; } by;
  union { float f; int i; } cy;
  float *fp;
  struct dataList_s *dlp;
  volatile FxU32 *fifoPtr;
  float snap_xa, snap_ya, snap_xb, snap_yb, snap_xc, snap_yc;
  
  culltest = gc->state.cull_mode; /* 1 if negative, 0 if positive */
  _GlideRoot.stats.trisProcessed++;
  
  {
    snap_ya = (volatile float) (*((float *)va + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)vb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)vc + yindex) + SNAP_BIAS);
  }
  /*
   **  Sort the vertices.
   **  Whenever the radial order is reversed (from counter-clockwise to
   **  clockwise), we need to change the area of the triangle.  Note
   **  that we know the first two elements are X & Y by looking at the
   **  grVertex structure.  
   */
  ay.f = snap_ya;
  by.f = snap_yb;
  cy.f = snap_yc;
  if (ay.i < 0) ay.i ^= 0x7FFFFFFF;
  if (by.i < 0) by.i ^= 0x7FFFFFFF;
  if (cy.i < 0) cy.i ^= 0x7FFFFFFF;
  if (ay.i < by.i) {
    if (by.i > cy.i) {              /* acb */
      if (ay.i < cy.i) {
        fa = (const float *)va + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)vb + xindex;
        culltest ^= 1;
      } else {                  /* cab */
        fa = (const float *)vc + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vb + xindex;
      }
      /* else it's already sorted */
    }
  } else {
    if (by.i < cy.i) {              /* bac */
      if (ay.i < cy.i) {
        fa = (const float *)vb + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vc + xindex;
        culltest ^= 1;
      } else {                  /* bca */
        fa = (const float *)vb + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)va + xindex;
      }
    } else {                    /* cba */
      fa = (const float *)vc + xindex;
      fb = (const float *)vb + xindex;
      fc = (const float *)va + xindex;
      culltest ^= 1;
    }
  }
  {
    snap_xa = (volatile float) (*((float *)fa + xindex) + SNAP_BIAS);
    snap_xb = (volatile float) (*((float *)fb + xindex) + SNAP_BIAS);
    snap_xc = (volatile float) (*((float *)fc + xindex) + SNAP_BIAS);
    snap_ya = (volatile float) (*((float *)fa + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)fb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)fc + yindex) + SNAP_BIAS);
  }

  /* Compute Area */
  dxAB = snap_xa - snap_xb;
  dxBC = snap_xb - snap_xc;
  
  dyAB = snap_ya - snap_yb;
  dyBC = snap_yb - snap_yc;
  
  /* this is where we store the area */
  _GlideRoot.pool.temp1.f = dxAB * dyBC - dxBC * dyAB;
  
  /* Zero-area triangles are BAD!! */
  if ((_GlideRoot.pool.temp1.i & 0x7FFFFFFF) == 0)
    return 0;
  
  /* Backface culling, use sign bit as test */
  if (gc->state.cull_mode != GR_CULL_DISABLE) {
    if ((_GlideRoot.pool.temp1.i ^ (culltest<<31)) >= 0) {
      return -1;
    }
  }

#if GL_X86  
  /* Fence On P6 If Necessary */
  if ( _GlideRoot.CPUType == 6 ) {
      /* In the macro there is a slop of 4 DWORDS that I have removed */
      if ( (gc->hwDep.sst96Dep.writesSinceFence + ( _GlideRoot.curTriSize >> 2 )) > 128 ) {
          P6FENCE;
          gc->hwDep.sst96Dep.writesSinceFence = 0;
      }
      gc->hwDep.sst96Dep.writesSinceFence      += 
          _GlideRoot.curTriSize>>2;
  }
#endif

  /* Wrap Fifo now if triangle is going to incur a wrap */
  if (gc->fifoData.hwDep.vg96FIFOData.fifoSize < (FxU32) _GlideRoot.curTriSize ) {
#if SST96_ALT_FIFO_WRAP
    gc->fifoData.hwDep.vg96FIFOData.blockSize = _GlideRoot.curTriSize;
    initWrapFIFO(&gc->fifoData);
#else
    _grSst96FifoMakeRoom();
#endif
  }
  
  GR_FLUSH_STATE();

  GR_SET_EXPECTED_SIZE(_GlideRoot.curTriSize);

  /* Grab fifo pointer into a local */
  fifoPtr = gc->fifoData.hwDep.vg96FIFOData.fifoPtr;
  
  /* Settle Bookeeping */
  gc->fifoData.hwDep.vg96FIFOData.fifoSize -= _GlideRoot.curTriSize;
  gc->fifoData.hwDep.vg96FIFOData.fifoPtr  += _GlideRoot.curTriSize>>2;
  
  /* Start first group write packet */
  SET_GW_CMD(    fifoPtr, 0, gc->hwDep.sst96Dep.gwCommand );
  SET_GW_HEADER( fifoPtr, 1, gc->hwDep.sst96Dep.gwHeaders[0] );

  ooa = _GlideRoot.pool.f1 / _GlideRoot.pool.temp1.f;
  /* GMT: note that we spread out our PCI writes */
  /* write out X & Y for vertex A */
  FSET_GW_ENTRY( fifoPtr, 2, snap_xa );
  FSET_GW_ENTRY( fifoPtr, 3, snap_ya );

  dlp = gc->dataList;
  i = dlp->i;
  
  /* write out X & Y for vertex B */
  FSET_GW_ENTRY( fifoPtr, 4, snap_xb );
  FSET_GW_ENTRY( fifoPtr, 5, snap_yb );

  /* write out X & Y for vertex C */
  FSET_GW_ENTRY( fifoPtr, 6, snap_xc );
  FSET_GW_ENTRY( fifoPtr, 7, snap_yc );
  fifoPtr += 8;

  /*
  ** Divide the deltas by the area for gradient calculation.
  */
  dxBC *= ooa;
  dyAB *= ooa;
  dxAB *= ooa;
  dyBC *= ooa;

  while (i) {
    fp = dlp->addr;
    /* chip field change */
    if (i & 1) 
      goto secondary_packet;
    else {
      float dpAB, dpBC,dpdx, dpdy;
      FxU32 bddr = dlp->bddr;
      if (bddr) {
        /* packed data (color) */
        dpBC = FbARRAY(fb,bddr);
        dpdx = FbARRAY(fa,bddr);
        FSET_GW_ENTRY( fifoPtr, 0, dpdx );
        dpAB = dpdx - dpBC;
        dpBC = dpBC - FbARRAY(fc,bddr);
      }
      else {
        /* non packed data */
        dpBC = FARRAY(fb,i);
        dpdx = FARRAY(fa,i);
        FSET_GW_ENTRY( fifoPtr, 0, dpdx );
        dpAB = dpdx - dpBC;
        dpBC = dpBC - FARRAY(fc,i);
      }
      dpdx = dpAB * dyBC - dpBC * dyAB;
      FSET_GW_ENTRY( fifoPtr, 1, dpdx );
      dpdy = dpBC * dxAB - dpAB * dxBC;
      dlp++;
      i = dlp->i;
      FSET_GW_ENTRY( fifoPtr, 2, dpdy );
      fifoPtr += 3;
    }
  }

  /* write triangle command */
triangle_command:
  FSET_GW_ENTRY( fifoPtr, 0, _GlideRoot.pool.temp1.f );
  fifoPtr+=1;
  
  if (((FxU32)fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr += 1;
  }

  GR_ASSERT(fifoPtr == gc->fifoData.hwDep.vg96FIFOData.fifoPtr);

  GR_CHECK_SIZE();
  
  _GlideRoot.stats.trisDrawn++;

  return 1;

secondary_packet:
  /* Round out last header */
  if (((FxU32) fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr  += 1;
  }
  /* Start new packet
     note, there can only ever be two different packets 
     using gwHeaderNum++ would be more general, but this
     reflects the actual implementation */
  SET_GW_CMD(    fifoPtr, 0, (FxU32)fp );
  SET_GW_HEADER( fifoPtr, 1, gc->hwDep.sst96Dep.gwHeaders[1] );
  fifoPtr+=2;
  dlp++;
  i = dlp->i;
  while( i ) {
    float dpAB, dpBC,dpdx, dpdy;
    FxU32 bddr = dlp->bddr;
    fp = dlp->addr;
    if (bddr) {
      /* packed data (color) */
      dpBC = FbARRAY(fb,bddr);
      dpdx = FbARRAY(fa,bddr);
      FSET_GW_ENTRY( fifoPtr, 0, dpdx );
      dpAB = dpdx - dpBC;
      dpBC = dpBC - FbARRAY(fc,bddr);
    }
    else {
      /* non packed data */
      dpBC = FARRAY(fb,i);
      dpdx = FARRAY(fa,i);
      FSET_GW_ENTRY( fifoPtr, 0, dpdx );
      dpAB = dpdx - dpBC;
      dpBC = dpBC - FARRAY(fc,i);
    }
    dpdx = dpAB * dyBC - dpBC * dyAB;
    FSET_GW_ENTRY( fifoPtr, 1, dpdx );
    dpdy = dpBC * dxAB - dpAB * dxBC;
    dlp++;
    i = dlp->i;
    FSET_GW_ENTRY( fifoPtr, 2, dpdy );
    fifoPtr += 3;
  }

  if (((FxU32)fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr += 1;
  }
  SET_GW_CMD(    fifoPtr, 0, gc->hwDep.sst96Dep.gwCommand );
  SET_GW_HEADER( fifoPtr, 1, GW_TRICMD_MASK );
  fifoPtr += 2;
  goto triangle_command;
} /* _trisetup */
#elif ( GLIDE_PLATFORM & GLIDE_HW_SST1 )
GR_DDFUNC(_trisetup, FxI32, ( const void *va, const void *vb, const void *vc ))
{
  GR_DCL_GC;
  GR_DCL_HW;
  FxI32 xindex = (gc->state.vData.vertexInfo.offset >> 2);
  FxI32 yindex = xindex + 1;
  const float *fa = (const float *)va + xindex;
  const float *fb = (const float *)vb + xindex;
  const float *fc = (const float *)vc + xindex;
  float ooa, dxAB, dxBC, dyAB, dyBC;
  int i,culltest;
  union { float f; int i; } ay;
  union { float f; int i; } by;
  union { float f; int i; } cy;
  float *fp;
  struct dataList_s *dlp;
  float snap_xa, snap_ya, snap_xb, snap_yb, snap_xc, snap_yc;

  culltest = gc->state.cull_mode; /* 1 if negative, 0 if positive */
  _GlideRoot.stats.trisProcessed++;
  
  {
    snap_ya = (volatile float) (*((float *)va + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)vb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)vc + yindex) + SNAP_BIAS);
  }
  /*
   **  Sort the vertices.
   **  Whenever the radial order is reversed (from counter-clockwise to
   **  clockwise), we need to change the area of the triangle.  Note
   **  that we know the first two elements are X & Y by looking at the
   **  grVertex structure.  
   */
  ay.f = snap_ya;
  by.f = snap_yb;
  cy.f = snap_yc;
  if (ay.i < 0) ay.i ^= 0x7FFFFFFF;
  if (by.i < 0) by.i ^= 0x7FFFFFFF;
  if (cy.i < 0) cy.i ^= 0x7FFFFFFF;
  if (ay.i < by.i) {
    if (by.i > cy.i) {              /* acb */
      if (ay.i < cy.i) {
        fa = (const float *)va + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)vb + xindex;
        culltest ^= 1;
      } else {                  /* cab */
        fa = (const float *)vc + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vb + xindex;
      }
      /* else it's already sorted */
    }
  } else {
    if (by.i < cy.i) {              /* bac */
      if (ay.i < cy.i) {
        fa = (const float *)vb + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vc + xindex;
        culltest ^= 1;
      } else {                  /* bca */
        fa = (const float *)vb + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)va + xindex;
      }
    } else {                    /* cba */
      fa = (const float *)vc + xindex;
      fb = (const float *)vb + xindex;
      fc = (const float *)va + xindex;
      culltest ^= 1;
    }
  }
  {
    snap_xa = (volatile float) (*((float *)fa + xindex) + SNAP_BIAS);
    snap_xb = (volatile float) (*((float *)fb + xindex) + SNAP_BIAS);
    snap_xc = (volatile float) (*((float *)fc + xindex) + SNAP_BIAS);
    snap_ya = (volatile float) (*((float *)fa + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)fb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)fc + yindex) + SNAP_BIAS);
  }

#if 0
#if (GLIDE_PLATFORM & GLIDE_HW_SST1) && defined(GLIDE_USE_ALT_REGMAP)
  hw = SST_WRAP(hw,128);                /* use alternate register mapping */
#endif
#endif
  /* Compute Area */
  dxAB = snap_xa - snap_xb;
  dxBC = snap_xb - snap_xc;
  
  dyAB = snap_ya - snap_yb;
  dyBC = snap_yb - snap_yc;
  
  /* this is where we store the area */
  _GlideRoot.pool.temp1.f = dxAB * dyBC - dxBC * dyAB;
  
  /* Zero-area triangles are BAD!! */
  if ((_GlideRoot.pool.temp1.i & 0x7FFFFFFF) == 0)
    return 0;
  
  /* Backface culling, use sign bit as test */
  if (gc->state.cull_mode != GR_CULL_DISABLE) {
    if ((_GlideRoot.pool.temp1.i ^ (culltest<<31)) >= 0) {
      return -1;
    }
  }
  
  GR_FLUSH_STATE();

  GR_SET_EXPECTED_SIZE(_GlideRoot.curTriSize);
  
  ooa = _GlideRoot.pool.f1 / _GlideRoot.pool.temp1.f;
  /* GMT: note that we spread out our PCI writes */
  /* write out X & Y for vertex A */
  GR_SETF( hw->FvA.x, snap_xa );
  GR_SETF( hw->FvA.y, snap_ya );

  dlp = gc->dataList;
  i = dlp->i;
  
  /* write out X & Y for vertex B */
  GR_SETF( hw->FvB.x, snap_xb );
  GR_SETF( hw->FvB.y, snap_yb );
  
  /* write out X & Y for vertex C */
  GR_SETF( hw->FvC.x, snap_xc );
  GR_SETF( hw->FvC.y, snap_yc );

  /*
  ** Divide the deltas by the area for gradient calculation.
  */
  dxBC *= ooa;
  dyAB *= ooa;
  dxAB *= ooa;
  dyBC *= ooa;

  /* 
  ** The src vector contains offsets from fa, fb, and fc to for which
  **  gradients need to be calculated, and is null-terminated.
  */
  while (i) {
    fp = dlp->addr;
    if (i & 1) {                   /* packer bug check */
      if (i & 2) P6FENCE;
      GR_SETF( fp[0], 0.0F );
      if (i & 2) P6FENCE;
      dlp++;
      i = dlp->i;
    }
    else {
      float dpAB, dpBC,dpdx, dpdy;
      FxU32 bddr = dlp->bddr;
      
      if (bddr) {
        /* packed data (color) */
        dpBC = FbARRAY(fb,bddr);
        dpdx = FbARRAY(fa,bddr);
        GR_SETF( fp[0], dpdx );
        
        dpAB = dpdx - dpBC;
        dpBC = dpBC - FbARRAY(fc,bddr);
      }
      else {
        dpBC = FARRAY(fb,i);
        dpdx = FARRAY(fa,i);
        GR_SETF( fp[0], dpdx );
        
        dpAB = dpdx - dpBC;
        dpBC = dpBC - FARRAY(fc,i);
      }
      dpdx = dpAB * dyBC - dpBC * dyAB;
        
      GDBG_INFO((285,"p0,1x: %g %g dpdx: %g\n",dpAB * dyBC,dpBC * dyAB,dpdx));
      GR_SETF( fp[DPDX_OFFSET>>2] , dpdx );
      dpdy = dpBC * dxAB - dpAB * dxBC;
      
      GDBG_INFO((285,"p0,1y: %g %g dpdy: %g\n",dpBC * dxAB,dpAB * dxBC,dpdy));
      dlp++;
      i = dlp->i;
      GR_SETF( fp[DPDY_OFFSET>>2] , dpdy );
    }
  }

  /* Draw the triangle by writing the area to the triangleCMD register */
  P6FENCE_CMD( GR_SETF( hw->FtriangleCMD, _GlideRoot.pool.temp1.f ) );
  _GlideRoot.stats.trisDrawn++;

  GR_CHECK_SIZE();
  _GlideRoot.stats.trisDrawn++;
  return 1;
} /* _trisetup */
#else
#error "Need to implement trisetup for this platform"
#endif

/*
**  _trisetup_nogradients
**
**  This routine does all the setup needed for drawing a triangle except
**  that it assumes that the gradients are already in the chip.
**
**  Also we assume we don't have to test for backface triangles - this is
**  typically done outside this routine, as in grDrawPlanarPolygon
*/  

#if ( GLIDE_PLATFORM & GLIDE_HW_SST96 )
GR_DDFUNC(_trisetup_nogradients, FxI32, ( const void *va, const void *vb, const void *vc ))
{
  GR_DCL_GC;
  FxI32 xindex = (gc->state.vData.vertexInfo.offset >> 2);
  FxI32 yindex = xindex + 1;
  const float *fa = (const float *)va + xindex;
  const float *fb = (const float *)vb + xindex;
  const float *fc = (const float *)vc + xindex;
  float dxAB, dxBC, dyAB, dyBC;
  int i;
  union { float f; int i; } ay;
  union { float f; int i; } by;
  union { float f; int i; } cy;
  float *fp;
  struct dataList_s *dlp;
  volatile FxU32 *fifoPtr;
  float snap_xa, snap_ya, snap_xb, snap_yb, snap_xc, snap_yc;

  {
    snap_ya = (volatile float) (*((float *)va + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)vb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)vc + yindex) + SNAP_BIAS);
  }
  /*
  **  Sort the vertices.
  **  Whenever the radial order is reversed (from counter-clockwise to
  **  clockwise), we need to change the area of the triangle.  Note
  **  that we know the first two elements are X & Y by looking at the
  **  grVertex structure.  
  */
  ay.f = snap_ya;
  by.f = snap_yb;
  cy.f = snap_yc;
  if (ay.i < 0) ay.i ^= 0x7FFFFFFF;
  if (by.i < 0) by.i ^= 0x7FFFFFFF;
  if (cy.i < 0) cy.i ^= 0x7FFFFFFF;
  if (ay.i < by.i) {
    if (by.i > cy.i) {              /* acb */
      if (ay.i < cy.i) {
        fa = (const float *)va + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)vb + xindex;
      } else {                  /* cab */
        fa = (const float *)vc + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vb + xindex;
      }
      /* else it's already sorted */
    }
  } else {
    if (by.i < cy.i) {              /* bac */
      if (ay.i < cy.i) {
        fa = (const float *)vb + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vc + xindex;
      } else {                  /* bca */
        fa = (const float *)vb + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)va + xindex;
      }
    } else {                    /* cba */
      fa = (const float *)vc + xindex;
      fb = (const float *)vb + xindex;
      fc = (const float *)va + xindex;
    }
  }
  {
    snap_xa = (volatile float) (*((float *)fa + xindex) + SNAP_BIAS);
    snap_xb = (volatile float) (*((float *)fb + xindex) + SNAP_BIAS);
    snap_xc = (volatile float) (*((float *)fc + xindex) + SNAP_BIAS);
    snap_ya = (volatile float) (*((float *)fa + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)fb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)fc + yindex) + SNAP_BIAS);
  }

  /* Compute Area */
  dxAB = snap_xa - snap_xb;
  dxBC = snap_xb - snap_xc;
  
  dyAB = snap_ya - snap_yb;
  dyBC = snap_yb - snap_yc;

  /* this is where we store the area */
  _GlideRoot.pool.temp1.f = dxAB * dyBC - dxBC * dyAB;
  _GlideRoot.stats.trisProcessed++;

  /* Zero-area triangles are BAD!! */
  if ((_GlideRoot.pool.temp1.i & 0x7FFFFFFF) == 0) {
    return 0;
  }

#if GL_X86
  /* Fence On P6 If Necessary */
  if ( _GlideRoot.CPUType == 6 ) {
      /* In the macro there is a slop of 4 DWORDS that I have removed */
      if ( (gc->hwDep.sst96Dep.writesSinceFence + 
            ( _GlideRoot.curTriSizeNoGradient >> 2 )) > 128 ) {
          P6FENCE;
          gc->hwDep.sst96Dep.writesSinceFence = 0;
      }
      gc->hwDep.sst96Dep.writesSinceFence      += 
          _GlideRoot.curTriSizeNoGradient>>2;
  }
#endif

  /* Wrap Fifo now if triangle is going to incur a wrap */
  if (gc->fifoData.hwDep.vg96FIFOData.fifoSize < (FxU32) _GlideRoot.curTriSizeNoGradient ) {
#if SST96_ALT_FIFO_WRAP
    gc->fifoData.hwDep.vg96FIFOData.blockSize = _GlideRoot.curTriSizeNoGradient;
    initWrapFIFO(&gc->fifoData);
#else
    _grSst96FifoMakeRoom();
#endif
  }

  /* bookeeping */
  fifoPtr = gc->fifoData.hwDep.vg96FIFOData.fifoPtr;
  gc->fifoData.hwDep.vg96FIFOData.fifoSize -= _GlideRoot.curTriSizeNoGradient;
  gc->fifoData.hwDep.vg96FIFOData.fifoPtr  += _GlideRoot.curTriSizeNoGradient>>2;

  /* packet header */
  SET_GW_CMD(    fifoPtr, 0, gc->hwDep.sst96Dep.gwCommand );
  SET_GW_HEADER( fifoPtr, 1, (gc->hwDep.sst96Dep.gwHeaders[0] & ~GWH_DXY_BITS) );

  FSET_GW_ENTRY( fifoPtr, 2, snap_xa );
  FSET_GW_ENTRY( fifoPtr, 3, snap_ya );

  dlp = gc->dataList;
  i = dlp->i;
  
  /* write out X & Y for vertex B */
  FSET_GW_ENTRY( fifoPtr, 4, snap_xb );
  FSET_GW_ENTRY( fifoPtr, 5, snap_yb );

  /* write out X & Y for vertex C */
  FSET_GW_ENTRY( fifoPtr, 6, snap_xc );
  FSET_GW_ENTRY( fifoPtr, 7, snap_yc );
  fifoPtr += 8;

  dlp = gc->dataList;
  i = dlp->i;

  while (i) {
    fp = dlp->addr;
    if (i & 1) 
      goto secondary_packet;
    else {
      float dpdx;
      if (dlp->bddr) {
        /* packed data (color) */
        dpdx = FbARRAY(fa,dlp->bddr);
      }
      else {
        /* non-packed color */
        dpdx = FARRAY(fa,i);
      }
      FSET_GW_ENTRY( fifoPtr, 0, dpdx );
      fifoPtr += 1;
      dlp++;
      i = dlp->i;
    }
  }

triangle_command:
  FSET_GW_ENTRY( fifoPtr, 0, _GlideRoot.pool.temp1.f );
  fifoPtr += 1;

  if (((FxU32)fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr += 1;
  }

  GR_ASSERT(fifoPtr == gc->fifoData.hwDep.vg96FIFOData.fifoPtr);

  _GlideRoot.stats.trisDrawn++;

  return 1;

secondary_packet:
  /* Round out last packet */
  if (((FxU32) fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr  += 1;
  }

  /* Start new packet
     note, there can only ever be two different packets 
     using gwHeaderNum++ would be more general, but this
     reflects the actual implementation */
  SET_GW_CMD(    fifoPtr, 0, (FxU32)fp );
  SET_GW_HEADER( fifoPtr, 1, gc->hwDep.sst96Dep.gwHeaders[1] );
  fifoPtr+=2;
  dlp++;
  i = dlp->i;
  
  while( i ) {
    if (dlp->bddr) {
      /* packed data (color) */
      FSET_GW_ENTRY( fifoPtr, 0, FbARRAY(fa,dlp->bddr));
    }
    else {
      /* non packed color */
      FSET_GW_ENTRY( fifoPtr, 0, FARRAY(fa,i));
    }
    dlp++;
    i = dlp->i;
    fifoPtr += 1;
  }

  if (((FxU32)fifoPtr) & 0x7) {
    FSET_GW_ENTRY( fifoPtr, 0, 0.0f );
    fifoPtr += 1;
  }
  SET_GW_CMD(    fifoPtr, 0, gc->hwDep.sst96Dep.gwCommand );
  SET_GW_HEADER( fifoPtr, 1, GW_TRICMD_MASK );
  fifoPtr += 2;
  goto triangle_command;
} /* _trisetup_nogradients */

#elif ( GLIDE_PLATFORM & GLIDE_HW_SST1 )

GR_DDFUNC(_trisetup_nogradients, FxI32, ( const void *va, const void *vb, const void *vc ))
{
  GR_DCL_GC;
  GR_DCL_HW;
  FxI32 xindex = (gc->state.vData.vertexInfo.offset >> 2);
  FxI32 yindex = xindex + 1;
  const float *fa = (const float *)va + xindex;
  const float *fb = (const float *)vb + xindex;
  const float *fc = (const float *)vc + xindex;
  float dxAB, dxBC, dyAB, dyBC;
  int i;
  union { float f; int i; } ay;
  union { float f; int i; } by;
  union { float f; int i; } cy;
  float *fp;
  struct dataList_s *dlp;
  float snap_xa, snap_ya, snap_xb, snap_yb, snap_xc, snap_yc;

  {
    snap_ya = (volatile float) (*((float *)va + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)vb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)vc + yindex) + SNAP_BIAS);
  }
  /*
  **  Sort the vertices.
  **  Whenever the radial order is reversed (from counter-clockwise to
  **  clockwise), we need to change the area of the triangle.  Note
  **  that we know the first two elements are X & Y by looking at the
  **  grVertex structure.  
  */
  ay.f = snap_ya;
  by.f = snap_yb;
  cy.f = snap_yc;
  if (ay.i < 0) ay.i ^= 0x7FFFFFFF;
  if (by.i < 0) by.i ^= 0x7FFFFFFF;
  if (cy.i < 0) cy.i ^= 0x7FFFFFFF;
  if (ay.i < by.i) {
    if (by.i > cy.i) {              /* acb */
      if (ay.i < cy.i) {
        fa = (const float *)va + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)vb + xindex;
      } else {                  /* cab */
        fa = (const float *)vc + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vb + xindex;
      }
      /* else it's already sorted */
    }
  } else {
    if (by.i < cy.i) {              /* bac */
      if (ay.i < cy.i) {
        fa = (const float *)vb + xindex;
        fb = (const float *)va + xindex;
        fc = (const float *)vc + xindex;
      } else {                  /* bca */
        fa = (const float *)vb + xindex;
        fb = (const float *)vc + xindex;
        fc = (const float *)va + xindex;
      }
    } else {                    /* cba */
      fa = (const float *)vc + xindex;
      fb = (const float *)vb + xindex;
      fc = (const float *)va + xindex;
    }
  }
  {
    snap_xa = (volatile float) (*((float *)fa + xindex) + SNAP_BIAS);
    snap_xb = (volatile float) (*((float *)fb + xindex) + SNAP_BIAS);
    snap_xc = (volatile float) (*((float *)fc + xindex) + SNAP_BIAS);
    snap_ya = (volatile float) (*((float *)fa + yindex) + SNAP_BIAS);
    snap_yb = (volatile float) (*((float *)fb + yindex) + SNAP_BIAS);
    snap_yc = (volatile float) (*((float *)fc + yindex) + SNAP_BIAS);
  }

#if (GLIDE_PLATFORM & GLIDE_HW_SST1) && defined(GLIDE_USE_ALT_REGMAP)
  hw = SST_WRAP(hw,128);                /* use alternate register mapping */
#endif

  GR_SET_EXPECTED_SIZE(_GlideRoot.curTriSizeNoGradient);

   /* GMT: note that we spread out our PCI writes */
   /* write out X & Y for vertex A */
   GR_SETF( hw->FvA.x, snap_xa );
   GR_SETF( hw->FvA.y, snap_ya );

   /* Compute Area */
   dxAB = snap_xa - snap_xb;
   dxBC = snap_xb - snap_xc;
   
   dyAB = snap_ya - snap_yb;
   dyBC = snap_yb - snap_yc;

   /* write out X & Y for vertex B */
   GR_SETF( hw->FvB.x, snap_xb );
   GR_SETF( hw->FvB.y, snap_yb );

  /* this is where we store the area */
  _GlideRoot.pool.temp1.f = dxAB * dyBC - dxBC * dyAB;
  _GlideRoot.stats.trisProcessed++;

  /* Zero-area triangles are BAD!! */
  if ((_GlideRoot.pool.temp1.i & 0x7FFFFFFF) == 0) {
    GR_CHECK_SIZE_SLOPPY();
    return 0;
  }
  
  /* write out X & Y for vertex C */
  GR_SETF( hw->FvC.x, snap_xc );
  GR_SETF( hw->FvC.y, snap_yc );

   dlp = gc->dataList;
   i = dlp->i;

  /* 
  ** The src vector contains offsets from fa, fb, and fc to for which
  ** gradients need to be calculated, and is null-terminated.
  */
  while (i) {
    fp = dlp->addr;
    if (i & 1) {                   /* packer bug check */
      if (i & 2) P6FENCE;
      GR_SETF( fp[0], 0.0F );
      if (i & 2) P6FENCE;
      dlp++;
      i = dlp->i;
    }
    else {
      if (dlp->bddr) {
        /* packed color */
        GR_SETF( fp[0], FbARRAY(fa,dlp->bddr) );
      }
      else {
        /* non packed data */
        GR_SETF( fp[0], FARRAY(fa,i) );
      }
      dlp++;
      i = dlp->i;
    }
  }

  /* Draw the triangle by writing the area to the triangleCMD register */
  P6FENCE_CMD( GR_SETF( hw->FtriangleCMD, _GlideRoot.pool.temp1.f ) );
  _GlideRoot.stats.trisDrawn++;

  GR_CHECK_SIZE();
  return 1;
} /* _trisetup_nogradients */
#else
#error  "Need Triangle Setup code for this hardware"
#endif

