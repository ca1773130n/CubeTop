#pragma once
#include "stdafx.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "MEM_guardedalloc.h"

#ifdef WIN32
#include "BLI_winstuff.h"
#endif

#include "GEN_messaging.h"

#include "DNA_ID.h"
#include "DNA_scene_types.h"

#include "BLI_blenlib.h"
#include "blendef.h" /* for MAXFRAME */


#include "BKE_utildefines.h"
#include "BKE_blender.h"
#include "BKE_font.h"
#include "BKE_global.h"
#include "BKE_main.h"
#include "BKE_material.h"
#include "BKE_packedFile.h"
#include "BKE_scene.h"
#include "BKE_node.h"
#include "BKE_subsurf.h"
//#include "CCGSubSurf.h"

#include "BIF_gl.h"
#include "BIF_graphics.h"
#include "BIF_mainqueue.h"
#include "BIF_graphics.h"
#include "BIF_editsound.h"
#include "BIF_usiblender.h"
#include "BIF_drawscene.h"      /* set_scene() */
#include "BIF_screen.h"         /* waitcursor and more */
#include "BIF_usiblender.h"
#include "BIF_toolbox.h"

#include "BLO_writefile.h"
#include "BLO_readfile.h"

#include "BDR_drawmesh.h"

#include "IMB_imbuf.h"	// for quicktime_init

#include "BPY_extern.h"

#include "RE_pipeline.h"

#include "playanim_ext.h"
#include "mydevice.h"
#include "nla.h"
#include "datatoc.h"

/* for passing information between creator and gameengine */
#include "SYS_System.h"

#include <signal.h>

#ifdef __FreeBSD__
# include <sys/types.h>
# include <floatingpoint.h>
# include <sys/rtprio.h>
#endif

#ifdef WITH_BINRELOC
#include "binreloc.h"
#endif

#include "BPY_menus.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "MEM_guardedalloc.h"

#include "PIL_time.h"

#include "Object.h"
#include "DNA_armature_types.h"
#include "DNA_customdata_types.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"
#include "DNA_object_force.h"
#include "DNA_screen_types.h"
#include "DNA_key_types.h"
#include "DNA_scene_types.h"
#include "DNA_view3d_types.h"
#include "DNA_material_types.h"
#include "DNA_modifier_types.h"
#include "DNA_texture_types.h"
#include "DNA_userdef_types.h"
#include "DNA_curve_types.h"
#include "DNA_text_types.h"

#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_editVert.h"
#include "BLI_dynstr.h"
#include "BLI_rand.h"

#include "BKE_DerivedMesh.h"
#include "BKE_depsgraph.h"
#include "BKE_cloth.h"
#include "BKE_customdata.h"
#include "BKE_global.h"
#include "BKE_key.h"
#include "BKE_library.h"
#include "BKE_main.h"
#include "BKE_material.h"
#include "BKE_mesh.h"
#include "BKE_modifier.h"
#include "BKE_multires.h"
#include "BKE_object.h"
#include "BKE_pointcache.h"
#include "BKE_softbody.h"
#include "BKE_texture.h"
#include "BKE_utildefines.h"
#include "BKE_text.h"

#ifdef WITH_VERSE
#include "BKE_verse.h"
#endif

#include "BIF_editkey.h"
#include "BIF_editmesh.h"
#include "BIF_editmode_undo.h"
#include "BIF_interface.h"
#include "BIF_meshtools.h"
#include "BIF_mywindow.h"
#include "BIF_retopo.h"
#include "BIF_space.h"
#include "BIF_screen.h"
#include "BIF_toolbox.h"
#include "BIF_fsmenu.h"

#include "BIF_gl.h"
#include "BIF_interface.h"
#include "BIF_usiblender.h"
#include "BIF_drawtext.h"
#include "BIF_editaction.h"
#include "BIF_editarmature.h"
#include "BIF_editlattice.h"
#include "BIF_editfont.h"
#include "BIF_editmesh.h"
#include "BIF_editmode_undo.h"
#include "BIF_editsound.h"
#include "BIF_filelist.h"
#include "BIF_poseobject.h"
#include "BIF_previewrender.h"
#include "BIF_renderwin.h"
#include "BIF_resources.h"
#include "BIF_screen.h"
#include "BIF_space.h"
#include "BIF_toolbox.h"
#include "BIF_cursors.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <windows.h> /* need to include windows.h so _WIN32_IE is defined  */
#ifndef _WIN32_IE
#define _WIN32_IE 0x0400 /* minimal requirements for SHGetSpecialFolderPath on MINGW MSVC has this defined already */
#endif
#include <shlobj.h> /* for SHGetSpecialFolderPath, has to be done before BLI_winstuff because 'near' is disabled through BLI_windstuff */
#include "BLI_winstuff.h"
#include <process.h> /* getpid */
#else
#include <unistd.h> /* getpid */
#endif
#include "MEM_guardedalloc.h"
//#include "MEM_CacheLimiterC-Api.h"

//#include "BMF_Api.h"
#include "BIF_language.h"
#ifdef INTERNATIONAL
#include "FTF_Api.h"
#endif

#include "BLI_blenlib.h"
#include "BLI_arithb.h"
#include "BLI_linklist.h"

#include "IMB_imbuf_types.h"
#include "IMB_imbuf.h"

#include "DNA_object_types.h"
#include "DNA_space_types.h"
#include "DNA_userdef_types.h"
#include "DNA_sound_types.h"
#include "DNA_scene_types.h"
#include "DNA_screen_types.h"

#include "BKE_blender.h"
#include "BKE_curve.h"
#include "BKE_displist.h"
#include "BKE_DerivedMesh.h"
#include "BKE_exotic.h"
#include "BKE_font.h"
#include "BKE_global.h"
#include "BKE_main.h"
//#include "BKE_mball.h"
#include "BKE_node.h"
#include "BKE_packedFile.h"
#include "BKE_texture.h"
#include "BKE_utildefines.h"
#include "BKE_pointcache.h"

#ifdef WITH_VERSE
#include "BKE_verse.h"
#endif

#include "BLI_vfontdata.h"

#include "BIF_fsmenu.h"
#include "BIF_gl.h"
#include "BIF_interface.h"
#include "BIF_usiblender.h"
#include "BIF_drawtext.h"
#include "BIF_editaction.h"
#include "BIF_editarmature.h"
#include "BIF_editlattice.h"
#include "BIF_editfont.h"
#include "BIF_editmesh.h"
#include "BIF_editmode_undo.h"
#include "BIF_editsound.h"
#include "BIF_filelist.h"
#include "BIF_poseobject.h"
#include "BIF_previewrender.h"
#include "BIF_renderwin.h"
#include "BIF_resources.h"
#include "BIF_screen.h"
#include "BIF_space.h"
#include "BIF_toolbox.h"
#include "BIF_cursors.h"
#include "BIF_usiblender.h"

#ifdef WITH_VERSE
#include "BIF_verse.h"
#endif


#include "BSE_drawview.h"
#include "BSE_edit.h"
#include "BSE_editipo.h"
#include "BSE_filesel.h"
#include "BSE_headerbuttons.h"
#include "BSE_node.h"

#include "BLO_readfile.h"
#include "BLO_writefile.h"

#include "BDR_drawobject.h"
#include "BDR_editobject.h"
#include "BDR_editcurve.h"
#include "BDR_imagepaint.h"
#include "BDR_vpaint.h"

#include "BPY_extern.h"

#include "blendef.h"

#include "RE_pipeline.h"		/* RE_ free stuff */

#include "radio.h"
#include "datatoc.h"

#include "SYS_System.h"

#include "PIL_time.h"



#include "BSE_drawview.h"
#include "BSE_edit.h"
#include "BSE_editipo.h"
#include "BSE_filesel.h"
#include "BSE_headerbuttons.h"
#include "BSE_node.h"

#include "BLO_readfile.h"
#include "BLO_writefile.h"

#include "BDR_drawobject.h"
#include "BDR_editobject.h"
#include "BDR_editcurve.h"
#include "BDR_imagepaint.h"
#include "BDR_vpaint.h"

#include "BPY_extern.h"

#include "blendef.h"

#include "RE_pipeline.h"		/* RE_ free stuff */

#include "radio.h"
#include "datatoc.h"

#include "SYS_System.h"

#include "PIL_time.h"

#ifdef WITH_VERSE
#include "BIF_verse.h"
#endif

#include "BSE_view.h"
#include "BSE_edit.h"
#include "BSE_trans_types.h"

#include "BDR_drawobject.h"
#include "BDR_editobject.h"
#include "BDR_editface.h"
#include "BDR_vpaint.h"


#include "multires.h"
#include "mydevice.h"
#include "blendef.h"

#include "EXPP_interface.h"


extern void Trace(char *fmt, ...);


#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
//
// 선언문
//
//////////////////////////////////////////////////////////////////////////

#define MERGELIMIT								0.0001f

//////////////////////////////////////////////////////////////////////////
//
// 사용되는 구조체들
//
//////////////////////////////////////////////////////////////////////////

typedef struct Collection{
	struct Collection *next, *prev;
	int index;
	ListBase collectionbase;
} Collection;

typedef struct CollectedEdge{
	struct CollectedEdge *next, *prev;
	EditEdge *eed;
} CollectedEdge;

typedef struct wUV{
	struct wUV *next, *prev;
	ListBase nodes;
	float u, v; /*cached copy of UV coordinates pointed to by nodes*/
	EditVert *eve;
	int f;
} wUV;

typedef struct wUVNode{
	struct wUVNode *next, *prev;
	float *u; /*pointer to original tface data*/
	float *v; /*pointer to original tface data*/
} wUVNode;

typedef struct wUVEdge{
	struct wUVEdge *next, *prev;
	float v1uv[2], v2uv[2]; /*nasty.*/
	struct wUV *v1, *v2; /*oriented same as editedge*/
	EditEdge *eed;
	int f;
} wUVEdge;

typedef struct wUVEdgeCollect{ /*used for grouping*/
	struct wUVEdgeCollect *next, *prev;
	wUVEdge *uved;
	int id; 
} wUVEdgeCollect;

//////////////////////////////////////////////////////////////////////////
// 
// 자체 블렌더 함수들
//
//////////////////////////////////////////////////////////////////////////

int			BL_Init( int argc, char** argv );

VOID		BL_DrawMesh( Mesh* );

void		BL_EnterEditmode( Object* pObj, int wc );

void		BL_ExitEditmode( int flag );

VOID		BL_RemoveVertexDoubles( Object* pObj, float limit );

VOID		BL_ConvertToTriMesh( Object* pObj );

VOID		BL_SubdivideFaces( Object* pObj );

FLOAT		BL_CalcDistanceVerts( MVert* v1, MVert* v2 );

static void build_edgecollection(ListBase *allcollections);
static void freecollections(ListBase *allcollections);
static void collapse_edgeuvs(void);
static void build_weldedUVs(ListBase *uvverts);
static void append_weldedUV(EditFace *efa, EditVert *eve, int tfindex, ListBase *uvverts);
static void append_weldedUVEdge(EditFace *efa, EditEdge *eed, ListBase *uvedges);
static void build_weldedUVEdges(ListBase *uvedges, ListBase *uvverts);
static void free_weldedUVs(ListBase *uvverts);
int			BL_CollapseEdges(void);

VOID		BL_MeshUnselectAll( Mesh* pMesh );

D3DXVECTOR3	BL_CalcCenter( Mesh* mesh, D3DXVECTOR3* vMiddle );

VOID		BL_ResizeMesh( Object* ob );