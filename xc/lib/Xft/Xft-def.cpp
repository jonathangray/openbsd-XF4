LIBRARY Xft
VERSION LIBRARY_VERSION
EXPORTS

#ifndef __UNIXOS2__
XftConfigAddDir
XftConfigAddEdit
XftConfigGetCache
XftConfigSetCache
XftConfigSubstitute
_XftConfigCompareValue
#endif
XftColorAllocName
XftColorAllocValue
XftColorFree
#ifndef __UNIXOS2__
XftCoreConvert16
XftCoreConvert32
XftCoreConvertUtf8
XftCoreExtents16
XftCoreExtents32
XftCoreExtents8
XftCoreExtentsUtf8
XftCoreGlyphExists
XftEditPrint
XftExprPrint
XftFontSetPrint
XftOpPrint
XftPatternPrint
XftSubstPrint
XftTestPrint
XftValueListPrint
XftValuePrint
#endif
XftDefaultGetBool
XftDefaultGetDouble
XftDefaultGetInteger
XftDefaultHasRender
XftDefaultParseBool
XftDefaultSet
XftDefaultSubstitute
#ifndef __UNIXOS2__
XftDisplayGetFontSet
#endif
XftDrawChange
#ifndef __UNIXOS2__
XftDrawCorePrepare
#endif
XftDrawCreate
XftDrawCreateBitmap
XftDrawDestroy
XftDrawRect
#ifndef __UNIXOS2__
XftDrawRenderPrepare
#endif
XftDrawSetClip
XftDrawString16
XftDrawString32
XftDrawString8
XftDrawStringUtf8
XftTextExtents16
XftTextExtents32
XftTextExtents8
XftTextExtentsUtf8
XftFontClose
XftFontMatch
XftFontOpen
XftFontOpenName
XftFontOpenPattern
XftFontOpenXlfd
#ifndef __UNIXOS2__
XftGlyphExists
_XftFontDebug
XftFontSetAdd
XftFontSetCreate
XftFontSetDestroy
XftConfigSaveField
XftConfigerror
XftConfigparse
XftConfigwrap
XftEditCreate
XftEditDestroy
XftExprCreateBool
XftExprCreateDouble
XftExprCreateField
XftExprCreateInteger
XftExprCreateNil
XftExprCreateOp
XftExprCreateString
XftExprDestroy
XftTestCreate
#endif
XftInit
#ifndef __UNIXOS2__
XftConfigLexFile
XftConfigPushInput
XftConfig_create_buffer
XftConfig_delete_buffer
XftConfig_flush_buffer
XftConfig_init_buffer
XftConfig_load_buffer_state
XftConfig_scan_buffer
XftConfig_scan_bytes
XftConfig_scan_string
XftConfig_switch_to_buffer
XftConfiglex
XftConfigrestart
XftListAppend
XftListFontSets
#endif
XftListFonts
XftListFontsPatternObjects
#ifndef __UNIXOS2__
XftListMatch
XftListValueCompare
XftListValueListCompare
XftObjectSetAdd
XftObjectSetBuild
XftObjectSetCreate
XftObjectSetDestroy
XftObjectSetVaBuild
XftFontSetMatch
XftNameConstant
#endif
XftNameParse
XftNameUnparse
#ifndef __UNIXOS2__
XftPatternAdd
XftPatternAddBool
XftPatternAddDouble
XftPatternAddInteger
XftPatternAddString
XftPatternBuild
XftPatternCreate
XftPatternDel
XftPatternDestroy
XftPatternDuplicate
XftPatternFind
XftPatternGet
XftPatternGetBool
XftPatternGetDouble
XftPatternGetInteger
XftPatternGetString
XftPatternVaBuild
XftValueDestroy
XftValueListDestroy
XftUtf8Len
XftUtf8ToUcs4
_XftDownStr
_XftGetInt
#endif
_XftMatchSymbolic
#ifndef __UNIXOS2__
_XftSaveString
_XftSplitField
_XftSplitStr
_XftSplitValue
_XftStrCmpIgnoreCase
XftCoreAddFonts
XftCoreClose
XftCoreOpen
#endif
XftXlfdParse
XftInitFtLibrary
#ifndef __UNIXOS2__
XftConfigDirs
XftDirScan
XftDirSave
#endif
XftDrawPicture
XftDrawSrcPicture
#ifndef __UNIXOS2__
XftGlyphLoad
XftGlyphCheck
XftFreeTypeGlyphExists
XftFreeTypeOpen
XftFreeTypeClose
XftRenderString16
#else
XftCharExists
#endif  /* __UNIXOS2__ */

/* $Id: Xft-def.cpp,v 1.3 2005/12/31 14:27:09 matthieu Exp $ */
