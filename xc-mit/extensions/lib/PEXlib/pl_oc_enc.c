/* $XConsortium: pl_oc_enc.c,v 1.6 92/11/10 11:46:11 mor Exp $ */

/******************************************************************************
Copyright 1992 by the Massachusetts Institute of Technology

                        All Rights Reserved

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation, and that the name of M.I.T. not be used in advertising or
publicity pertaining to distribution of the software without specific,
written prior permission.  M.I.T. makes no representations about the
suitability of this software for any purpose.  It is provided "as is"
without express or implied warranty.
******************************************************************************/

#include "PEXlib.h"
#include "PEXlibint.h"
#include "pl_oc_util.h"


char *
PEXEncodeOCs (float_format, oc_count, oc_data, length_return)

INPUT int		float_format;
INPUT unsigned long	oc_count;
INPUT PEXOCData		*oc_data;
OUTPUT unsigned long	*length_return;

{
    extern void		(*(PEX_encode_oc_funcs[]))();
    PEXOCData		*ocSrc;
    char		*ocDest, *ocRet;
    int			i;


    /*
     * Allocate a buffer to hold the encodings.
     */

    *length_return = PEXGetSizeOCs (float_format, oc_count, oc_data);
    ocRet = ocDest = (char *) PEXAllocBuf ((unsigned ) *length_return);


    /*
     * Now, encode the OCs in the buffer.
     */

    for (i = 0, ocSrc = oc_data; i < oc_count; i++, ocSrc++)
	(*PEX_encode_oc_funcs[ocSrc->oc_type]) (float_format, ocSrc, &ocDest);


#ifdef DEBUG
    if (ocDest - ocRet != *length_return)
    {
	printf ("PEXlib WARNING : Internal error in PEXEncodeOCs :\n");
	printf ("Data size encoded != Data size allocated.\n");
    }
#endif

    return (ocRet);
}


void _PEXEncodeEnumType (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexEnumTypeIndex enumIndex = ocSrc->data.SetMarkerType.marker_type;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type,
	sizeof (pexEnumTypeIndex), &enumIndex);
}


void _PEXEncodeTableIndex (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexTableIndex tableIndex = ocSrc->data.SetMarkerColorIndex.index;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type,
	sizeof (pexTableIndex), &tableIndex);
}


void _PEXEncodeColor (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXColorSpecifier	pcs;
    
    InitializeColorSpecifier (pcs, &(ocSrc->data.SetMarkerColor.color),
	ocSrc->data.SetMarkerColor.color_type);
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type,
	sizeof (PEXColorSpecifier) -
	AdjustSizeFromType (ocSrc->data.SetMarkerColor.color_type), &pcs);
}


void _PEXEncodeFloat (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    float f = ocSrc->data.SetMarkerScale.scale;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type, sizeof (float), &f);
}


void _PEXEncodeCARD16 (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    CARD16 c16 = ocSrc->data.SetTextPrecision.precision;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type, sizeof (CARD16), &c16);
}


void _PEXEncodeVector2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexVector2D vec;
    
    vec.x = ocSrc->data.SetCharUpVector.vector.x;
    vec.y = ocSrc->data.SetCharUpVector.vector.y;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type, sizeof (pexVector2D), &vec);
}


void _PEXEncodeTextAlignment (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexTextAlignmentData align;
    
    align.horizontal = ocSrc->data.SetTextAlignment.halignment;
    align.vertical = ocSrc->data.SetTextAlignment.valignment;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type,
        sizeof (pexTextAlignmentData), &align);
}


void _PEXEncodeCurveApprox (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCurveApprox approx;
    
    approx.approxMethod = ocSrc->data.SetCurveApprox.method;
    approx.tolerance = ocSrc->data.SetCurveApprox.tolerance;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCCurveApprox,
	sizeof (pexCurveApprox), &approx);
}


void _PEXEncodeReflectionAttr (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type,
	sizeof (PEXReflectionAttributes) -
	AdjustSizeFromType (ocSrc->data.SetReflectionAttributes.attributes.specular_color.type),
	&(ocSrc->data.SetReflectionAttributes.attributes));
}


void _PEXEncodeSurfaceApprox (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexSurfaceApprox approx;
    
    approx.approxMethod = ocSrc->data.SetSurfaceApprox.method;
    approx.uTolerance = ocSrc->data.SetSurfaceApprox.utolerance;
    approx.vTolerance = ocSrc->data.SetSurfaceApprox.vtolerance;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCSurfaceApprox,
        sizeof (pexSurfaceApprox), &approx);
}


void _PEXEncodeCullMode (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCullMode mode = ocSrc->data.SetFacetCullingMode.mode;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCFacetCullingMode,
        sizeof (pexCullMode), &mode);
}


void _PEXEncodeSwitch (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexSwitch flag = ocSrc->data.SetFacetDistinguishFlag.flag;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type, sizeof (pexSwitch), &flag);
}


void _PEXEncodePatternSize (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexVector2D size;
    
    size.x = ocSrc->data.SetPatternSize.width;
    size.y = ocSrc->data.SetPatternSize.height;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCPatternSize, sizeof (pexVector2D), &size);
}


void _PEXEncodePatternAttr2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCoord2D coord;
    
    coord.x = ocSrc->data.SetPatternAttributes2D.ref_point.x;
    coord.y = ocSrc->data.SetPatternAttributes2D.ref_point.y;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCPatternAttributes2D,
	sizeof (pexCoord2D), &coord);
}


void _PEXEncodePatternAttr (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCoord3D attr[3];
    
    attr[0].x = ocSrc->data.SetPatternAttributes.ref_point.x;
    attr[0].y = ocSrc->data.SetPatternAttributes.ref_point.y;
    attr[0].z = ocSrc->data.SetPatternAttributes.ref_point.z;
    attr[1].x = ocSrc->data.SetPatternAttributes.vector1.x;
    attr[1].y = ocSrc->data.SetPatternAttributes.vector1.y;
    attr[1].z = ocSrc->data.SetPatternAttributes.vector1.z;
    attr[2].x = ocSrc->data.SetPatternAttributes.vector2.x;
    attr[2].y = ocSrc->data.SetPatternAttributes.vector2.y;
    attr[2].z = ocSrc->data.SetPatternAttributes.vector2.z;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCPatternAttributes, sizeof (attr), attr);
}


void _PEXEncodeASF (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXASFData asf;
    
    asf.attribute = ocSrc->data.SetIndividualASF.attribute;
    asf.value = ocSrc->data.SetIndividualASF.asf;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCIndividualASF, sizeof (PEXASFData), &asf);
}


void _PEXEncodeLocalTransform (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexLocalTransform *pInfo;
    
    PEXInitEncodeOC (*ocDest, PEXOCLocalTransform,
        LENOF (pexLocalTransform), 0, pexLocalTransform, pInfo);
    
    pInfo->compType = ocSrc->data.SetLocalTransform.composition;
    COPY_AREA (ocSrc->data.SetLocalTransform.transform,
	pInfo->matrix, sizeof (pexMatrix));
}


void _PEXEncodeLocalTransform2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexLocalTransform2D *pInfo;
    
    PEXInitEncodeOC (*ocDest, PEXOCLocalTransform2D,
        LENOF (pexLocalTransform2D), 0, pexLocalTransform2D, pInfo);
    
    pInfo->compType = ocSrc->data.SetLocalTransform2D.composition;
    COPY_AREA (ocSrc->data.SetLocalTransform2D.transform,
	pInfo->matrix3X3, sizeof (pexMatrix3X3));
}


void _PEXEncodeGlobalTransform (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, PEXOCGlobalTransform,
        sizeof (pexMatrix), ocSrc->data.SetGlobalTransform.transform);
}


void _PEXEncodeGlobalTransform2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, PEXOCGlobalTransform2D,
        sizeof (pexMatrix3X3), ocSrc->data.SetGlobalTransform2D.transform);
}


void _PEXEncodeModelClipVolume (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexModelClipVolume	*pInfo;
    int			lenofData;
    
    lenofData = NUMWORDS (sizeof (PEXHalfSpace) *
	ocSrc->data.SetModelClipVolume.count);
    
    PEXInitEncodeOC (*ocDest, PEXOCModelClipVolume,
	LENOF (pexModelClipVolume), lenofData, pexModelClipVolume, pInfo);
    
    pInfo->modelClipOperator = ocSrc->data.SetModelClipVolume.op;
    pInfo->numHalfSpaces = ocSrc->data.SetModelClipVolume.count;
    
    PEXEncodeWords ((char *) ocSrc->data.SetModelClipVolume.half_spaces,
	*ocDest, lenofData);
}


void _PEXEncodeModelClipVolume2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexModelClipVolume2D	*pInfo;
    int				lenofData;
    
    lenofData = NUMWORDS (sizeof (PEXHalfSpace2D) *
	ocSrc->data.SetModelClipVolume2D.count);
    
    PEXInitEncodeOC (*ocDest, PEXOCModelClipVolume2D,
	LENOF (pexModelClipVolume2D), lenofData, pexModelClipVolume2D, pInfo);
    
    pInfo->modelClipOperator = ocSrc->data.SetModelClipVolume2D.op;
    pInfo->numHalfSpaces = ocSrc->data.SetModelClipVolume2D.count;
    
    PEXEncodeWords ((char *) ocSrc->data.SetModelClipVolume2D.half_spaces,
	*ocDest, lenofData);
}


void _PEXEncodeRestoreModelClip (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, PEXOCRestoreModelClipVolume, 0, (char *) NULL);
}


void _PEXEncodeLightSourceState (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    int 		sizeofEnableList, sizeofDisableList;
    pexLightState	*pInfo;
    
    sizeofEnableList = sizeof (CARD16) *
	ocSrc->data.SetLightSourceState.enable_count;
    sizeofDisableList = sizeof (CARD16) *
	ocSrc->data.SetLightSourceState.disable_count;
    
    PEXInitEncodeOC (*ocDest, PEXOCLightSourceState,
	LENOF (pexLightState),
	NUMWORDS (sizeofEnableList) + NUMWORDS (sizeofDisableList),
	pexLightState, pInfo);
    
    pInfo->numEnable = ocSrc->data.SetLightSourceState.enable_count;
    pInfo->numDisable =	ocSrc->data.SetLightSourceState.disable_count;
    
    PEXEncodeBytes ((char *) ocSrc->data.SetLightSourceState.enable,
	*ocDest, sizeofEnableList);

    PEXEncodeBytes ((char *) ocSrc->data.SetLightSourceState.disable,
	*ocDest, sizeofDisableList);
}


void _PEXEncodeID (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    CARD32 id = ocSrc->data.SetPickID.pick_id;
    
    PEXEncodeSimpleOC (*ocDest, ocSrc->oc_type, sizeof (CARD32), &id);
}


void _PEXEncodePSC (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexParaSurfCharacteristics 	*pInfo;
    PEXPSCData			*pscData;
    int				pscType;
    int				lenofData = 0;
    
    pscType = ocSrc->data.SetParaSurfCharacteristics.psc_type;
    pscData = &(ocSrc->data.SetParaSurfCharacteristics.characteristics);
    
    switch (pscType)
    {
    case PEXPSCIsoCurves:
	lenofData = LENOF (PEXPSCIsoparametricCurves);
	break;
	
    case PEXPSCMCLevelCurves:
    case PEXPSCWCLevelCurves:
	lenofData = NUMWORDS (sizeof (pexPSC_LevelCurves) +
	    sizeof (float) * pscData->level_curves.count);
	break;
	
    default:
	break;
    }
    
    PEXInitEncodeOC (*ocDest, PEXOCParaSurfCharacteristics,
	LENOF (pexParaSurfCharacteristics), lenofData,
	pexParaSurfCharacteristics, pInfo);
    
    pInfo->characteristics = pscType;
    pInfo->length = NUMBYTES (lenofData);
    
    if (lenofData > 0)
    {
	if (pscType == PEXPSCIsoCurves)
	{
	    PEXEncodeWords ((char *) pscData, *ocDest, lenofData);
	}
	else if (pscType == PEXPSCMCLevelCurves ||
	    pscType == PEXPSCWCLevelCurves)
	{
	    PEXEncodeBytes ((char *) pscData, *ocDest,
		sizeof (pexPSC_LevelCurves));
	    
	    PEXEncodeBytes ((char *) pscData->level_curves.parameters,
		*ocDest, pscData->level_curves.count * sizeof (float));
	}
    }
}


void _PEXEncodeNameSet (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, ocSrc->oc_type, False,
	ocSrc->data.AddToNameSet.count, sizeof (PEXName),
	(char *) ocSrc->data.AddToNameSet.names);
}


void _PEXEncodeExecuteStructure (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, PEXOCExecuteStructure,
	sizeof (PEXStructure), &(ocSrc->data.ExecuteStructure.structure));
}


void _PEXEncodeLabel (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    INT32 label = ocSrc->data.Label.label;
    
    PEXEncodeSimpleOC (*ocDest, PEXOCLabel, sizeof (INT32), &label);
}


void _PEXEncodeApplicationData (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, PEXOCApplicationData, True,
	ocSrc->data.ApplicationData.length, sizeof (char),
	(char *) ocSrc->data.ApplicationData.data);
}


void _PEXEncodeGSE (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexGse *pInfo;
    
    PEXInitEncodeOC (*ocDest, PEXOCGSE, LENOF (pexGse),
	NUMWORDS (ocSrc->data.GSE.length), pexGse, pInfo);
    
    pInfo->id = ocSrc->data.GSE.id;
    pInfo->numElements = ocSrc->data.GSE.length;
    
    PEXEncodeBytes ((char *) ocSrc->data.GSE.data,
        *ocDest, ocSrc->data.GSE.length);
}


void _PEXEncodeMarkers (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, PEXOCMarkers, False,
	ocSrc->data.Markers.count, sizeof (pexCoord3D),
	(char *) ocSrc->data.Markers.points);
}


void _PEXEncodePolyline (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, PEXOCPolyline, False,
	ocSrc->data.Polyline.count, sizeof (pexCoord3D),
	(char *) ocSrc->data.Polyline.points);
}


void _PEXEncodeMarkers2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, PEXOCMarkers2D, False,
	ocSrc->data.Markers2D.count, sizeof (pexCoord2D),
	(char *) ocSrc->data.Markers2D.points);
}


void _PEXEncodePolyline2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeListOC (*ocDest, PEXOCPolyline2D, False,
	ocSrc->data.Polyline2D.count, sizeof (pexCoord2D),
	(char *) ocSrc->data.Polyline2D.points);
}


void _PEXEncodeText (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    /* Text is always mono encoded */
    
    pexText		*pInfo;
    PEXEncodedTextData  *nextString;
    int 		lenofStrings, i;
    
    GetStringsLength (ocSrc->data.EncodedText.count,
	ocSrc->data.EncodedText.encoded_text, lenofStrings);
    
    PEXInitEncodeOC (*ocDest, PEXOCText,
	LENOF (pexText), lenofStrings, pexText, pInfo);
    
    pInfo->origin.x = ocSrc->data.EncodedText.origin.x;
    pInfo->origin.y = ocSrc->data.EncodedText.origin.y;
    pInfo->origin.z = ocSrc->data.EncodedText.origin.z;
    pInfo->vector1.x = ocSrc->data.EncodedText.vector1.x;
    pInfo->vector1.y = ocSrc->data.EncodedText.vector1.y;
    pInfo->vector1.z = ocSrc->data.EncodedText.vector1.z;
    pInfo->vector2.x = ocSrc->data.EncodedText.vector2.x;
    pInfo->vector2.y = ocSrc->data.EncodedText.vector2.y;
    pInfo->vector2.z = ocSrc->data.EncodedText.vector2.z;
    pInfo->numEncodings = (CARD16) ocSrc->data.EncodedText.count;
    
    EncodeMonoStrings (*ocDest, ocSrc->data.EncodedText.count,
	ocSrc->data.EncodedText.encoded_text);
}


void _PEXEncodeText2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    /* Text is always mono encoded */
    
    pexText2D		*pInfo;
    PEXEncodedTextData  *nextString;
    int 		lenofStrings, i;
    
    GetStringsLength (ocSrc->data.EncodedText2D.count,
	ocSrc->data.EncodedText2D.encoded_text, lenofStrings);
    
    PEXInitEncodeOC (*ocDest, PEXOCText2D,
	LENOF (pexText2D), lenofStrings, pexText2D, pInfo);
    
    pInfo->origin.x = ocSrc->data.EncodedText2D.origin.x;
    pInfo->origin.y = ocSrc->data.EncodedText2D.origin.y;
    pInfo->numEncodings = (CARD16) ocSrc->data.EncodedText2D.count;
    
    EncodeMonoStrings (*ocDest, ocSrc->data.EncodedText2D.count,
	ocSrc->data.EncodedText2D.encoded_text);
}


void _PEXEncodeAnnoText (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    /* Anno Text is always mono encoded */
    
    pexAnnotationText	*pInfo;
    PEXEncodedTextData  *nextString;
    int 		lenofStrings, i;
    
    GetStringsLength (ocSrc->data.EncodedAnnoText.count,
	ocSrc->data.EncodedAnnoText.encoded_text, lenofStrings);
    
    PEXInitEncodeOC (*ocDest, PEXOCAnnotationText,
	LENOF (pexAnnotationText), lenofStrings, pexAnnotationText, pInfo);
    
    pInfo->origin.x = ocSrc->data.EncodedAnnoText.origin.x;
    pInfo->origin.y = ocSrc->data.EncodedAnnoText.origin.y;
    pInfo->origin.z = ocSrc->data.EncodedAnnoText.origin.z;
    pInfo->offset.x = ocSrc->data.EncodedAnnoText.offset.x;
    pInfo->offset.y = ocSrc->data.EncodedAnnoText.offset.y;
    pInfo->offset.z = ocSrc->data.EncodedAnnoText.offset.z;
    pInfo->numEncodings = (CARD16) ocSrc->data.EncodedAnnoText.count;
    
    EncodeMonoStrings (*ocDest, ocSrc->data.EncodedAnnoText.count,
	ocSrc->data.EncodedAnnoText.encoded_text);
}


void _PEXEncodeAnnoText2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    /* Anno Text is always mono encoded */
    
    pexAnnotationText2D	*pInfo;
    PEXEncodedTextData	*nextString;
    int 		lenofStrings, i;
    
    GetStringsLength (ocSrc->data.EncodedAnnoText2D.count,
	ocSrc->data.EncodedAnnoText2D.encoded_text, lenofStrings);
    
    PEXInitEncodeOC (*ocDest, PEXOCAnnotationText2D,
	LENOF (pexAnnotationText2D), lenofStrings, pexAnnotationText2D, pInfo);
    
    pInfo->origin.x = ocSrc->data.EncodedAnnoText2D.origin.x;
    pInfo->origin.y = ocSrc->data.EncodedAnnoText2D.origin.y;
    pInfo->offset.x = ocSrc->data.EncodedAnnoText2D.offset.x;
    pInfo->offset.y = ocSrc->data.EncodedAnnoText2D.offset.y;
    pInfo->numEncodings = (CARD16) ocSrc->data.EncodedAnnoText2D.count;
    
    EncodeMonoStrings (*ocDest, ocSrc->data.EncodedAnnoText2D.count,
	ocSrc->data.EncodedAnnoText2D.encoded_text);
}


void _PEXEncodePolylineSet (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexPolylineSet	*pInfo;
    CARD32 		*pData;
    int			numPoints, i;
    int			lenofVertex;
    unsigned int	vertexAttributes;
    unsigned int	numPolylines;
    int			colorType;
    PEXListOfVertex	*polylines;
    
    numPolylines = ocSrc->data.PolylineSetWithData.count;
    polylines = ocSrc->data.PolylineSetWithData.vertex_lists;
    colorType = ocSrc->data.PolylineSetWithData.color_type;
    vertexAttributes = ocSrc->data.PolylineSetWithData.vertex_attributes;
    
    for (i = 0, numPoints = 0; i < numPolylines; i++)
	numPoints += polylines[i].count;
    
    lenofVertex = LENOF (pexCoord3D) + ((vertexAttributes & PEXGAColor) ?
	GetColorLength (colorType) : 0); 
    
    PEXInitEncodeOC (*ocDest, PEXOCPolylineSetWithData,
	LENOF (pexPolylineSet),
	numPolylines + (numPoints * lenofVertex),
	pexPolylineSet, pInfo);
    
    pInfo->colorType = colorType;
    pInfo->vertexAttribs = vertexAttributes;
    pInfo->numLists = numPolylines;
    
    for (i = 0; i < numPolylines; i++)
    {
	pData = (CARD32 *) *ocDest;
	*ocDest += sizeof (CARD32);
	*pData = polylines[i].count;  
	
	PEXEncodeWords ((char *) polylines[i].vertices.no_data,
	    *ocDest, polylines[i].count * lenofVertex);
    }
}


void _PEXEncodeNURBCurve (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexNurbCurve 	*pInfo;
    int			lenofVertexList;
    int			lenofKnotList;
    unsigned int	numPoints;
    int			rationality, order;
    
    numPoints = ocSrc->data.NURBCurve.count;
    rationality = ocSrc->data.NURBCurve.rationality;
    order = ocSrc->data.NURBCurve.order;
    
    lenofVertexList = numPoints * ((rationality == PEXRational) ?
	LENOF (pexCoord4D) : LENOF (pexCoord3D));
    lenofKnotList = order + numPoints;
    
    PEXInitEncodeOC (*ocDest, PEXOCNURBCurve,
	LENOF (pexNurbCurve), lenofKnotList + lenofVertexList,
	pexNurbCurve, pInfo);
    
    pInfo->curveOrder = order;
    pInfo->coordType = rationality;
    pInfo->tmin = ocSrc->data.NURBCurve.tmin;
    pInfo->tmax = ocSrc->data.NURBCurve.tmax;
    pInfo->numKnots = order + numPoints;
    pInfo->numPoints = numPoints;
    
    PEXEncodeWords ((char *) ocSrc->data.NURBCurve.knots,
	*ocDest, lenofKnotList);

    PEXEncodeWords ((char *) ocSrc->data.NURBCurve.points.point,
	*ocDest, lenofVertexList);
}


void _PEXEncodeFillArea (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexFillArea	*pInfo;
    int		lenofVertexList;
    
    lenofVertexList = ocSrc->data.FillArea.count * LENOF (pexCoord3D);
    
    PEXInitEncodeOC (*ocDest, PEXOCFillArea,
	LENOF (pexFillArea), lenofVertexList, pexFillArea, pInfo);
    
    pInfo->shape = ocSrc->data.FillArea.shape_hint;
    pInfo->ignoreEdges = ocSrc->data.FillArea.ignore_edges;
    
    PEXEncodeWords ((char *) ocSrc->data.FillArea.points,
	*ocDest, lenofVertexList);
}


void _PEXEncodeFillArea2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexFillArea2D	*pInfo;
    int			lenofVertexList;
    
    lenofVertexList = ocSrc->data.FillArea2D.count * LENOF (pexCoord2D);
    
    PEXInitEncodeOC (*ocDest, PEXOCFillArea2D,
	LENOF (pexFillArea2D), lenofVertexList, pexFillArea2D, pInfo);
    
    pInfo->shape = ocSrc->data.FillArea2D.shape_hint;
    pInfo->ignoreEdges = ocSrc->data.FillArea2D.ignore_edges;
    
    PEXEncodeWords ((char *) ocSrc->data.FillArea2D.points,
	*ocDest, lenofVertexList);
}


void _PEXEncodeFillAreaWithData (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexExtFillArea 	*pInfo;
    CARD32		*pData;
    int			lenofFacetData;
    int			lenofVertex;
    int			lenofColor;
    unsigned int	facetAttributes, vertexAttributes;
    int			colorType;
    unsigned int	numVertices;
    
    colorType = ocSrc->data.FillAreaWithData.color_type;
    facetAttributes = ocSrc->data.FillAreaWithData.facet_attributes;
    vertexAttributes = ocSrc->data.FillAreaWithData.vertex_attributes;
    numVertices = ocSrc->data.FillAreaWithData.count;
    
    lenofColor = GetColorLength (colorType);
    lenofFacetData = GetFacetDataLength (facetAttributes, lenofColor); 
    lenofVertex = GetVertexWithDataLength (vertexAttributes, lenofColor);
    
    PEXInitEncodeOC (*ocDest, PEXOCFillAreaWithData,
	LENOF (pexExtFillArea),
	lenofFacetData + 1 /* count */ + numVertices * lenofVertex,
	pexExtFillArea, pInfo);
    
    pInfo->shape = ocSrc->data.FillAreaWithData.shape_hint;
    pInfo->ignoreEdges = ocSrc->data.FillAreaWithData.ignore_edges;
    pInfo->colorType = colorType;
    pInfo->facetAttribs = facetAttributes;
    pInfo->vertexAttribs = vertexAttributes;
    
    if (facetAttributes)
    {
	PEXEncodeWords ((char *) &(ocSrc->data.FillAreaWithData.facet_data),
	    *ocDest, lenofFacetData);
    }
    
    pData = (CARD32 *) *ocDest;
    *ocDest += sizeof (CARD32);
    *pData = numVertices;
    
    PEXEncodeWords ((char *) ocSrc->data.FillAreaWithData.vertices.no_data,
	*ocDest, lenofVertex * numVertices);
}


void _PEXEncodeFillAreaSet (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexFillAreaSet	*pInfo;
    CARD32		*pData;
    int			numPoints, i;
    unsigned int	numFillAreas;
    PEXListOfCoord	*vertices;
    
    numFillAreas = ocSrc->data.FillAreaSet.count;
    vertices = ocSrc->data.FillAreaSet.point_lists;
    
    for (i = 0, numPoints = 0; i < numFillAreas; i++)
	numPoints += vertices[i].count;
    
    PEXInitEncodeOC (*ocDest, PEXOCFillAreaSet,
	LENOF (pexFillAreaSet),
	numFillAreas + (numPoints * LENOF (pexCoord3D)),
	pexFillAreaSet, pInfo);
    
    pInfo->shape = ocSrc->data.FillAreaSet.shape_hint; 
    pInfo->ignoreEdges = ocSrc->data.FillAreaSet.ignore_edges;
    pInfo->contourHint = ocSrc->data.FillAreaSet.contour_hint;
    pInfo->numLists = numFillAreas;
    
    for (i = 0; i < numFillAreas; i++)
    {
	pData = (CARD32 *) *ocDest;
	*ocDest += sizeof (CARD32);
	*pData = vertices[i].count;
	
	PEXEncodeWords ((char *) vertices[i].points, *ocDest,
	    vertices[i].count * LENOF (pexCoord3D));
    }
}


void _PEXEncodeFillAreaSet2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexFillAreaSet2D	*pInfo;
    CARD32		*pData;
    int			numPoints, i;
    unsigned int	numFillAreas;
    PEXListOfCoord2D	*vertices;
    
    numFillAreas = ocSrc->data.FillAreaSet2D.count;
    vertices = ocSrc->data.FillAreaSet2D.point_lists;
    
    for (i = 0, numPoints = 0; i < numFillAreas; i++)
	numPoints += vertices[i].count;
    
    PEXInitEncodeOC (*ocDest, PEXOCFillAreaSet2D,
	LENOF (pexFillAreaSet2D),
	numFillAreas /* counts */ + (numPoints * LENOF (pexCoord2D)),
	pexFillAreaSet2D, pInfo);
    
    pInfo->shape = ocSrc->data.FillAreaSet2D.shape_hint; 
    pInfo->ignoreEdges = ocSrc->data.FillAreaSet2D.ignore_edges;
    pInfo->contourHint = ocSrc->data.FillAreaSet2D.contour_hint;
    pInfo->numLists = numFillAreas;
    
    for (i = 0; i < numFillAreas; i++)
    {
	pData = (CARD32 *) *ocDest;
	*ocDest += sizeof (CARD32);
	*pData = vertices[i].count;
	
	PEXEncodeWords ((char *) vertices[i].points, *ocDest,
	    vertices[i].count * LENOF (pexCoord2D));
    }
}


void _PEXEncodeFillAreaSetWithData (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexExtFillAreaSet	*pInfo;
    CARD32		*pData;
    int			lenofColor;
    int			lenofFacetData;
    int			lenofVertex;
    int			numVertices, i;
    int			colorType;
    unsigned int	numFillAreas;
    unsigned int	facetAttributes, vertexAttributes;
    PEXListOfVertex	*vertices;
    
    colorType = ocSrc->data.FillAreaSetWithData.color_type;
    numFillAreas = ocSrc->data.FillAreaSetWithData.count;
    facetAttributes = ocSrc->data.FillAreaSetWithData.facet_attributes;
    vertexAttributes = ocSrc->data.FillAreaSetWithData.vertex_attributes;
    vertices = ocSrc->data.FillAreaSetWithData.vertex_lists;
    
    lenofColor = GetColorLength (colorType);
    lenofFacetData = GetFacetDataLength (facetAttributes, lenofColor); 
    lenofVertex = GetVertexWithDataLength (vertexAttributes, lenofColor);
    
    if (vertexAttributes & PEXGAEdges)
	lenofVertex++; 			/* edge switch is CARD32 */
    
    for (i = 0, numVertices = 0; i < numFillAreas; i++)
	numVertices += vertices[i].count;
    
    PEXInitEncodeOC (*ocDest, PEXOCFillAreaSetWithData, 
	LENOF (pexExtFillAreaSet), 
	lenofFacetData + numFillAreas + numVertices * lenofVertex,
	pexExtFillAreaSet, pInfo);
    
    pInfo->shape = ocSrc->data.FillAreaSetWithData.shape_hint;
    pInfo->ignoreEdges = ocSrc->data.FillAreaSetWithData.ignore_edges;
    pInfo->contourHint = ocSrc->data.FillAreaSetWithData.contour_hint;
    pInfo->colorType = colorType;
    pInfo->facetAttribs = facetAttributes;
    pInfo->vertexAttribs = vertexAttributes;
    pInfo->numLists = numFillAreas;
    
    if (facetAttributes)
    {
	PEXEncodeWords (
	    (char *) &(ocSrc->data.FillAreaSetWithData.facet_data),
	    *ocDest, lenofFacetData);
    }
    
    for (i = 0; i < numFillAreas; i++)
    {
	pData = (CARD32 *) *ocDest;
	*ocDest += sizeof (CARD32);
	*pData = vertices[i].count; 
	
	PEXEncodeWords ((char *) vertices[i].vertices.no_data,
	    *ocDest, vertices[i].count * lenofVertex);
    }
}


void _PEXEncodeTriangleStrip (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexTriangleStrip	*pInfo;
    int			lenofColor;
    int			lenofFacetDataList;
    int			lenofVertexList;
    int			colorType;
    unsigned long	numVertices;
    unsigned int	facetAttributes, vertexAttributes;
    
    colorType = ocSrc->data.TriangleStrip.color_type;
    numVertices = ocSrc->data.TriangleStrip.count;
    facetAttributes = ocSrc->data.TriangleStrip.facet_attributes;
    vertexAttributes = ocSrc->data.TriangleStrip.vertex_attributes;
    
    lenofColor = GetColorLength (colorType);
    lenofFacetDataList = (numVertices - 2) *
	GetFacetDataLength (facetAttributes, lenofColor); 
    lenofVertexList = numVertices *
	GetVertexWithDataLength (vertexAttributes, lenofColor);
    
    PEXInitEncodeOC (*ocDest, PEXOCTriangleStrip, 
	LENOF (pexTriangleStrip),
	lenofFacetDataList + lenofVertexList,
	pexTriangleStrip, pInfo);
    
    pInfo->colorType = colorType;
    pInfo->facetAttribs = facetAttributes;
    pInfo->vertexAttribs = vertexAttributes;
    pInfo->numVertices = numVertices;
    
    if (facetAttributes)
    {
	PEXEncodeWords (
	    (char *) ocSrc->data.TriangleStrip.facet_data.index,
	    *ocDest, lenofFacetDataList);
    }
    
    PEXEncodeWords ((char *) ocSrc->data.TriangleStrip.vertices.no_data,
	*ocDest, lenofVertexList);
}


void _PEXEncodeQuadMesh (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexQuadrilateralMesh 	*pInfo;
    int				lenofColor;
    int				lenofFacetDataList;
    int				lenofVertexList;
    int				colorType;
    unsigned int		rowCount, colCount;
    unsigned int		facetAttributes, vertexAttributes;
    
    colorType = ocSrc->data.QuadrilateralMesh.color_type;
    rowCount = ocSrc->data.QuadrilateralMesh.row_count;
    colCount = ocSrc->data.QuadrilateralMesh.col_count;
    facetAttributes = ocSrc->data.QuadrilateralMesh.facet_attributes;
    vertexAttributes = ocSrc->data.QuadrilateralMesh.vertex_attributes;
    
    lenofColor = GetColorLength (colorType);
    lenofFacetDataList = ((rowCount - 1) * (colCount - 1)) *
	GetFacetDataLength (facetAttributes, lenofColor); 
    lenofVertexList = rowCount * colCount *
	GetVertexWithDataLength (vertexAttributes, lenofColor);
    
    PEXInitEncodeOC (*ocDest, PEXOCQuadrilateralMesh, 
	LENOF (pexQuadrilateralMesh),
	lenofFacetDataList + lenofVertexList,
	pexQuadrilateralMesh, pInfo);
    
    pInfo->colorType = colorType;
    pInfo->mPts = colCount;
    pInfo->nPts = rowCount;
    pInfo->facetAttribs = facetAttributes;
    pInfo->vertexAttribs = vertexAttributes;
    pInfo->shape = ocSrc->data.QuadrilateralMesh.shape_hint;
    
    if (facetAttributes)
    {
	PEXEncodeWords (
	    (char *) ocSrc->data.QuadrilateralMesh.facet_data.index,
	    *ocDest, lenofFacetDataList);
    }
    
    PEXEncodeWords (
	(char *) ocSrc->data.QuadrilateralMesh.vertices.no_data,
	*ocDest, lenofVertexList);
}


void _PEXEncodeSOFA (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexSOFAS		*pInfo;
    PEXConnectivityData *pConnectivity;
    PEXListOfUShort 	*pList;
    int 		lenofColor;
    int 		lenofFacet;
    int 		lenofVertex;
    int 		sizeofEdge;
    int			totLength;
    int 		numContours;
    int 		count = 0;
    int			i, j;
    CARD16		*pData;
    unsigned int	facetAttributes;
    unsigned int	vertexAttributes;
    unsigned int	edgeAttributes;
    int			colorType, cbytes;
    unsigned int	numFillAreaSets;
    unsigned int	numVertices;
    unsigned int	numIndices;
    
    colorType = ocSrc->data.SetOfFillAreaSets.color_type;
    facetAttributes = ocSrc->data.SetOfFillAreaSets.facet_attributes;
    vertexAttributes = ocSrc->data.SetOfFillAreaSets.vertex_attributes;
    edgeAttributes = ocSrc->data.SetOfFillAreaSets.edge_attributes;
    numFillAreaSets = ocSrc->data.SetOfFillAreaSets.set_count;
    numVertices = ocSrc->data.SetOfFillAreaSets.vertex_count;
    numIndices = ocSrc->data.SetOfFillAreaSets.index_count;
    
    numContours = 0;
    pConnectivity = ocSrc->data.SetOfFillAreaSets.connectivity;
    for (i = 0; i < numFillAreaSets; i++, pConnectivity++)
	numContours += pConnectivity->count;
    
    lenofColor = GetColorLength (colorType);
    lenofFacet = GetFacetDataLength (facetAttributes, lenofColor); 
    lenofVertex = GetVertexWithDataLength (vertexAttributes, lenofColor);
    sizeofEdge = edgeAttributes ? sizeof (CARD8) : 0;
    
    cbytes = sizeof (CARD16) * (numFillAreaSets + numContours + numIndices);

    totLength = (lenofFacet * numFillAreaSets) + (lenofVertex * numVertices) + 
	NUMWORDS (sizeofEdge * numIndices) + NUMWORDS (cbytes);
    
    PEXInitEncodeOC (*ocDest, PEXOCSetOfFillAreaSets,
	LENOF (pexSOFAS), totLength, pexSOFAS, pInfo);
    
    pInfo->shape = ocSrc->data.SetOfFillAreaSets.shape_hint;
    pInfo->colorType = colorType;
    pInfo->FAS_Attributes = facetAttributes;
    pInfo->vertexAttributes = vertexAttributes;
    pInfo->edgeAttributes = edgeAttributes ? PEXOn : PEXOff;
    pInfo->contourHint = ocSrc->data.SetOfFillAreaSets.contour_hint;
    pInfo->contourCountsFlag = ocSrc->data.SetOfFillAreaSets.contours_all_one;
    pInfo->numFAS = numFillAreaSets;
    pInfo->numVertices = numVertices;
    pInfo->numEdges = numIndices;
    pInfo->numContours = numContours;
    
    if (facetAttributes)
    {
	PEXEncodeWords (
	    (char *) ocSrc->data.SetOfFillAreaSets.facet_data.index,
	    *ocDest, lenofFacet * numFillAreaSets);
    }
    
    PEXEncodeWords ((char *) ocSrc->data.SetOfFillAreaSets.vertices.no_data,
	*ocDest, lenofVertex * numVertices);
    
    if (edgeAttributes)
    {
	PEXEncodeBytes ((char *) ocSrc->data.SetOfFillAreaSets.edge_flags,
	    *ocDest, numIndices * sizeof (CARD8));
    }
    
    pConnectivity = ocSrc->data.SetOfFillAreaSets.connectivity;
    
    for (i = 0 ; i < numFillAreaSets; i++)
    {
	pData = (CARD16 *) *ocDest;
	*ocDest += sizeof (CARD16);
	*pData = count = pConnectivity->count;
	
	for (j = 0, pList = pConnectivity->lists; j < count; j++, pList++)
	{
	    pData = (CARD16 *) *ocDest;
	    *ocDest += sizeof (CARD16);
	    *pData = pList->count;
	    
	    COPY_AREA ((char *) pList->shorts,
	        *ocDest, pList->count * sizeof (CARD16));
	    *ocDest += (pList->count * sizeof (CARD16));
	}
	
	pConnectivity++;
    }

    *ocDest += PAD (cbytes);
}


void _PEXEncodeNURBSurface (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexNurbSurface	*pInfo;
    pexTrimCurve	*pTCHead;
    PEXTrimCurve	*ptrimCurve;
    PEXListOfTrimCurve	*ptrimLoop;
    CARD32		*pData;
    int			lenofVertexList;
    int			lenofUKnotList;
    int			lenofVKnotList;
    int			lenofTrimData;
    int			thisLength, i;
    int			count = 0;
    unsigned int	numMPoints, numNPoints; 
    int			rationality, uorder, vorder;
    unsigned long	numTrimLoops;
    
    numMPoints = ocSrc->data.NURBSurface.col_count;
    numNPoints = ocSrc->data.NURBSurface.row_count;
    rationality = ocSrc->data.NURBSurface.rationality;
    uorder = ocSrc->data.NURBSurface.uorder;
    vorder = ocSrc->data.NURBSurface.vorder;
    numTrimLoops = ocSrc->data.NURBSurface.curve_count;
    
    lenofVertexList = numMPoints * numNPoints *
	((rationality == PEXRational) ?
	LENOF (pexCoord4D) : LENOF (pexCoord3D));
    lenofUKnotList = uorder + numMPoints;
    lenofVKnotList = vorder + numNPoints;
    
    lenofTrimData = numTrimLoops * LENOF (CARD32);
    
    ptrimLoop = ocSrc->data.NURBSurface.trim_curves;
    for (i = 0; i < numTrimLoops; i++, ptrimLoop++)
    {
	ptrimCurve = ptrimLoop->curves;
	count = ptrimLoop->count;
	
	while (count--)
	{
	    lenofTrimData += (LENOF (pexTrimCurve) +
		ptrimCurve->count +
		ptrimCurve->order +     /* knot list */
		ptrimCurve->count *
		(ptrimCurve->rationality == PEXRational ?
		LENOF (pexCoord3D) : LENOF (pexCoord2D)));
	    ptrimCurve++;
	}
    }
    
    PEXInitEncodeOC (*ocDest, PEXOCNURBSurface,
	LENOF (pexNurbSurface),
	(lenofUKnotList + lenofVKnotList + lenofVertexList + lenofTrimData), 
	pexNurbSurface, pInfo);
    
    pInfo->type = rationality;
    pInfo->uOrder = uorder;
    pInfo->vOrder = vorder;
    pInfo->numUknots = uorder + numMPoints;
    pInfo->numVknots = vorder + numNPoints;
    pInfo->mPts = numMPoints;
    pInfo->nPts = numNPoints;
    pInfo->numLists = numTrimLoops;
    
    PEXEncodeWords ((char *) ocSrc->data.NURBSurface.uknots,
	*ocDest, lenofUKnotList);

    PEXEncodeWords ((char *) ocSrc->data.NURBSurface.vknots,
	*ocDest, lenofVKnotList);

    PEXEncodeWords ((char *) ocSrc->data.NURBSurface.points.point,
	*ocDest, lenofVertexList);
    
    ptrimLoop = ocSrc->data.NURBSurface.trim_curves;
    for (i = 0; i < numTrimLoops; i++, ptrimLoop++)
    {
	pData = (CARD32 *) *ocDest;
	*ocDest += sizeof (CARD32);
	*pData = count = ptrimLoop->count;
	
	ptrimCurve = ptrimLoop->curves;
	
	while (count--)
	{
	    thisLength = ptrimCurve->order + ptrimCurve->count;
	    
	    pTCHead = (pexTrimCurve *) *ocDest;
	    *ocDest += sizeof (pexTrimCurve);
	    
	    pTCHead->visibility = (pexSwitch) ptrimCurve->visibility;
	    pTCHead->order = (CARD16) ptrimCurve->order;
	    pTCHead->type = (pexCoordType) ptrimCurve->rationality;
	    pTCHead->approxMethod = (INT16) ptrimCurve->approx_method;
	    pTCHead->tolerance = (float) ptrimCurve->tolerance;
	    pTCHead->tMin = (float) ptrimCurve->tmin;
	    pTCHead->tMax = (float) ptrimCurve->tmax;
	    pTCHead->numKnots = thisLength;
	    pTCHead->numCoord = ptrimCurve->count;
	
	    PEXEncodeWords ((char *) ptrimCurve->knots.floats,
	        *ocDest, thisLength);
	
	    thisLength = ptrimCurve->count *
	        ((ptrimCurve->rationality == PEXRational) ?
	        LENOF (pexCoord3D) : LENOF (pexCoord2D));
	
	    PEXEncodeWords ((char *) ptrimCurve->control_points.point,
	        *ocDest, thisLength);
	
	    ptrimCurve++;
	}
    }
}


void _PEXEncodeCellArray (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCellArray	*pInfo;
    int			bytes;
    
    bytes = ocSrc->data.CellArray.col_count *
	ocSrc->data.CellArray.row_count * sizeof (pexTableIndex);
    
    PEXInitEncodeOC (*ocDest, PEXOCCellArray,
	LENOF (pexCellArray), NUMWORDS (bytes), pexCellArray, pInfo);
    
    pInfo->point1.x = ocSrc->data.CellArray.point1.x;
    pInfo->point1.y = ocSrc->data.CellArray.point1.y;
    pInfo->point1.z = ocSrc->data.CellArray.point1.z;
    pInfo->point2.x = ocSrc->data.CellArray.point2.x;
    pInfo->point2.y = ocSrc->data.CellArray.point2.y;
    pInfo->point2.z = ocSrc->data.CellArray.point2.z;
    pInfo->point3.x = ocSrc->data.CellArray.point3.x;
    pInfo->point3.y = ocSrc->data.CellArray.point3.y;
    pInfo->point3.z = ocSrc->data.CellArray.point3.z;
    pInfo->dx = ocSrc->data.CellArray.col_count;
    pInfo->dy = ocSrc->data.CellArray.row_count;
    
    PEXEncodeBytes ((char *) ocSrc->data.CellArray.color_indices,
	*ocDest, bytes);
}


void _PEXEncodeCellArray2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexCellArray2D	*pInfo;
    int			bytes;
    
    bytes = ocSrc->data.CellArray2D.col_count *
	ocSrc->data.CellArray2D.row_count * sizeof (pexTableIndex);
    
    PEXInitEncodeOC (*ocDest, PEXOCCellArray2D,
	LENOF (pexCellArray2D), NUMWORDS (bytes), pexCellArray2D, pInfo);
    
    pInfo->point1.x = ocSrc->data.CellArray2D.point1.x;
    pInfo->point1.y = ocSrc->data.CellArray2D.point1.y;
    pInfo->point2.x = ocSrc->data.CellArray2D.point2.x;
    pInfo->point2.y = ocSrc->data.CellArray2D.point2.y;
    pInfo->dx = ocSrc->data.CellArray2D.col_count;
    pInfo->dy = ocSrc->data.CellArray2D.row_count;
    
    PEXEncodeBytes ((char *) ocSrc->data.CellArray2D.color_indices,
	*ocDest, bytes);
}


void _PEXEncodeExtendedCellArray (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexExtCellArray	*pInfo;
    int			lenofColorList;
    
    lenofColorList = ocSrc->data.ExtendedCellArray.col_count *
	ocSrc->data.ExtendedCellArray.row_count *
	GetColorLength (ocSrc->data.ExtendedCellArray.color_type);
    
    PEXInitEncodeOC (*ocDest, PEXOCExtendedCellArray,
	LENOF (pexExtCellArray), lenofColorList, pexExtCellArray, pInfo);
    
    pInfo->colorType = ocSrc->data.ExtendedCellArray.color_type;
    pInfo->point1.x = ocSrc->data.ExtendedCellArray.point1.x;
    pInfo->point1.y = ocSrc->data.ExtendedCellArray.point1.y;
    pInfo->point1.z = ocSrc->data.ExtendedCellArray.point1.z;
    pInfo->point2.x = ocSrc->data.ExtendedCellArray.point2.x;
    pInfo->point2.y = ocSrc->data.ExtendedCellArray.point2.y;
    pInfo->point2.z = ocSrc->data.ExtendedCellArray.point2.z;
    pInfo->point3.x = ocSrc->data.ExtendedCellArray.point3.x;
    pInfo->point3.y = ocSrc->data.ExtendedCellArray.point3.y;
    pInfo->point3.z = ocSrc->data.ExtendedCellArray.point3.z;
    pInfo->dx = ocSrc->data.ExtendedCellArray.col_count;
    pInfo->dy = ocSrc->data.ExtendedCellArray.row_count;
    
    PEXEncodeWords ((char *) ocSrc->data.ExtendedCellArray.colors.indexed,
	*ocDest, lenofColorList);
}


void _PEXEncodeGDP (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexGdp	*pInfo;
    int		lenofVertexList;
    
    lenofVertexList = LENOF (pexCoord3D) * ocSrc->data.GDP.count;
    
    PEXInitEncodeOC (*ocDest, PEXOCGDP,
	LENOF (pexGdp), lenofVertexList + NUMWORDS (ocSrc->data.GDP.length),
	pexGdp, pInfo);
    
    pInfo->gdpId = ocSrc->data.GDP.gdp_id;
    pInfo->numPoints = ocSrc->data.GDP.count;
    pInfo->numBytes = ocSrc->data.GDP.length;
    
    PEXEncodeWords ((char *) ocSrc->data.GDP.points, *ocDest, lenofVertexList);

    PEXEncodeBytes ((char *) ocSrc->data.GDP.data,
	*ocDest, ocSrc->data.GDP.length);
}


void _PEXEncodeGDP2D (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    pexGdp2D	*pInfo;
    int		lenofVertexList;
    
    lenofVertexList = LENOF (pexCoord2D) *
	ocSrc->data.GDP2D.count;
    
    PEXInitEncodeOC (*ocDest, PEXOCGDP2D,
	LENOF (pexGdp2D), lenofVertexList + NUMWORDS (ocSrc->data.GDP2D.length),
	pexGdp2D, pInfo);
    
    pInfo->gdpId = ocSrc->data.GDP2D.gdp_id;
    pInfo->numPoints = ocSrc->data.GDP2D.count;
    pInfo->numBytes = ocSrc->data.GDP2D.length;
    
    PEXEncodeWords ((char *) ocSrc->data.GDP2D.points,
	*ocDest, lenofVertexList);

    PEXEncodeBytes ((char *) ocSrc->data.GDP2D.data,
	*ocDest, ocSrc->data.GDP2D.length);
}


void _PEXEncodeNoop (float_format, ocSrc, ocDest)

int		float_format;
PEXOCData	*ocSrc;
char		**ocDest;

{
    PEXEncodeSimpleOC (*ocDest, PEXOCNoop, 0, (char *) NULL);
}
