// project: dicm2deck
// file: decktypes.h
// created by jacquesfauquex on 2024-04-04.


#ifndef decktypes_h
#define decktypes_h

#pragma mark - C

#include <stdio.h>  //puts() printf()
#include <stdbool.h>
#include <stdlib.h> //malloc()
#include <string.h> //memcpy()
#include <ctype.h>
#include <unistd.h>
//#include <sys/time.h>
//#include <uuid/uuid.h>//for uuid_generate() and uuid_unparse()
#include <sys/stat.h>//for directory creation
#include <time.h>
//#include <sqlite3.h>
//#include <errno.h>

//openjpeg
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <stdint.h>


typedef   int8_t s8;//%c
typedef  uint8_t u8;//%c
typedef  int16_t s16;//%hd
typedef uint16_t u16;//%hu
typedef  int32_t s32;//%d
typedef uint32_t u32;//%u
typedef  int64_t s64;//%lld
typedef uint64_t u64;//%llu

u8 intdecsize(int i);

//DICOM atribute header 8 bytes tag vr vl
struct trcl {
   u32 t;
   u16 r;
   u16 c;
   u32 l;
};


/*potencialmente multivalue: AE AS AT CS DA DS DT FD FL IS LO OW PN SH SV TM UC UI UV*/
enum DICMvr {
   AE=0x4541,//application entity
   AS=0x5341,//age string
   AT=0x5441,//attribute tag
   CS=0x5343,//coded string
   DA=0x4144,//date
   DS=0x5344,//decimal string
   DT=0x5444,//date time
   FD=0x4446,//floating point double
   FL=0x4C46,//floating point single
   IS=0x5349,//integer string
   LO=0x4f4c,//long string
   LT=0x544c,//long text
   PN=0x4e50,//person name
   SH=0x4853,//short string
   SL=0x4C53,//signed long
   SS=0x5353,//signed short
   ST=0x5453,//short text
   TM=0x4d54,//time
   UI=0x4955,//unique ID
   UL=0x4C55,//unsigned long
   US=0x5355,//unsigned short
   OB=0x424F,//other byte
   OD=0x444F,//other double
   OF=0x464F,//other float
   OL=0x4C4F,//other long
   OV=0x564F,//other 64-bit very long
   OW=0x574F,//other word
   SV=0x5653,//signed 64-bit very long
   UC=0x4355,//unlimited characters
   UR=0x5255,//universal resrcurl identifier/locator
   UT=0x5455,//unlimited text
   UV=0x5655,//unsigned 64-bit very long
   UN=0x4E55,
   SQ=0x5153,
   SA=0x0000,
   IA=0x2B2B,//++
   IZ=0x5F5F,//__
   SZ=0xFFFF
};

#pragma mark - endianness

u64 u64swap(u64 x);
u32 u32swap(u32 x);
u16 u16swap(u16 x);


#pragma mark - uid shrink

extern char const  base64EncodingTable[65];
extern char const  base64DecodingTable[128];
bool ui2b64( char *ui, u8 uilength, char *b64, u8 *b64length );

#pragma mark - main & log

enum exitValue{
   dckvErrorIn=-2,
   dckvErrorOutPath=-3,
   dckvErrorWrite=-4,
   dckvErrorCreateKV=-5,
   dckvSOPinstanceRejected=-6,
   dckvErrorParsing=-7
};

//https://stackoverflow.com/questions/53522586/variadic-macro-calling-fprintf-how-to-add-arguments-to-va-args
//el if permite sumar los niveles más fundamentales
enum DIWEFenum {D,I,W,E,F};
extern enum DIWEFenum DIWEF;
bool loglevel(const char * logletter);

#define D(format, ...) do {                 \
  if (DIWEF <= D){                          \
    fprintf(stderr, (format), __VA_ARGS__); \
    fputc('\n', stderr);                    \
  }                                         \
} while (0)


#define I(format, ...) do {                 \
  if (DIWEF <= I){                          \
    fprintf(stderr, (format), __VA_ARGS__); \
    fputc('\n', stderr);                    \
  }                                         \
} while (0)


#define W(format, ...) do {                 \
  if (DIWEF <= W){                          \
    fprintf(stderr, (format), __VA_ARGS__); \
    fputc('\n', stderr);                    \
  }                                         \
} while (0)


#define E(format, ...) do {                 \
  if (DIWEF <= E){                          \
    fprintf(stderr, (format), __VA_ARGS__); \
    fputc('\n', stderr);                    \
  }                                         \
  false;                                    \
} while (0)


#define F(format, ...) do {                \
  fprintf(stderr, @"%@", __FAULT__);       \
} while (0)



#pragma mark - repertoires

enum repertoireenum{
   
   REPERTOIRE_GL,
   ISO_IR100,
   ISO_IR101,
   ISO_IR109,
   ISO_IR110,
   ISO_IR148,
   ISO_IR126,
   ISO_IR127,
   
   ISO_IR192,
   RFC3986,
   ISO_IR13,
   ISO_IR144,
   ISO_IR138,
   ISO_IR166,
   GB18030,
   GBK,
   
   ISO2022IR6,
   ISO2022IR100,
   ISO2022IR101,
   ISO2022IR109,
   ISO2022IR110,
   ISO2022IR148,
   ISO2022IR126,
   ISO2022IR127,
   
   ISO2022IR87,
   ISO2022IR159,
   ISO2022IR13,
   ISO2022IR144,
   ISO2022IR138,
   ISO2022IR166,
   ISO2022IR149,
   ISO2022IR58
};


u32 repertoireidx( char *vbuf, u16 vallength );

bool utf8
(
 u32         repidx,
 const char *charbytes,
 u32         charstart,
 size_t      charlength,
 
 char      *utf8bytes,
 u32        utf8start,
 u32       *utf8length
 );


#pragma mark - SopClasses

//sc_=1.2.840.10008
enum sc_uidenum{
sc_1_1=0,
sc_1_3_10,
sc_1_9,
sc_1_20_1,
sc_1_20_2,
sc_1_40,
sc_1_42,
sc_3_1_2_1_1,
sc_3_1_2_1_4,
sc_3_1_2_2_1,
sc_3_1_2_3_1,
sc_3_1_2_3_2,
sc_3_1_2_3_3,
sc_3_1_2_3_4,
sc_3_1_2_3_5,
sc_3_1_2_5_1,
sc_3_1_2_5_4,
sc_3_1_2_5_5,
sc_3_1_2_6_1,
sc_5_1_1_1,
sc_5_1_1_2,
sc_5_1_1_4,
sc_5_1_1_4_1,
sc_5_1_1_4_2,
sc_5_1_1_9,
sc_5_1_1_9_1,
sc_5_1_1_14,
sc_5_1_1_15,
sc_5_1_1_16,
sc_5_1_1_16_376,
sc_5_1_1_18,
sc_5_1_1_18_1,
sc_5_1_1_22,
sc_5_1_1_23,
sc_5_1_1_24,
sc_5_1_1_24_1,
sc_5_1_1_26,
sc_5_1_1_27,
sc_5_1_1_29,
sc_5_1_1_30,
sc_5_1_1_31,
sc_5_1_1_32,
sc_5_1_1_33,
sc_5_1_1_40,
sc_5_1_4_1_1_1,
sc_5_1_4_1_1_1_1,
sc_5_1_4_1_1_1_1_1,
sc_5_1_4_1_1_1_2,
sc_5_1_4_1_1_1_2_1,
sc_5_1_4_1_1_1_3,
sc_5_1_4_1_1_1_3_1,
sc_5_1_4_1_1_2,
sc_5_1_4_1_1_2_1,
sc_5_1_4_1_1_2_2,
sc_5_1_4_1_1_3,
sc_5_1_4_1_1_3_1,
sc_5_1_4_1_1_4,
sc_5_1_4_1_1_4_1,
sc_5_1_4_1_1_4_2,
sc_5_1_4_1_1_4_3,
sc_5_1_4_1_1_4_4,
sc_5_1_4_1_1_5,
sc_5_1_4_1_1_6,
sc_5_1_4_1_1_6_1,
sc_5_1_4_1_1_6_2,
sc_5_1_4_1_1_6_3,
sc_5_1_4_1_1_7,
sc_5_1_4_1_1_7_1,
sc_5_1_4_1_1_7_2,
sc_5_1_4_1_1_7_3,
sc_5_1_4_1_1_7_4,
sc_5_1_4_1_1_8,
sc_5_1_4_1_1_9,
sc_5_1_4_1_1_9_1,
sc_5_1_4_1_1_9_1_1,
sc_5_1_4_1_1_9_1_2,
sc_5_1_4_1_1_9_1_3,
sc_5_1_4_1_1_9_1_4,
sc_5_1_4_1_1_9_2_1,
sc_5_1_4_1_1_9_3_1,
sc_5_1_4_1_1_9_4_1,
sc_5_1_4_1_1_9_4_2,
sc_5_1_4_1_1_9_5_1,
sc_5_1_4_1_1_9_6_1,
sc_5_1_4_1_1_9_6_2,
sc_5_1_4_1_1_9_7_1,
sc_5_1_4_1_1_9_7_2,
sc_5_1_4_1_1_9_7_3,
sc_5_1_4_1_1_9_7_4,
sc_5_1_4_1_1_9_8_1,
sc_5_1_4_1_1_10,
sc_5_1_4_1_1_11,
sc_5_1_4_1_1_11_1,
sc_5_1_4_1_1_11_2,
sc_5_1_4_1_1_11_3,
sc_5_1_4_1_1_11_4,
sc_5_1_4_1_1_11_5,
sc_5_1_4_1_1_11_6,
sc_5_1_4_1_1_11_7,
sc_5_1_4_1_1_11_8,
sc_5_1_4_1_1_11_9,
sc_5_1_4_1_1_11_10,
sc_5_1_4_1_1_11_11,
sc_5_1_4_1_1_11_12,
sc_5_1_4_1_1_12_1,
sc_5_1_4_1_1_12_1_1,
sc_5_1_4_1_1_12_2,
sc_5_1_4_1_1_12_2_1,
sc_5_1_4_1_1_12_3,
sc_5_1_4_1_1_12_77,
sc_5_1_4_1_1_13_1_1,
sc_5_1_4_1_1_13_1_2,
sc_5_1_4_1_1_13_1_3,
sc_5_1_4_1_1_13_1_4,
sc_5_1_4_1_1_13_1_5,
sc_5_1_4_1_1_14_1,
sc_5_1_4_1_1_14_2,
sc_5_1_4_1_1_20,
sc_5_1_4_1_1_30,
sc_5_1_4_1_1_40,
sc_5_1_4_1_1_66,
sc_5_1_4_1_1_66_1,
sc_5_1_4_1_1_66_2,
sc_5_1_4_1_1_66_3,
sc_5_1_4_1_1_66_4,
sc_5_1_4_1_1_66_5,
sc_5_1_4_1_1_66_6,
sc_5_1_4_1_1_67,
sc_5_1_4_1_1_68_1,
sc_5_1_4_1_1_68_2,
sc_5_1_4_1_1_77_1,
sc_5_1_4_1_1_77_2,
sc_5_1_4_1_1_77_1_1,
sc_5_1_4_1_1_77_1_1_1,
sc_5_1_4_1_1_77_1_2,
sc_5_1_4_1_1_77_1_2_1,
sc_5_1_4_1_1_77_1_3,
sc_5_1_4_1_1_77_1_4,
sc_5_1_4_1_1_77_1_4_1,
sc_5_1_4_1_1_77_1_5_1,
sc_5_1_4_1_1_77_1_5_2,
sc_5_1_4_1_1_77_1_5_3,
sc_5_1_4_1_1_77_1_5_4,
sc_5_1_4_1_1_77_1_5_5,
sc_5_1_4_1_1_77_1_5_6,
sc_5_1_4_1_1_77_1_5_7,
sc_5_1_4_1_1_77_1_5_8,
sc_5_1_4_1_1_77_1_6,
sc_5_1_4_1_1_77_1_7,
sc_5_1_4_1_1_77_1_8,
sc_5_1_4_1_1_77_1_9,
sc_5_1_4_1_1_78_1,
sc_5_1_4_1_1_78_2,
sc_5_1_4_1_1_78_3,
sc_5_1_4_1_1_78_4,
sc_5_1_4_1_1_78_5,
sc_5_1_4_1_1_78_6,
sc_5_1_4_1_1_78_7,
sc_5_1_4_1_1_78_8,
sc_5_1_4_1_1_79_1,
sc_5_1_4_1_1_80_1,
sc_5_1_4_1_1_81_1,
sc_5_1_4_1_1_82_1,
sc_5_1_4_1_1_88_1,
sc_5_1_4_1_1_88_2,
sc_5_1_4_1_1_88_3,
sc_5_1_4_1_1_88_4,
sc_5_1_4_1_1_88_11,
sc_5_1_4_1_1_88_22,
sc_5_1_4_1_1_88_33,
sc_5_1_4_1_1_88_34,
sc_5_1_4_1_1_88_35,
sc_5_1_4_1_1_88_40,
sc_5_1_4_1_1_88_50,
sc_5_1_4_1_1_88_59,
sc_5_1_4_1_1_88_65,
sc_5_1_4_1_1_88_67,
sc_5_1_4_1_1_88_68,
sc_5_1_4_1_1_88_69,
sc_5_1_4_1_1_88_70,
sc_5_1_4_1_1_88_71,
sc_5_1_4_1_1_88_72,
sc_5_1_4_1_1_88_73,
sc_5_1_4_1_1_88_74,
sc_5_1_4_1_1_88_75,
sc_5_1_4_1_1_88_76,
sc_5_1_4_1_1_90_1,
sc_5_1_4_1_1_91_1,
sc_5_1_4_1_1_104_1,
sc_5_1_4_1_1_104_2,
sc_5_1_4_1_1_104_3,
sc_5_1_4_1_1_104_4,
sc_5_1_4_1_1_104_5,
sc_5_1_4_1_1_128,
sc_5_1_4_1_1_128_1,
sc_5_1_4_1_1_129,
sc_5_1_4_1_1_130,
sc_5_1_4_1_1_131,
sc_5_1_4_1_1_200_1,
sc_5_1_4_1_1_200_2,
sc_5_1_4_1_1_200_3,
sc_5_1_4_1_1_200_4,
sc_5_1_4_1_1_200_5,
sc_5_1_4_1_1_200_6,
sc_5_1_4_1_1_200_7,
sc_5_1_4_1_1_200_8,
sc_5_1_4_1_1_201_1,
sc_5_1_4_1_1_201_2,
sc_5_1_4_1_1_201_3,
sc_5_1_4_1_1_201_4,
sc_5_1_4_1_1_201_5,
sc_5_1_4_1_1_201_6,
sc_5_1_4_1_1_481_1,
sc_5_1_4_1_1_481_2,
sc_5_1_4_1_1_481_3,
sc_5_1_4_1_1_481_4,
sc_5_1_4_1_1_481_5,
sc_5_1_4_1_1_481_6,
sc_5_1_4_1_1_481_7,
sc_5_1_4_1_1_481_8,
sc_5_1_4_1_1_481_9,
sc_5_1_4_1_1_481_10,
sc_5_1_4_1_1_481_11,
sc_5_1_4_1_1_481_12,
sc_5_1_4_1_1_481_13,
sc_5_1_4_1_1_481_14,
sc_5_1_4_1_1_481_15,
sc_5_1_4_1_1_481_16,
sc_5_1_4_1_1_481_17,
sc_5_1_4_1_1_481_18,
sc_5_1_4_1_1_481_19,
sc_5_1_4_1_1_481_20,
sc_5_1_4_1_1_481_21,
sc_5_1_4_1_1_481_22,
sc_5_1_4_1_1_481_23,
sc_5_1_4_1_1_481_24,
sc_5_1_4_1_1_481_25,
sc_5_1_4_1_1_501_1,
sc_5_1_4_1_1_501_2_1,
sc_5_1_4_1_1_501_2_2,
sc_5_1_4_1_1_501_3,
sc_5_1_4_1_1_501_4,
sc_5_1_4_1_1_501_5,
sc_5_1_4_1_1_501_6,
sc_5_1_4_1_1_601_1,
sc_5_1_4_1_1_601_2,
sc_5_1_4_1_2_1_1,
sc_5_1_4_1_2_1_2,
sc_5_1_4_1_2_1_3,
sc_5_1_4_1_2_2_1,
sc_5_1_4_1_2_2_2,
sc_5_1_4_1_2_2_3,
sc_5_1_4_1_2_3_1,
sc_5_1_4_1_2_3_2,
sc_5_1_4_1_2_3_3,
sc_5_1_4_1_2_4_2,
sc_5_1_4_1_2_4_3,
sc_5_1_4_1_2_5_3,
sc_5_1_4_20_1,
sc_5_1_4_20_2,
sc_5_1_4_20_3,
sc_5_1_4_31,
sc_5_1_4_32,
sc_5_1_4_32_1,
sc_5_1_4_32_2,
sc_5_1_4_32_3,
sc_5_1_4_33,
sc_5_1_4_34_1,
sc_5_1_4_34_2,
sc_5_1_4_34_3,
sc_5_1_4_34_4,
sc_5_1_4_34_4_1,
sc_5_1_4_34_4_2,
sc_5_1_4_34_4_3,
sc_5_1_4_34_4_4,
sc_5_1_4_34_5,
sc_5_1_4_34_5_1,
sc_5_1_4_34_6,
sc_5_1_4_34_6_1,
sc_5_1_4_34_6_2,
sc_5_1_4_34_6_3,
sc_5_1_4_34_6_4,
sc_5_1_4_34_6_5,
sc_5_1_4_34_7,
sc_5_1_4_34_8,
sc_5_1_4_34_9,
sc_5_1_4_34_10,
sc_5_1_4_37_1,
sc_5_1_4_37_2,
sc_5_1_4_37_3,
sc_5_1_4_38_1,
sc_5_1_4_38_2,
sc_5_1_4_38_3,
sc_5_1_4_38_4,
sc_5_1_4_39_1,
sc_5_1_4_39_2,
sc_5_1_4_39_3,
sc_5_1_4_39_4,
sc_5_1_4_41,
sc_5_1_4_42,
sc_5_1_4_43_1,
sc_5_1_4_43_2,
sc_5_1_4_43_3,
sc_5_1_4_43_4,
sc_5_1_4_44_1,
sc_5_1_4_44_2,
sc_5_1_4_44_3,
sc_5_1_4_44_4,
sc_5_1_4_45_1,
sc_5_1_4_45_2,
sc_5_1_4_45_3,
sc_5_1_4_45_4,
sc_10_1,
sc_10_2,
sc_10_3,
sc_10_4
};

enum sc_keyenum{
Verification,//0
MediaStorageDirectoryStorage,
BasicStudyContentNotification,
StorageCommitmentPushModel,
StorageCommitmentPullModel,
ProceduralEventLogging,
SubstanceAdministrationLogging,
DetachedPatientManagement,
DetachedPatientManagementMeta,
DetachedVisitManagement,
DetachedStudyManagement,//10
StudyComponentManagement,
ModalityPerformedProcedureStep,
ModalityPerformedProcedureStepRetrieve,
ModalityPerformedProcedureStepNotification,
DetachedResultsManagement,
DetachedResultsManagementMeta,
DetachedStudyManagementMeta,
DetachedInterpretationManagement,
BasicFilmSession,
BasicFilmBox,//20
BasicGrayscaleImageBox,
BasicColorImageBox,
ReferencedImageBox,
BasicGrayscalePrintManagementMeta,
ReferencedGrayscalePrintManagementMeta,
PrintJob,
BasicAnnotationBox,
Printer,
PrinterConfigurationRetrieval,
BasicColorPrintManagementMeta,//30
ReferencedColorPrintManagementMeta,
VOILUTBox,
PresentationLUT,
ImageOverlayBox,
BasicPrintImageOverlayBox,
PrintQueueManagement,
StoredPrintStorage,
HardcopyGrayscaleImageStorage,
HardcopyColorImageStorage,
PullPrintRequest,//40
PullStoredPrintManagementMeta,
MediaCreationManagement,
DisplaySystem,
ComputedRadiographyImageStorage,
DigitalXRayImageStorageForPresentation,
DigitalXRayImageStorageForProcessing,
DigitalMammographyXRayImageStorageForPresentation,
DigitalMammographyXRayImageStorageForProcessing,
DigitalIntraOralXRayImageStorageForPresentation,
DigitalIntraOralXRayImageStorageForProcessing,//50
CTImageStorage,
EnhancedCTImageStorage,
LegacyConvertedEnhancedCTImageStorage,
UltrasoundMultiFrameImageStorageRetired,
UltrasoundMultiFrameImageStorage,
MRImageStorage,
EnhancedMRImageStorage,
MRSpectroscopyStorage,
EnhancedMRColorImageStorage,
LegacyConvertedEnhancedMRImageStorage,//60
NuclearMedicineImageStorageRetired,
UltrasoundImageStorageRetired,
UltrasoundImageStorage,
EnhancedUSVolumeStorage,
PhotoacousticImageStorage,
SecondaryCaptureImageStorage,
MultiFrameSingleBitSecondaryCaptureImageStorage,
MultiFrameGrayscaleByteSecondaryCaptureImageStorage,
MultiFrameGrayscaleWordSecondaryCaptureImageStorage,
MultiFrameTrueColorSecondaryCaptureImageStorage,
StandaloneOverlayStorage,
StandaloneCurveStorage,
WaveformStorageTrial,
TwelveLeadECGWaveformStorage,
GeneralECGWaveformStorage,
AmbulatoryECGWaveformStorage,
General32bitECGWaveformStorage,
HemodynamicWaveformStorage,
CardiacElectrophysiologyWaveformStorage,
BasicVoiceAudioWaveformStorage,
GeneralAudioWaveformStorage,
ArterialPulseWaveformStorage,
RespiratoryWaveformStorage,
MultichannelRespiratoryWaveformStorage,
RoutineScalpElectroencephalogramWaveformStorage,
ElectromyogramWaveformStorage,
ElectrooculogramWaveformStorage,
SleepElectroencephalogramWaveformStorage,
BodyPositionWaveformStorage,
StandaloneModalityLUTStorage,
StandaloneVOILUTStorage,
GrayscaleSoftcopyPresentationStateStorage,
ColorSoftcopyPresentationStateStorage,
PseudoColorSoftcopyPresentationStateStorage,
BlendingSoftcopyPresentationStateStorage,
XAXRFGrayscaleSoftcopyPresentationStateStorage,
GrayscalePlanarMPRVolumetricPresentationStateStorage,
CompositingPlanarMPRVolumetricPresentationStateStorage,
AdvancedBlendingPresentationStateStorage,
VolumeRenderingVolumetricPresentationStateStorage,
SegmentedVolumeRenderingVolumetricPresentationStateStorage,
MultipleVolumeRenderingVolumetricPresentationStateStorage,
VariableModalityLUTSoftcopyPresentationStateStorage,
XRayAngiographicImageStorage,
EnhancedXAImageStorage,
XRayRadiofluoroscopicImageStorage,
EnhancedXRFImageStorage,
XRayAngiographicBiPlaneImageStorage,
Retired,
XRay3DAngiographicImageS,
XRay3DCraniofacialImageStorage,
BreastTomosynthesisImageStorage,
BreastProjectionXRayImageStorageForPresentation,
BreastProjectionXRayImageStorageForProcessing,
IntravascularOpticalCoherenceTomographyImageStorageForPresentation,
IntravascularOpticalCoherenceTomographyImageStorageForProcessing,
NuclearMedicineImageStorage,
ParametricMapStorage,
Retired2,
RawDataStorage,
SpatialRegistrationStorage,
SpatialFiducialsStorage,
DeformableSpatialRegistrationStorage,
SegmentationStorage,
SurfaceSegmentationStorage,
TractographyResultsStorage,
RealWorldValueMappingStorage,
SurfaceScanMeshStorage,
SurfaceScanPointCloudStorage,
VLImageStorageTrial,
VLMultiFrameImageStorageTrial,
VLEndoscopicImageStorage,
VideoEndoscopicImageStorage,
VLMicroscopicImageStorage,
VideoMicroscopicImageStorage,
VLSlideCoordinatesMicroscopicImageStorage,
VLPhotographicImageStorage,
VideoPhotographicImageStorage,
OphthalmicPhotography8BitImageStorage,
OphthalmicPhotography16BitImageStorage,
StereometricRelationshipStorage,
OphthalmicTomographyImageStorage,
WideFieldOphthalmicPhotographyStereographicProjectionImageStorage,
WideFieldOphthalmicPhotography3DCoordinatesImageStorage,
OphthalmicOpticalCoherenceTomographyEnFaceImageStorage,
OphthalmicOpticalCoherenceTomographyBscanVolumeAnalysisStorage,
VLWholeSlideMicroscopyImageStorage,
DermoscopicPhotographyImageStorage,
ConfocalMicroscopyImageStorage,
ConfocalMicroscopyTiledPyramidalImageStorage,
LensometryMeasurementsStorage,
AutorefractionMeasurementsStorage,
KeratometryMeasurementsStorage,
SubjectiveRefractionMeasurementsStorage,
VisualAcuityMeasurementsStorage,
SpectaclePrescriptionReportStorage,
OphthalmicAxialMeasurementsStorage,
IntraocularLensCalculationsStorage,
MacularGridThicknessAndVolumeReportStorage,
OphthalmicVisualFieldStaticPerimetryMeasurementsStorage,
OphthalmicThicknessMapStorage,
CornealTopographyMapStorage,
TextSRStorageTrial,
AudioSRStorageTrial,
DetailSRStorageTrial,
ComprehensiveSRStorageTrial,
BasicTextSRStorage,
EnhancedSRStorage,
ComprehensiveSRStorage,
Comprehensive3DSRStorage,
ExtensibleSRStorage,
ProcedureLogStorage,
MammographyCADSRStorage,
KeyObjectSelectionDocumentStorage,
ChestCADSRStorage,
XRayRadiationDoseSRStorage,
RadiopharmaceuticalRadiationDoseSRStorage,
ColonCADSRStorage,
ImplantationPlanSRStorage,
AcquisitionContextSRStorage,
SimplifiedAdultEchoSRStorage,
PatientRadiationDoseSRStorage,
PlannedImagingAgentAdministrationSRStorage,
PerformedImagingAgentAdministrationSRStorage,
EnhancedXRayRadiationDoseSRStorage,
ContentAssessmentResultsStorage,
MicroscopyBulkSimpleAnnotationsStorage,
EncapsulatedPDFStorage,
EncapsulatedCDAStorage,
EncapsulatedSTLStorage,
EncapsulatedOBJStorage,
EncapsulatedMTLStorage,
PositronEmissionTomographyImageStorage,
LegacyConvertedEnhancedPETImageStorage,
StandalonePETCurveStorage,
EnhancedPETImageStorage,
BasicStructuredDisplayStorage,
CTDefinedProcedureProtocolStorage,
CTPerformedProcedureProtocolStorage,
ProtocolApprovalStorage,
ProtocolApprovalInformationModelFind,
ProtocolApprovalInformationModelMove,
ProtocolApprovalInformationModelGet,
XADefinedProcedureProtocolStorage,
XAPerformedProcedureProtocolStorage,
InventoryStorage,
InventoryFind,
InventoryMove,
InventoryGet,
InventoryCreation,
RepositoryQuery,
RTImageStorage,
RTDoseStorage,
RTStructureSetStorage,
RTBeamsTreatmentRecordStorage,
RTPlanStorage,
RTBrachyTreatmentRecordStorage,
RTTreatmentSummaryRecordStorage,
RTIonPlanStorage,
RTIonBeamsTreatmentRecordStorage,
RTPhysicianIntentStorage,
RTSegmentAnnotationStorage,
RTRadiationSetStorage,
CArmPhotonElectronRadiationStorage,
TomotherapeuticRadiationStorage,
RoboticArmRadiationStorage,
RTRadiationRecordSetStorage,
RTRadiationSalvageRecordStorage,
TomotherapeuticRadiationRecordStorage,
CArmPhotonElectronRadiationRecordStorage,
RoboticRadiationRecordStorage,
RTRadiationSetDeliveryInstructionStorage,
RTTreatmentPreparationStorage,
EnhancedRTImageStorage,
EnhancedContinuousRTImageStorage,
RTPatientPositionAcquisitionInstructionStorage,
DICOSCTImageStorage,
DICOSDigitalXRayImageStorageForPresentation,
DICOSDigitalXRayImageStorageForProcessing,
DICOSThreatDetectionReportStorage,
DICOS2DAITStorage,
DICOS3DAITStorage,
DICOSQuadrupoleResonanceStorage,
EddyCurrentImageStorage,
EddyCurrentMultiFrameImageStorage,
PatientRootQueryRetrieveInformationModelFind,
PatientRootQueryRetrieveInformationModelMove,
PatientRootQueryRetrieveInformationModelGet,
StudyRootQueryRetrieveInformationModelFind,
StudyRootQueryRetrieveInformationModelMove,
StudyRootQueryRetrieveInformationModelGet,
PatientStudyOnlyQueryRetrieveInformationModelFind,
PatientStudyOnlyQueryRetrieveInformationModelMove,
PatientStudyOnlyQueryRetrieveInformationModelGet,
CompositeInstanceRootRetrieveMove,
CompositeInstanceRootRetrieveGet,
CompositeInstanceRetrieveWithoutBulkDataGet,
DefinedProcedureProtocolInformationModelFind,
DefinedProcedureProtocolInformationModelMove,
DefinedProcedureProtocolInformationModelGet,
ModalityWorklistInformationModelFind,
GeneralPurposeWorklistManagementMeta,
GeneralPurposeWorklistInformationModelFind,
GeneralPurposeScheduledProcedureStep,
GeneralPurposePerformedProcedureStep,
InstanceAvailabilityNotification,
RTBeamsDeliveryInstructionStorageTrial,
RTConventionalMachineVerificationTrial,
RTIonMachineVerificationTrial,
UnifiedWorklistAndProcedureStepTrial,
UnifiedProcedureStepPushTrial,
UnifiedProcedureStepWatchTrial,
UnifiedProcedureStepPullTrial,
UnifiedProcedureStepEventTrial,
UPSGlobalSubscriptionInstance,
UPSFilteredGlobalSubscriptionInstance,
UnifiedWorklistAndProcedureStep,
UnifiedProcedureStepPush,
UnifiedProcedureStepWatch,
UnifiedProcedureStepPull,
UnifiedProcedureStepEvent,
UnifiedProcedureStepQuery,
RTBeamsDeliveryInstructionStorage,
RTConventionalMachineVerification,
RTIonMachineVerification,
RTBrachyApplicationSetupDeliveryInstructionStorage,
GeneralRelevantPatientInformationQuery,
BreastImagingRelevantPatientInformationQuery,
CardiacRelevantPatientInformationQuery,
HangingProtocolStorage,
HangingProtocolInformationModelFind,
HangingProtocolInformationModelMove,
HangingProtocolInformationModelGet,
ColorPaletteStorage,
ColorPaletteQueryRetrieveInformationModelFind,
ColorPaletteQueryRetrieveInformationModelMove,
ColorPaletteQueryRetrieveInformationModelGet,
ProductCharacteristicsQuery,
SubstanceApprovalQuery,
GenericImplantTemplateStorage,
GenericImplantTemplateInformationModelFind,
GenericImplantTemplateInformationModelMove,
GenericImplantTemplateInformationModelGet,
ImplantAssemblyTemplateStorage,
ImplantAssemblyTemplateInformationModelFind,
ImplantAssemblyTemplateInformationModelMove,
ImplantAssemblyTemplateInformationModelGet,
ImplantTemplateGroupStorage,
ImplantTemplateGroupInformationModelFind,
ImplantTemplateGroupInformationModelMove,
ImplantTemplateGroupInformationModelGet,
VideoEndoscopicImageRealTimeCommunication,
VideoPhotographicImageRealTimeCommunication,
AudioWaveformRealTimeCommunication,
RenditionSelectionDocumentRealTimeCommunication
};
   

u16 sopclassidx( char *vbuf, u16 vallength );

bool sopclassidxisimage( u16 sopclassidx);

bool sopclassidxisframes( u16 sopclassidx);


#pragma mark - TransfertSyntaxes


enum sopkeyenum{
PapyrusImplicitVRLittleEndian,
ImplicitVRLittleEndian,
ExplicitVRLittleEndian,
EncapsulatedUncompressedExplicitVRLittleEndian,
DeflatedExplicitVRLittleEndian,
ExplicitVRBigEndianRetired,
JPEGBaseline8Bit,
JPEGExtended12Bit,
JPEGExtended35Retired,
JPEGSpectralSelectionNonHierarchical68Retired,
JPEGSpectralSelectionNonHierarchical79Retired,
JPEGFullProgressionNonHierarchical1012Retired,
JPEGFullProgressionNonHierarchical1113Retired,
JPEGLossless,
JPEGLosslessNonHierarchical15Retired,
JPEGExtendedHierarchical1618Retired,
JPEGExtendedHierarchical1719Retired,
JPEGSpectralSelectionHierarchical2022Retired,
JPEGSpectralSelectionHierarchical2123Retired,
JPEGFullProgressionHierarchical2426Retired,
JPEGFullProgressionHierarchical2527Retired,
JPEGLosslessHierarchical28Retired,
JPEGLosslessHierarchical29Retired,
JPEGLosslessSV1,
JPEGLSLossless,
JPEGLSNearLossless,
JPEG2000Lossless,
JPEG2000,
JPEG2000MCLossless,
JPEG2000MC,
JPIPReferenced,
JPIPReferencedDeflate,
MPEG2MPML,
MPEG2MPMLF,
MPEG2MPHL,
MPEG2MPHLF,
MPEG4HP41,
MPEG4HP41F,
MPEG4HP41BD,
MPEG4HP41BDF,
MPEG4HP422D,
MPEG4HP422DF,
MPEG4HP423D,
MPEG4HP423DF,
MPEG4HP42STEREO,
MPEG4HP42STEREOF,
HEVCMP51,
HEVCM10P51,
HTJ2KLossless,
HTJ2KLosslessRPCL,
HTJ2K,
JPIPHTJ2KReferenced,
JPIPHTJ2KReferencedDeflate,
RLELossless,
RFC2557MIMEEncapsulation,
XMLEncoding,
SMPTEST211020UncompressedProgressiveActiveVideo,
SMPTEST211020UncompressedInterlacedActiveVideo,
SMPTEST211030PCMDigitalAudio
};

//ts_=1.2.840.10008.1.2
enum ts_uidenum{
ts_papyrus3ile,
ts_,
ts_1,
ts_1_98,
ts_1_99,
ts_2,
ts_4_50,
ts_4_51,
ts_4_52,
ts_4_53,
ts_4_54,
ts_4_55,
ts_4_56,
ts_4_57,
ts_4_58,
ts_4_59,
ts_4_60,
ts_4_61,
ts_4_62,
ts_4_63,
ts_4_64,
ts_4_65,
ts_4_66,
ts_4_70,
ts_4_80,
ts_4_81,
ts_4_90,
ts_4_91,
ts_4_92,
ts_4_93,
ts_4_94,
ts_4_95,
ts_4_100,
ts_4_100_1,
ts_4_101,
ts_4_101_1,
ts_4_102,
ts_4_102_1,
ts_4_103,
ts_4_103_1,
ts_4_104,
ts_4_104_1,
ts_4_105,
ts_4_105_1,
ts_4_106,
ts_4_106_1,
ts_4_107,
ts_4_108,
ts_4_201,
ts_4_202,
ts_4_203,
ts_4_204,
ts_4_205,
ts_5,
ts_6_1,
ts_6_2,
ts_7_1,
ts_7_2,
ts_7_3
};

u8 tsidx( char *vbuf, u16 vallength );




#endif /* decktypes_h */
