// project: dicm2deck
// uapi override for capi targets
// file: uapi.m
// created by jacquesfauquex on 2024-04-04.

#include "capi.h"
#include <locale.h>


#pragma mark base dataset level patient

const u32 B00081120P=0x20110800;//SQ Referenced Patient Sequence
const u32 B00100010P=0x10001000;//PN Patient​Name
const u32 B00100020P=0x20001000;//LO Patient​ID
const u32 B00100021P=0x21001000;//LO Issuer​Of​Patient​ID
const u32 B00100022P=0x22001000;//CS Type​Of​Patient​ID
const u32 B00100024P=0x24001000;//SQ Issuer​Of​Patient​ID​Qualifiers​Sequence
const u32 B00100026P=0x26001000;//SQ Source​Patient​Group​Identification​Sequence
const u32 B00100027P=0x27001000;//SQ Group​Of​Patients​Identification​Sequence
const u32 B00100030P=0x30001000;//DA Patient​Birth​Date
const u32 B00100032P=0x32001000;//TM Patient​Birth​Time
const u32 B00100033P=0x33001000;//LO Patient​Birth​Date​In​Alternative​Calendar
const u32 B00100034P=0x34001000;//LO Patient​Death​Date​In​Alternative​Calendar
const u32 B00100035P=0x35001000;//CS Patient​Alternative​Calendar
const u32 B00100040P=0x40001000;//CS Patient​Sex
const u32 B00100200P=0x00021000;//CS Quality​Control​Subject
const u32 B00100212P=0x12021000;//UC Strain​Description
const u32 B00100213P=0x13021000;//LO Strain​Nomenclature
const u32 B00100216P=0x16021000;//SQ Strain​Stock​Sequence
const u32 B00100218P=0x18021000;//UT Strain​Additional​Information
const u32 B00100219P=0x19021000;//SQ Strain​Code​Sequence
const u32 B00100221P=0x21021000;//SQ Genetic​Modifications​​Sequence
const u32 B00101000P=0x00101000;//LO Other​Patient​IDs
const u32 B00101001P=0x01101000;//PN Other​Patient​Names
const u32 B00101002P=0x02101000;//SQ Other​Patient​IDs​Sequence
const u32 B00101100P=0x00111000;//SQ Referenced​Patient​Photo​Sequence
const u32 B00102160P=0x60211000;//SH Ethnic​Group
const u32 B00102201P=0x01221000;//LO Patient​Species​Description
const u32 B00102202P=0x02221000;//SQ Patient​Species​Code​Sequence
const u32 B00102292P=0x92221000;//LO Patient​Breed​Description
const u32 B00102293P=0x93221000;//SQ Patient​Breed​Code​Sequence
const u32 B00102294P=0x94221000;//SQ Breed​Registration​Sequence
const u32 B00102297P=0x97221000;//PN Responsible​Person
const u32 B00102298P=0x98221000;//CS Responsible​Person​Role
const u32 B00102299P=0x99221000;//LO Responsible​Organization
const u32 B00104000P=0x00401000;//LT Patient​Comments
const u32 B00120010P=0x10001200;//LO Clinical​Trial​Sponsor​Name
const u32 B00120020P=0x20001200;//LO Clinical​Trial​Protocol​ID
const u32 B00120021P=0x21001200;//LO Clinical​Trial​Protocol​Name
const u32 B00120030P=0x30001200;//LO Clinical​Trial​Site​ID
const u32 B00120031P=0x31001200;//LO Clinical​Trial​Site​Name
const u32 B00120040P=0x40001200;//LO Clinical​Trial​Subject​ID
const u32 B00120042P=0x42001200;//LO Clinical​Trial​Subject​Reading​ID
const u32 B00120062P=0x62001200;//CS Patient​Identity​Removed
const u32 B00120063P=0x63001200;//LO Deidentification​Method
const u32 B00120064P=0x64001200;//SQ Deidentification​Method​Code​Sequence
const u32 B00120081P=0x81001200;//LO Clinical​Trial​Protocol​Ethics​Committee​Name
const u32 B00120082P=0x82001200;//LO Clinical​Trial​Protocol​Ethics​Committee​Approval​Number
/*
const u32 L00081120P=0x00081120;//SQ Referenced Patient Sequence
const u32 L00100010P=0x00100010;//PN Patient​Name
const u32 L00100020P=0x00100020;//LO Patient​ID
const u32 L00100021P=0x00100021;//LO Issuer​Of​Patient​ID
const u32 L00100022P=0x00100022;//CS Type​Of​Patient​ID
const u32 L00100024P=0x00100024;//SQ Issuer​Of​Patient​ID​Qualifiers​Sequence
const u32 L00100026P=0x00100026;//SQ Source​Patient​Group​Identification​Sequence
const u32 L00100027P=0x00100027;//SQ Group​Of​Patients​Identification​Sequence
const u32 L00100030P=0x00100030;//DA Patient​Birth​Date
const u32 L00100032P=0x00100032;//TM Patient​Birth​Time
const u32 L00100033P=0x00100033;//LO Patient​Birth​Date​In​Alternative​Calendar
const u32 L00100034P=0x00100034;//LO Patient​Death​Date​In​Alternative​Calendar
const u32 L00100035P=0x00100035;//CS Patient​Alternative​Calendar
const u32 L00100040P=0x00100040;//CS Patient​Sex
const u32 L00100200P=0x00100200;//CS Quality​Control​Subject
const u32 L00100212P=0x00100212;//UC Strain​Description
const u32 L00100213P=0x00100213;//LO Strain​Nomenclature
const u32 L00100216P=0x00100216;//SQ Strain​Stock​Sequence
const u32 L00100218P=0x00100218;//UT Strain​Additional​Information
const u32 L00100219P=0x00100219;//SQ Strain​Code​Sequence
const u32 L00100221P=0x00100221;//SQ Genetic​Modifications​​Sequence
const u32 L00101000P=0x00101000;//LO Other​Patient​IDs
const u32 L00101001P=0x00101001;//PN Other​Patient​Names
const u32 L00101002P=0x00101002;//SQ Other​Patient​IDs​Sequence
const u32 L00101100P=0x00101100;//SQ Referenced​Patient​Photo​Sequence
const u32 L00102160P=0x00102160;//SH Ethnic​Group
const u32 L00102201P=0x00102201;//LO Patient​Species​Description
const u32 L00102202P=0x00102202;//SQ Patient​Species​Code​Sequence
const u32 L00102292P=0x00102292;//LO Patient​Breed​Description
const u32 L00102293P=0x00102293;//SQ Patient​Breed​Code​Sequence
const u32 L00102294P=0x00102294;//SQ Breed​Registration​Sequence
const u32 L00102297P=0x00102297;//PN Responsible​Person
const u32 L00102298P=0x00102298;//CS Responsible​Person​Role
const u32 L00102299P=0x00102299;//LO Responsible​Organization
const u32 L00104000P=0x00104000;//LT Patient​Comments
const u32 L00120010P=0x00120010;//LO Clinical​Trial​Sponsor​Name
const u32 L00120020P=0x00120020;//LO Clinical​Trial​Protocol​ID
const u32 L00120021P=0x00120021;//LO Clinical​Trial​Protocol​Name
const u32 L00120030P=0x00120030;//LO Clinical​Trial​Site​ID
const u32 L00120031P=0x00120031;//LO Clinical​Trial​Site​Name
const u32 L00120040P=0x00120040;//LO Clinical​Trial​Subject​ID
const u32 L00120042P=0x00120042;//LO Clinical​Trial​Subject​Reading​ID
const u32 L00120062P=0x00120062;//CS Patient​Identity​Removed
const u32 L00120063P=0x00120063;//LO Deidentification​Method
const u32 L00120064P=0x00120064;//SQ Deidentification​Method​Code​Sequence
const u32 L00120081P=0x00120081;//LO Clinical​Trial​Protocol​Ethics​Committee​Name
const u32 L00120082P=0x00120082;//LO Clinical​Trial​Protocol​Ethics​Committee​Approval​Number
*/

#pragma mark base dataset level study

const u32 B00080020E=0x20000800;//DA Study​Date
const u32 B00080030E=0x30000800;//TM Study​Time
const u32 B00080050E=0x50000800;//SH Accession​Number
const u32 B00080051E=0x51000800;//SQ Issuer​Of​Accession​Number​Sequence
const u32 B00080090E=0x90000800;//PN Referring​Physician​Name
const u32 B00080096E=0x96000800;//SQ Referring​Physician​Identification​Sequence
const u32 B0008009CE=0x9C000800;//PN Consulting​Physician​Name
const u32 B0008009DE=0x9D000800;//SQ Consulting​Physician​Identification​Sequence
const u32 B00081030E=0x30100800;//LO Study​Description
const u32 B00081032E=0x32100800;//SQ Procedure​Code​Sequence
const u32 B00081048E=0x48100800;//PN Physicians​Of​Record
const u32 B00081049E=0x49100800;//SQ Physicians​Of​Record​Identification​Sequence
const u32 B00081060E=0x60100800;//PN Name​Of​Physicians​Reading​Study
const u32 B00081062E=0x62100800;//SQ Physicians​Reading​Study​Identification​Sequence
const u32 B00081080E=0x80100800;//LO Admitting​Diagnoses​Description
const u32 B00081084E=0x84100800;//SQ Admitting​Diagnoses​Code​Sequence
const u32 B00081110E=0x10110800;//SQ Referenced​Study​Sequence
const u32 B00101010E=0x10101000;//AS Patient​Age
const u32 B00101020E=0x20101000;//DS Patient​Size
const u32 B00101021E=0x21101000;//SQ Patient​Size​Code​Sequence
const u32 B00101022E=0x22101000;//DS Patient​Body​Mass​Index
const u32 B00101023E=0x23101000;//DS Measured​APDimension
const u32 B00101024E=0x24101000;//DS Measured​Lateral​Dimension
const u32 B00101030E=0x30101000;//DS Patient​Weight
const u32 B00102000E=0x00201000;//LO Medical​Alerts
const u32 B00102110E=0x10211000;//LO Allergies
const u32 B00102180E=0x80211000;//SH Occupation
const u32 B001021A0E=0xA0211000;//CS Smoking​Status
const u32 B001021B0E=0xB0211000;//LT Additional​Patient​History
const u32 B001021C0E=0xC0211000;//US Pregnancy​Status
const u32 B001021D0E=0xD0211000;//DA Last​Menstrual​Date
const u32 B00102203E=0x03221000;//CS Patient​Sex​Neutered
const u32 B00120050E=0x50001200;//LO Clinical​Trial​Time​Point​ID
const u32 B00120051E=0x51001200;//ST Clinical​Trial​Time​Point​Description
const u32 B00120052E=0x52001200;//FD Longitudinal​Temporal​Offset​From​Event
const u32 B00120053E=0x53001200;//CS Longitudinal​Temporal​Event​Type
const u32 B00120054E=0x54001200;//SQ Clinical​Trial​Time​Point​Type​Code​Sequence
const u32 B00120083E=0x83001200;//SQ Consent​For​Clinical​Trial​Use​Sequence
const u32 B0020000DE=0x0D002000;//UI Study​Instance​UID
const u32 B00200010E=0x10002000;//SH Study​ID
const u32 B00321033E=0x33103200;//LO Requesting​Service
const u32 B00321034E=0x34103200;//SQ Requesting​Service​Code​Sequence
const u32 B00321066E=0x66103200;//UT Reason​For​Visit
const u32 B00321067E=0x67103200;//SQ Reason​For​Visit​Code​Sequence
const u32 B00380010E=0x10003800;//LO Admission​ID
const u32 B00380014E=0x14003800;//SQ Issuer​Of​Admission​ID​Sequence
const u32 B00380060E=0x60003800;//LO Service​Episode​ID
const u32 B00380062E=0x62003800;//LO Service​Episode​Description
const u32 B00380064E=0x64003800;//SQ Issuer​Of​Service​Episode​ID​Sequence
const u32 B00385000E=0x00053800;//LO Patient​State
const u32 B00401012E=0x12104000;//SQ Reason​For​Performed​Procedure​Code​Sequence
/*
const u32 L00080020E=0x00080020;//DA Study​Date
const u32 L00080030E=0x00080030;//TM Study​Time
const u32 L00080050E=0x00080050;//SH Accession​Number
const u32 L00080051E=0x00080051;//SQ Issuer​Of​Accession​Number​Sequence
const u32 L00080090E=0x00080090;//PN Referring​Physician​Name
const u32 L00080096E=0x00080096;//SQ Referring​Physician​Identification​Sequence
const u32 L0008009CE=0x0008009C;//PN Consulting​Physician​Name
const u32 L0008009DE=0x0008009D;//SQ Consulting​Physician​Identification​Sequence
const u32 L00081030E=0x00081030;//LO Study​Description
const u32 L00081032E=0x00081032;//SQ Procedure​Code​Sequence
const u32 L00081048E=0x00081048;//PN Physicians​Of​Record
const u32 L00081049E=0x00081049;//SQ Physicians​Of​Record​Identification​Sequence
const u32 L00081060E=0x00081060;//PN Name​Of​Physicians​Reading​Study
const u32 L00081062E=0x00081062;//SQ Physicians​Reading​Study​Identification​Sequence
const u32 L00081080E=0x00081080;//LO Admitting​Diagnoses​Description
const u32 L00081084E=0x00081084;//SQ Admitting​Diagnoses​Code​Sequence
const u32 L00081110E=0x00081110;//SQ Referenced​Study​Sequence
const u32 L00101010E=0x00101010;//AS Patient​Age
const u32 L00101020E=0x00101020;//DS Patient​Size
const u32 L00101021E=0x00101021;//SQ Patient​Size​Code​Sequence
const u32 L00101022E=0x00101022;//DS Patient​Body​Mass​Index
const u32 L00101023E=0x00101023;//DS Measured​APDimension
const u32 L00101024E=0x00101024;//DS Measured​Lateral​Dimension
const u32 L00101030E=0x00101030;//DS Patient​Weight
const u32 L00102000E=0x00102000;//LO Medical​Alerts
const u32 L00102110E=0x00102110;//LO Allergies
const u32 L00102180E=0x00102180;//SH Occupation
const u32 L001021A0E=0x001021A0;//CS Smoking​Status
const u32 L001021B0E=0x001021B0;//LT Additional​Patient​History
const u32 L001021C0E=0x001021C0;//US Pregnancy​Status
const u32 L001021D0E=0x001021D0;//DA Last​Menstrual​Date
const u32 L00102203E=0x00102203;//CS Patient​Sex​Neutered
const u32 L00120050E=0x00120050;//LO Clinical​Trial​Time​Point​ID
const u32 L00120051E=0x00120051;//ST Clinical​Trial​Time​Point​Description
const u32 L00120052E=0x00120052;//FD Longitudinal​Temporal​Offset​From​Event
const u32 L00120053E=0x00120053;//CS Longitudinal​Temporal​Event​Type
const u32 L00120054E=0x00120054;//SQ Clinical​Trial​Time​Point​Type​Code​Sequence
const u32 L00120083E=0x00120083;//SQ Consent​For​Clinical​Trial​Use​Sequence
const u32 L0020000DE=0x0020000D;//UI Study​Instance​UID
const u32 L00200010E=0x00200010;//SH Study​ID
const u32 L00321033E=0x00321033;//LO Requesting​Service
const u32 L00321034E=0x00321034;//SQ Requesting​Service​Code​Sequence
const u32 L00321066E=0x00321066;//UT Reason​For​Visit
const u32 L00321067E=0x00321067;//SQ Reason​For​Visit​Code​Sequence
const u32 L00380010E=0x00380010;//LO Admission​ID
const u32 L00380014E=0x00380014;//SQ Issuer​Of​Admission​ID​Sequence
const u32 L00380060E=0x00380060;//LO Service​Episode​ID
const u32 L00380062E=0x00380062;//LO Service​Episode​Description
const u32 L00380064E=0x00380064;//SQ Issuer​Of​Service​Episode​ID​Sequence
const u32 L00385000E=0x00385000;//LO Patient​State
const u32 L00401012E=0x00401012;//SQ Reason​For​Performed​Procedure​Code​Sequence
*/

#pragma mark base dataset level series root

const u32 B00080021S=0x21000800;//DA Series​Date
const u32 B00080031S=0x31000800;//TM Series​Time
const u32 B00080060S=0x60000800;//CS Modality
const u32 B00080064S=0x64000800;//CS Conversion​Type
const u32 B00080068S=0x68000800;//CS Presentation​Intent​Type
const u32 B00080070S=0x70000800;//LO Manufacturer
const u32 B00080080S=0x80000800;//LO Institution​Name
const u32 B00080081S=0x81000800;//ST Institution​Address
const u32 B00081010S=0x10100800;//SH Station​Name
const u32 B0008103ES=0x3E100800;//LO Series​Description
const u32 B0008103FS=0x3F100800;//SQ Series​Description​Code​Sequence
const u32 B00081040S=0x40100800;//LO Institutional​Department​Name
const u32 B00081041S=0x41100800;//SQ Institutional​Department​Type​Code​Sequence
const u32 B00081050S=0x50100800;//PN Performing​Physician​Name
const u32 B00081052S=0x52100800;//SQ Performing​Physician​Identification​Sequence
const u32 B00081070S=0x70100800;//PN Operators​Name
const u32 B00081072S=0x72100800;//SQ Operator​Identification​Sequence
const u32 B00081090S=0x90100800;//LO Manufacturer​Model​Name
const u32 B00081111S=0x11110800;//SQ Referenced​Performed​Procedure​Step​Sequence
const u32 B00081250S=0x50120800;//SQ Related​Series​Sequence
const u32 B00120060S=0x60001200;//LO Clinical​Trial​Coordinating​Center​Name
const u32 B00120071S=0x71001200;//LO Clinical​Trial​Series​ID
const u32 B00120072S=0x72001200;//LO Clinical​Trial​Series​Description
const u32 B0016004DS=0x4D001600;//UT Camera​Owner​Name
const u32 B0016004ES=0x4E001600;//DS Lens​Specification
const u32 B0016004FS=0x4F001600;//UT Lens​Make
const u32 B00160050S=0x50001600;//UT Lens​Model
const u32 B00160051S=0x51001600;//UT Lens​Serial​Number
const u32 B00180015S=0x15001800;//CS Body​Part​Examined
const u32 B00180026S=0x26001800;//SQ Intervention​Drug​Information​Sequence
const u32 B00180071S=0x71001800;//CS Acquisition​Termination​Condition
const u32 B00180073S=0x73001800;//CS Acquisition​Start​Condition
const u32 B00180074S=0x74001800;//IS Acquisition​Start​Condition​Data
const u32 B00180075S=0x75001800;//IS Acquisition​Termination​Condition​Data
const u32 B00181000S=0x00101800;//LO Device​Serial​Number
const u32 B00181002S=0x02101800;//UI Device​UID
const u32 B00181008S=0x08101800;//LO Gantry​ID
const u32 B0018100AS=0x0A101800;//SQ UDI​Sequence
const u32 B0018100BS=0x0B101800;//UI Manufacturer​Device​Class​UID
const u32 B00181010S=0x10101800;//LO Secondary​Capture​Device​ID
const u32 B00181016S=0x16101800;//LO Secondary​Capture​Device​Manufacturer
const u32 B00181018S=0x18101800;//LO Secondary​Capture​Device​Manufacturer​Model​Name
const u32 B00181019S=0x19101800;//LO Secondary​Capture​Device​Software​Versions
const u32 B00181020S=0x20101800;//LO Software​Versions
const u32 B00181022S=0x22101800;//SH Video​Image​Format​Acquired
const u32 B00181023S=0x23101800;//LO Digital​Image​Format​Acquired
const u32 B00181030S=0x30101800;//LO Protocol​Name
const u32 B00181050S=0x50101800;//DS Spatial​Resolution
const u32 B00181061S=0x61101800;//LO Trigger​Source​Or​Type
const u32 B00181064S=0x64101800;//LO Cardiac​Framing​Type
const u32 B0018106AS=0x6A101800;//CS Synchronization​Trigger
const u32 B0018106CS=0x6C101800;//US Synchronization​Channel
const u32 B00181080S=0x80101800;//CS Beat​Rejection​Flag
const u32 B00181085S=0x85101800;//LO PVC​Rejection
const u32 B00181086S=0x86101800;//IS Skip​Beats
const u32 B00181088S=0x88101800;//IS Heart​Rate
const u32 B00181100S=0x00111800;//DS Reconstruction​Diameter
const u32 B00181120S=0x20111800;//DS Gantry​Detector​Tilt
const u32 B00181121S=0x21111800;//DS Gantry​Detector​Slew
const u32 B00181147S=0x47111800;//CS Field​Of​View​Shape
const u32 B00181149S=0x49111800;//IS Field​Of​View​Dimensions
const u32 B00181160S=0x60111800;//SH Filter​Type
const u32 B00181180S=0x80111800;//SH Collimator​Grid​Name
const u32 B00181181S=0x81111800;//CS Collimator​Type
const u32 B00181190S=0x90111800;//DS Focal​Spots
const u32 B00181200S=0x00121800;//DA Date​Of​Last​Calibration
const u32 B00181201S=0x01121800;//TM Time​Of​Last​Calibration
const u32 B00181204S=0x04121800;//DA Date​Of​Manufacture
const u32 B00181205S=0x05121800;//DA Date​Of​Installation
const u32 B00181210S=0x10121800;//SH Convolution​Kernel
const u32 B00181260S=0x60121800;//SH Plate​Type
const u32 B00181261S=0x61121800;//LO Phosphor​Type
const u32 B00181800S=0x00181800;//CS Acquisition​Time​Synchronized
const u32 B00181801S=0x01181800;//SH Time​Source
const u32 B00181802S=0x02181800;//CS Time​Distribution​Protocol
const u32 B00181803S=0x03181800;//LO NTP​Source​Address
const u32 B00185100S=0x00511800;//CS Patient​Position
const u32 B00185101S=0x01511800;//CS View​Position
const u32 B0020000ES=0x0E002000;//UI Series​Instance​UID
const u32 B00200011S=0x11002000;//IS Series​Number
const u32 B00200052S=0x52002000;//UI Frame​Of​Reference​UID
const u32 B00200060S=0x60002000;//CS Laterality
const u32 B00200200S=0x00022000;//UI Synchronization​Frame​Of​Reference​UID
const u32 B00201040S=0x40102000;//LO Position​Reference​Indicator
const u32 B00209307S=0x07932000;//CS Ultrasound​Acquisition​Geometry
const u32 B00209308S=0x08932000;//FD Apex​Position
const u32 B00209309S=0x09932000;//FD Volume​To​Transducer​Mapping​Matrix
const u32 B0020930AS=0x0A932000;//FD Volume​To​Table​Mapping​Matrix
const u32 B0020930BS=0x0B932000;//CS Volume​To​Transducer​Relationship
const u32 B0020930CS=0x0C932000;//CS Patient​Frame​Of​Reference​Source
const u32 B00209312S=0x12932000;//UI Volume​Frame​Of​Reference​UID
const u32 B00209313S=0x13932000;//UI Table​Frame​Of​Reference​UID
const u32 B00280051S=0x51002800;//CS Corrected​Image
const u32 B00280108S=0x08012800;//US or SS Smallest​Pixel​Value​In​Series
const u32 B00280109S=0x09012800;//US or SS Largest​Pixel​Value​In​Series
const u32 B00280120S=0x20012800;//US or SS Pixel​Padding​Value
const u32 B00400244S=0x44024000;//DA Performed​Procedure​Step​Start​Date
const u32 B00400245S=0x45024000;//TM Performed​Procedure​Step​Start​Time
const u32 B00400250S=0x50024000;//DA Performed​Procedure​Step​End​Date
const u32 B00400251S=0x51024000;//TM Performed​Procedure​Step​End​Time
const u32 B00400253S=0x53024000;//SH Performed​Procedure​Step​ID
const u32 B00400254S=0x54024000;//LO Performed​Procedure​Step​Description
const u32 B00400260S=0x60024000;//SQ Performed​Protocol​Code​Sequence
const u32 B00400261S=0x61024000;//CS Performed​Protocol​Type
const u32 B00400275S=0x75024000;//SQ Request​Attributes​Sequence
const u32 B00400280S=0x80024000;//ST Comments​On​The​Performed​Procedure​Step
/*
const u32 L00080021S=0x00080021;//DA Series​Date
const u32 L00080031S=0x00080031;//TM Series​Time
const u32 L00080060S=0x00080060;//CS Modality
const u32 L00080064S=0x00080064;//CS Conversion​Type
const u32 L00080068S=0x00080068;//CS Presentation​Intent​Type
const u32 L00080070S=0x00080070;//LO Manufacturer
const u32 L00080080S=0x00080080;//LO Institution​Name
const u32 L00080081S=0x00080081;//ST Institution​Address
const u32 L00081010S=0x00081010;//SH Station​Name
const u32 L0008103ES=0x0008103E;//LO Series​Description
const u32 L0008103FS=0x0008103F;//SQ Series​Description​Code​Sequence
const u32 L00081040S=0x00081040;//LO Institutional​Department​Name
const u32 L00081041S=0x00081041;//SQ Institutional​Department​Type​Code​Sequence
const u32 L00081050S=0x00081050;//PN Performing​Physician​Name
const u32 L00081052S=0x00081052;//SQ Performing​Physician​Identification​Sequence
const u32 L00081070S=0x00081070;//PN Operators​Name
const u32 L00081072S=0x00081072;//SQ Operator​Identification​Sequence
const u32 L00081090S=0x00081090;//LO Manufacturer​Model​Name
const u32 L00081111S=0x00081111;//SQ Referenced​Performed​Procedure​Step​Sequence
const u32 L00081250S=0x00081250;//SQ Related​Series​Sequence
const u32 L00120060S=0x00120060;//LO Clinical​Trial​Coordinating​Center​Name
const u32 L00120071S=0x00120071;//LO Clinical​Trial​Series​ID
const u32 L00120072S=0x00120072;//LO Clinical​Trial​Series​Description
const u32 L0016004DS=0x0016004D;//UT Camera​Owner​Name
const u32 L0016004ES=0x0016004E;//DS Lens​Specification
const u32 L0016004FS=0x0016004F;//UT Lens​Make
const u32 L00160050S=0x00160050;//UT Lens​Model
const u32 L00160051S=0x00160051;//UT Lens​Serial​Number
const u32 L00180015S=0x00180015;//CS Body​Part​Examined
const u32 L00180026S=0x00180026;//SQ Intervention​Drug​Information​Sequence
const u32 L00180071S=0x00180071;//CS Acquisition​Termination​Condition
const u32 L00180073S=0x00180073;//CS Acquisition​Start​Condition
const u32 L00180074S=0x00180074;//IS Acquisition​Start​Condition​Data
const u32 L00180075S=0x00180075;//IS Acquisition​Termination​Condition​Data
const u32 L00181000S=0x00181000;//LO Device​Serial​Number
const u32 L00181002S=0x00181002;//UI Device​UID
const u32 L00181008S=0x00181008;//LO Gantry​ID
const u32 L0018100AS=0x0018100A;//SQ UDI​Sequence
const u32 L0018100BS=0x0018100B;//UI Manufacturer​Device​Class​UID
const u32 L00181010S=0x00181010;//LO Secondary​Capture​Device​ID
const u32 L00181016S=0x00181016;//LO Secondary​Capture​Device​Manufacturer
const u32 L00181018S=0x00181018;//LO Secondary​Capture​Device​Manufacturer​Model​Name
const u32 L00181019S=0x00181019;//LO Secondary​Capture​Device​Software​Versions
const u32 L00181020S=0x00181020;//LO Software​Versions
const u32 L00181022S=0x00181022;//SH Video​Image​Format​Acquired
const u32 L00181023S=0x00181023;//LO Digital​Image​Format​Acquired
const u32 L00181030S=0x00181030;//LO Protocol​Name
const u32 L00181050S=0x00181050;//DS Spatial​Resolution
const u32 L00181061S=0x00181061;//LO Trigger​Source​Or​Type
const u32 L00181064S=0x00181064;//LO Cardiac​Framing​Type
const u32 L0018106AS=0x0018106A;//CS Synchronization​Trigger
const u32 L0018106CS=0x0018106C;//US Synchronization​Channel
const u32 L00181080S=0x00181080;//CS Beat​Rejection​Flag
const u32 L00181085S=0x00181085;//LO PVC​Rejection
const u32 L00181086S=0x00181086;//IS Skip​Beats
const u32 L00181088S=0x00181088;//IS Heart​Rate
const u32 L00181100S=0x00181100;//DS Reconstruction​Diameter
const u32 L00181120S=0x00181120;//DS Gantry​Detector​Tilt
const u32 L00181121S=0x00181121;//DS Gantry​Detector​Slew
const u32 L00181147S=0x00181147;//CS Field​Of​View​Shape
const u32 L00181149S=0x00181149;//IS Field​Of​View​Dimensions
const u32 L00181160S=0x00181160;//SH Filter​Type
const u32 L00181180S=0x00181180;//SH Collimator​Grid​Name
const u32 L00181181S=0x00181181;//CS Collimator​Type
const u32 L00181190S=0x00181190;//DS Focal​Spots
const u32 L00181200S=0x00181200;//DA Date​Of​Last​Calibration
const u32 L00181201S=0x00181201;//TM Time​Of​Last​Calibration
const u32 L00181204S=0x00181204;//DA Date​Of​Manufacture
const u32 L00181205S=0x00181205;//DA Date​Of​Installation
const u32 L00181210S=0x00181210;//SH Convolution​Kernel
const u32 L00181260S=0x00181260;//SH Plate​Type
const u32 L00181261S=0x00181261;//LO Phosphor​Type
const u32 L00181800S=0x00181800;//CS Acquisition​Time​Synchronized
const u32 L00181801S=0x00181801;//SH Time​Source
const u32 L00181802S=0x00181802;//CS Time​Distribution​Protocol
const u32 L00181803S=0x00181803;//LO NTP​Source​Address
const u32 L00185100S=0x00185100;//CS Patient​Position
const u32 L00185101S=0x00185101;//CS View​Position
const u32 L0020000ES=0x0020000E;//UI Series​Instance​UID
const u32 L00200011S=0x00200011;//IS Series​Number
const u32 L00200052S=0x00200052;//UI Frame​Of​Reference​UID
const u32 L00200060S=0x00200060;//CS Laterality
const u32 L00200200S=0x00200200;//UI Synchronization​Frame​Of​Reference​UID
const u32 L00201040S=0x00201040;//LO Position​Reference​Indicator
const u32 L00209307S=0x00209307;//CS Ultrasound​Acquisition​Geometry
const u32 L00209308S=0x00209308;//FD Apex​Position
const u32 L00209309S=0x00209309;//FD Volume​To​Transducer​Mapping​Matrix
const u32 L0020930AS=0x0020930A;//FD Volume​To​Table​Mapping​Matrix
const u32 L0020930BS=0x0020930B;//CS Volume​To​Transducer​Relationship
const u32 L0020930CS=0x0020930C;//CS Patient​Frame​Of​Reference​Source
const u32 L00209312S=0x00209312;//UI Volume​Frame​Of​Reference​UID
const u32 L00209313S=0x00209313;//UI Table​Frame​Of​Reference​UID
const u32 L00280051S=0x00280051;//CS Corrected​Image
const u32 L00280108S=0x00280108;//US or SS Smallest​Pixel​Value​In​Series
const u32 L00280109S=0x00280109;//US or SS Largest​Pixel​Value​In​Series
const u32 L00280120S=0x00280120;//US or SS Pixel​Padding​Value
const u32 L00400244S=0x00400244;//DA Performed​Procedure​Step​Start​Date
const u32 L00400245S=0x00400245;//TM Performed​Procedure​Step​Start​Time
const u32 L00400250S=0x00400250;//DA Performed​Procedure​Step​End​Date
const u32 L00400251S=0x00400251;//TM Performed​Procedure​Step​End​Time
const u32 L00400253S=0x00400253;//SH Performed​Procedure​Step​ID
const u32 L00400254S=0x00400254;//LO Performed​Procedure​Step​Description
const u32 L00400260S=0x00400260;//SQ Performed​Protocol​Code​Sequence
const u32 L00400261S=0x00400261;//CS Performed​Protocol​Type
const u32 L00400275S=0x00400275;//SQ Request​Attributes​Sequence
const u32 L00400280S=0x00400280;//ST Comments​On​The​Performed​Procedure​Step
*/

#pragma mark base dataset series modality specific

const u32 B00540013X=0x13005400;//SQ Energy​Window​Range​Sequence
const u32 B00540014X=0x14005400;//DS Energy​Window​Lower​Limit
const u32 B00540015X=0x15005400;//DS Energy​Window​Upper​Limit
const u32 B00540016X=0x16005400;//SQ Radio​pharmaceutical​Information​Sequence
const u32 B00540061X=0x61005400;//US Number​Of​RR​Intervals
const u32 B00540071X=0x71005400;//US Number​Of​Time​Slots
const u32 B00540081X=0x81005400;//US Number​Of​Slices
const u32 B00540101X=0x01015400;//US Number​Of​Time​Slices
const u32 B00540202X=0x02025400;//CS Type​Of​Detector​Motion
const u32 B00540410X=0x10045400;//SQ Patient​Orientation​Code​Sequence
const u32 B00540414X=0x14045400;//SQ Patient​Gantry​Relationship​Code​Sequence
const u32 B00540501X=0x01055400;//CS Scan​Progression​Direction
const u32 B00541000X=0x00105400;//CS Series​Type
const u32 B00541001X=0x01105400;//CS Units
const u32 B00541002X=0x02105400;//CS Counts​Source
const u32 B00541004X=0x04105400;//CS Reprojection​Method
const u32 B00541006X=0x06105400;//CS SUV​Type
const u32 B00541100X=0x00115400;//CS Randoms​Correction​Method
const u32 B00541101X=0x01115400;//LO Attenuation​Correction​Method
const u32 B00541102X=0x02115400;//CS Decay​Correction
const u32 B00541103X=0x03115400;//LO Reconstruction​Method
const u32 B00541104X=0x04115400;//LO Detector​Lines​Of​Response​Used
const u32 B00541105X=0x05115400;//LO Scatter​Correction​Method
const u32 B00541200X=0x00125400;//DS Axial​Acceptance
const u32 B00541201X=0x01125400;//IS Axial​Mash
const u32 B00541202X=0x02125400;//IS Transverse​Mash
const u32 B00541203X=0x03125400;//DS Detector​Element​Size
const u32 B00541210X=0x10125400;//DS Coincidence​Window​Width
const u32 B00541220X=0x20125400;//CS Secondary​Counts​Type
const u32 B00800013X=0x13008000;//SQ Referenced​Surface​Data​Sequence
const u32 B300A0700X=0x00070A30;//UI Treatment​Session​UID
/*
const u32 L00540013X=0x00540013;//SQ Energy​Window​Range​Sequence
const u32 L00540014X=0x00540014;//DS Energy​Window​Lower​Limit
const u32 L00540015X=0x00540015;//DS Energy​Window​Upper​Limit
const u32 L00540016X=0x00540016;//SQ Radio​pharmaceutical​Information​Sequence
const u32 L00540061X=0x00540061;//US Number​Of​RR​Intervals
const u32 L00540071X=0x00540071;//US Number​Of​Time​Slots
const u32 L00540081X=0x00540081;//US Number​Of​Slices
const u32 L00540101X=0x00540101;//US Number​Of​Time​Slices
const u32 L00540202X=0x00540202;//CS Type​Of​Detector​Motion
const u32 L00540410X=0x00540410;//SQ Patient​Orientation​Code​Sequence
const u32 L00540414X=0x00540414;//SQ Patient​Gantry​Relationship​Code​Sequence
const u32 L00540501X=0x00540501;//CS Scan​Progression​Direction
const u32 L00541000X=0x00541000;//CS Series​Type
const u32 L00541001X=0x00541001;//CS Units
const u32 L00541002X=0x00541002;//CS Counts​Source
const u32 L00541004X=0x00541004;//CS Reprojection​Method
const u32 L00541006X=0x00541006;//CS SUV​Type
const u32 L00541100X=0x00541100;//CS Randoms​Correction​Method
const u32 L00541101X=0x00541101;//LO Attenuation​Correction​Method
const u32 L00541102X=0x00541102;//CS Decay​Correction
const u32 L00541103X=0x00541103;//LO Reconstruction​Method
const u32 L00541104X=0x00541104;//LO Detector​Lines​Of​Response​Used
const u32 L00541105X=0x00541105;//LO Scatter​Correction​Method
const u32 L00541200X=0x00541200;//DS Axial​Acceptance
const u32 L00541201X=0x00541201;//IS Axial​Mash
const u32 L00541202X=0x00541202;//IS Transverse​Mash
const u32 L00541203X=0x00541203;//DS Detector​Element​Size
const u32 L00541210X=0x00541210;//DS Coincidence​Window​Width
const u32 L00541220X=0x20125400;//CS Secondary​Counts​Type
const u32 L00800013X=0x00800013;//SQ Referenced​Surface​Data​Sequence
const u32 L300A0700X=0x300A0700;//UI Treatment​Session​UID
*/
#pragma mark Opendicom CDA series instead of instance
const u32 B0040E001X=0x01E04000;//ST HL7InstanceIdentifier 0040E001  root^extension
const u32 B00420010X=0x10004200;//ST DocumentTitel
const u32 B00420011X=0x11004200;//OB Encapsulated​Document
const u32 B00420012X=0x12004200;//LO mime type
/*
const u32 L0040E001X=0x0040E001;//ST HL7InstanceIdentifier 0040E001  root^extension
const u32 L00420010X=0x00420010;//ST DocumentTitel
const u32 L00420011X=0x00420011;//OB Encapsulated​Document
const u32 L00420012X=0x00420012;//LO mime type
*/


//attribute type study 00 series 01 (~=level)
const u8 P=0x00;//patient level
const u8 C=0x00;//clinical study level
const u8 S=0x01;//series level
const u8 X=0x01;//special series level
//Patient Clinical(Study) and Series tags and parallel array with category (PCSX)
const u32 PCStag[]={
   B00080020E,//DA Study​Date
   B00080021S,//DA Series​Date
   B00080030E,//TM Study​Time
   B00080031S,//TM Series​Time
   B00080050E,//SH Accession​Number
   B00080051E,//SQ Issuer​Of​Accession​Number​Sequence
   B00080060S,//CS Modality
   B00080064S,//CS Conversion​Type
   B00080068S,//CS Presentation​Intent​Type
   B00080070S,//LO Manufacturer
   B00080080S,//LO Institution​Name (standard is series S, moved it to Exam E)
   B00080081S,//ST Institution​Address
   B00080090E,//PN Referring​Physician​Name
   B00080096E,//SQ Referring​Physician​Identification​Sequence
   B0008009CE,//PN Consulting​Physician​Name
   B0008009DE,//SQ Consulting​Physician​Identification​Sequence
   B00081010S,//SH Station​Name
   B00081030E,//LO Study​Description
   B00081032E,//SQ Procedure​Code​Sequence
   B0008103ES,//LO Series​Description
   B0008103FS,//SQ Series​Description​Code​Sequence
   B00081040S,//LO Institutional​Department​Name
   B00081041S,//SQ Institutional​Department​Type​Code​Sequence
   B00081048E,//PN Physicians​Of​Record
   B00081049E,//SQ Physicians​Of​Record​Identification​Sequence
   B00081050S,//PN Performing​Physician​Name
   B00081052S,//SQ Performing​Physician​Identification​Sequence
   B00081060E,//PN Name​Of​Physicians​Reading​Study
   B00081062E,//SQ Physicians​Reading​Study​Identification​Sequence
   B00081070S,//PN Operators​Name
   B00081072S,//SQ Operator​Identification​Sequence
   B00081080E,//LO Admitting​Diagnoses​Description
   B00081084E,//SQ Admitting​Diagnoses​Code​Sequence
   B00081090S,//LO Manufacturer​Model​Name
   B00081110E,//SQ Referenced​Study​Sequence
   B00081111S,//SQ Referenced​Performed​Procedure​Step​Sequence
   B00081120P,//SQ Referenced Patient Sequence
   B00081250S,//SQ Related​Series​Sequence
   B00100010P,//PN Patient​Name
   B00100020P,//LO Patient​ID
   B00100021P,//LO Issuer​Of​Patient​ID
   B00100022P,//CS Type​Of​Patient​ID
   B00100024P,//SQ Issuer​Of​Patient​ID​Qualifiers​Sequence
   B00100026P,//SQ Source​Patient​Group​Identification​Sequence
   B00100027P,//SQ Group​Of​Patients​Identification​Sequence
   B00100030P,//DA Patient​Birth​Date
   B00100032P,//TM Patient​Birth​Time
   B00100033P,//LO Patient​Birth​Date​In​Alternative​Calendar
   B00100034P,//LO Patient​Death​Date​In​Alternative​Calendar
   B00100035P,//CS Patient​Alternative​Calendar
   B00100040P,//CS Patient​Sex
   B00100200P,//CS Quality​Control​Subject
   B00100212P,//UC Strain​Description
   B00100213P,//LO Strain​Nomenclature
   B00100216P,//SQ Strain​Stock​Sequence
   B00100218P,//UT Strain​Additional​Information
   B00100219P,//SQ Strain​Code​Sequence
   B00100221P,//SQ Genetic​Modifications​​Sequence
   B00101000P,//LO Other​Patient​IDs
   B00101001P,//PN Other​Patient​Names
   B00101002P,//SQ Other​Patient​IDs​Sequence
   B00101010E,//AS Patient​Age
   B00101020E,//DS Patient​Size
   B00101021E,//SQ Patient​Size​Code​Sequence
   B00101022E,//DS Patient​Body​Mass​Index
   B00101023E,//DS Measured​APDimension
   B00101024E,//DS Measured​Lateral​Dimension
   B00101030E,//DS Patient​Weight
   B00101100P,//SQ Referenced​Patient​Photo​Sequence
   B00102000E,//LO Medical​Alerts
   B00102110E,//LO Allergies
   B00102160P,//SH Ethnic​Group
   B00102180E,//SH Occupation
   B001021A0E,//CS Smoking​Status
   B001021B0E,//LT Additional​Patient​History
   B001021C0E,//US Pregnancy​Status
   B001021D0E,//DA Last​Menstrual​Date
   B00102201P,//LO Patient​Species​Description
   B00102202P,//SQ Patient​Species​Code​Sequence
   B00102203E,//CS Patient​Sex​Neutered
   B00102292P,//LO Patient​Breed​Description
   B00102293P,//SQ Patient​Breed​Code​Sequence
   B00102294P,//SQ Breed​Registration​Sequence
   B00102297P,//PN Responsible​Person
   B00102298P,//CS Responsible​Person​Role
   B00102299P,//LO Responsible​Organization
   B00104000P,//LT Patient​Comments
   B00120010P,//LO Clinical​Trial​Sponsor​Name
   B00120020P,//LO Clinical​Trial​Protocol​ID
   B00120021P,//LO Clinical​Trial​Protocol​Name
   B00120030P,//LO Clinical​Trial​Site​ID
   B00120031P,//LO Clinical​Trial​Site​Name
   B00120040P,//LO Clinical​Trial​Subject​ID
   B00120042P,//LO Clinical​Trial​Subject​Reading​ID
   B00120050E,//LO Clinical​Trial​Time​Point​ID
   B00120051E,//ST Clinical​Trial​Time​Point​Description
   B00120052E,//FD Longitudinal​Temporal​Offset​From​Event
   B00120053E,//CS Longitudinal​Temporal​Event​Type
   B00120054E,//SQ Clinical​Trial​Time​Point​Type​Code​Sequence
   B00120060S,//LO Clinical​Trial​Coordinating​Center​Name
   B00120062P,//CS Patient​Identity​Removed
   B00120063P,//LO Deidentification​Method
   B00120064P,//SQ Deidentification​Method​Code​Sequence
   B00120071S,//LO Clinical​Trial​Series​ID
   B00120072S,//LO Clinical​Trial​Series​Description
   B00120081P,//LO Clinical​Trial​Protocol​Ethics​Committee​Name
   B00120082P,//LO Clinical​Trial​Protocol​Ethics​Committee​Approval​Number
   B00120083E,//SQ Consent​For​Clinical​Trial​Use​Sequence
   B0016004DS,//UT Camera​Owner​Name
   B0016004ES,//DS Lens​Specification
   B0016004FS,//UT Lens​Make
   B00160050S,//UT Lens​Model
   B00160051S,//UT Lens​Serial​Number
   B00180015S,//CS Body​Part​Examined
   B00180026S,//SQ Intervention​Drug​Information​Sequence
   B00180071S,//CS Acquisition​Termination​Condition
   B00180073S,//CS Acquisition​Start​Condition
   B00180074S,//IS Acquisition​Start​Condition​Data
   B00180075S,//IS Acquisition​Termination​Condition​Data
   B00181000S,//LO Device​Serial​Number
   B00181002S,//UI Device​UID
   B00181008S,//LO Gantry​ID
   B0018100AS,//SQ UDI​Sequence
   B0018100BS,//UI Manufacturer​Device​Class​UID
   B00181010S,//LO Secondary​Capture​Device​ID
   B00181016S,//LO Secondary​Capture​Device​Manufacturer
   B00181018S,//LO Secondary​Capture​Device​Manufacturer​Model​Name
   B00181019S,//LO Secondary​Capture​Device​Software​Versions
   B00181020S,//LO Software​Versions
   B00181022S,//SH Video​Image​Format​Acquired
   B00181023S,//LO Digital​Image​Format​Acquired
   B00181030S,//LO Protocol​Name
   B00181050S,//DS Spatial​Resolution
   B00181061S,//LO Trigger​Source​Or​Type
   B00181064S,//LO Cardiac​Framing​Type
   B0018106AS,//CS Synchronization​Trigger
   B0018106CS,//US Synchronization​Channel
   B00181080S,//CS Beat​Rejection​Flag
   B00181085S,//LO PVC​Rejection
   B00181086S,//IS Skip​Beats
   B00181088S,//IS Heart​Rate
   B00181100S,//DS Reconstruction​Diameter
   B00181120S,//DS Gantry​Detector​Tilt
   B00181121S,//DS Gantry​Detector​Slew
   B00181147S,//CS Field​Of​View​Shape
   B00181149S,//IS Field​Of​View​Dimensions
   B00181160S,//SH Filter​Type
   B00181180S,//SH Collimator​Grid​Name
   B00181181S,//CS Collimator​Type
   B00181190S,//DS Focal​Spots
   B00181200S,//DA Date​Of​Last​Calibration
   B00181201S,//TM Time​Of​Last​Calibration
   B00181204S,//DA Date​Of​Manufacture
   B00181205S,//DA Date​Of​Installation
   B00181210S,//SH Convolution​Kernel
   B00181260S,//SH Plate​Type
   B00181261S,//LO Phosphor​Type
   B00181800S,//CS Acquisition​Time​Synchronized
   B00181801S,//SH Time​Source
   B00181802S,//CS Time​Distribution​Protocol
   B00181803S,//LO NTP​Source​Address
   B00185100S,//CS Patient​Position
   B00185101S,//CS View​Position
   B0020000DE,//UI Study​Instance​UID
   B0020000ES,//UI Series​Instance​UID
   B00200010E,//SH Study​ID
   B00200011S,//IS Series​Number
   B00200052S,//UI Frame​Of​Reference​UID
   B00200060S,//CS Laterality
   B00200200S,//UI Synchronization​Frame​Of​Reference​UID
   B00201040S,//LO Position​Reference​Indicator
   B00209307S,//CS Ultrasound​Acquisition​Geometry
   B00209308S,//FD Apex​Position
   B00209309S,//FD Volume​To​Transducer​Mapping​Matrix
   B0020930AS,//FD Volume​To​Table​Mapping​Matrix
   B0020930BS,//CS Volume​To​Transducer​Relationship
   B0020930CS,//CS Patient​Frame​Of​Reference​Source
   B00209312S,//UI Volume​Frame​Of​Reference​UID
   B00209313S,//UI Table​Frame​Of​Reference​UID
   B00280051S,//CS Corrected​Image
   B00280108S,//US or SS Smallest​Pixel​Value​In​Series
   B00280109S,//US or SS Largest​Pixel​Value​In​Series
   B00280120S,//US or SS Pixel​Padding​Value
   B00321033E,//LO Requesting​Service
   B00321034E,//SQ Requesting​Service​Code​Sequence
   B00321066E,//UT Reason​For​Visit
   B00321067E,//SQ Reason​For​Visit​Code​Sequence
   B00380010E,//LO Admission​ID
   B00380014E,//SQ Issuer​Of​Admission​ID​Sequence
   B00380060E,//LO Service​Episode​ID
   B00380062E,//LO Service​Episode​Description
   B00380064E,//SQ Issuer​Of​Service​Episode​ID​Sequence
   B00385000E,//LO Patient​State
   B00400244S,//DA Performed​Procedure​Step​Start​Date
   B00400245S,//TM Performed​Procedure​Step​Start​Time
   B00400250S,//DA Performed​Procedure​Step​End​Date
   B00400251S,//TM Performed​Procedure​Step​End​Time
   B00400253S,//SH Performed​Procedure​Step​ID
   B00400254S,//LO Performed​Procedure​Step​Description
   B00400260S,//SQ Performed​Protocol​Code​Sequence
   B00400261S,//CS Performed​Protocol​Type
   B00400275S,//SQ Request​Attributes​Sequence
   B00400280S,//ST Comments​On​The​Performed​Procedure​Step
   B00401012E,//SQ Reason​For​Performed​Procedure​Code​Sequence

   B0040E001X,//ST HL7InstanceIdentifier 0040E001  root^extension
   B00420010X,//ST DocumentTitel
   B00420011X,//OB Encapsulated​Document
   B00420012X,//LO Mime type

   B00540013X,//SQ Energy​Window​Range​Sequence
   B00540014X,//DS Energy​Window​Lower​Limit
   B00540015X,//DS Energy​Window​Upper​Limit
   B00540016X,//SQ Radio​pharmaceutical​Information​Sequence
   B00540061X,//US Number​Of​RR​Intervals
   B00540071X,//US Number​Of​Time​Slots
   B00540081X,//US Number​Of​Slices
   B00540101X,//US Number​Of​Time​Slices
   B00540202X,//CS Type​Of​Detector​Motion
   B00540410X,//SQ Patient​Orientation​Code​Sequence
   B00540414X,//SQ Patient​Gantry​Relationship​Code​Sequence
   B00540501X,//CS Scan​Progression​Direction
   B00541000X,//CS Series​Type
   B00541001X,//CS Units
   B00541002X,//CS Counts​Source
   B00541004X,//CS Reprojection​Method
   B00541006X,//CS SUV​Type
   B00541100X,//CS Randoms​Correction​Method
   B00541101X,//LO Attenuation​Correction​Method
   B00541102X,//CS Decay​Correction
   B00541103X,//LO Reconstruction​Method
   B00541104X,//LO Detector​Lines​Of​Response​Used
   B00541105X,//LO Scatter​Correction​Method
   B00541200X,//DS Axial​Acceptance
   B00541201X,//IS Axial​Mash
   B00541202X,//IS Transverse​Mash
   B00541203X,//DS Detector​Element​Size
   B00541210X,//DS Coincidence​Window​Width
   B00541220X,//CS Secondary​Counts​Type
   B00800013X,//SQ Referenced​Surface​Data​Sequence
   B300A0700X //UI Treatment​Session​UID
};
const u8 PCStype[]={
   C,//DA Study​Date
   S,//DA Series​Date
   C,//TM Study​Time
   S,//TM Series​Time
   C,//SH Accession​Number
   C,//SQ Issuer​Of​Accession​Number​Sequence
   S,//CS Modality
   S,//CS Conversion​Type
   S,//CS Presentation​Intent​Type
   S,//LO Manufacturer
   C,//LO Institution​Name  (is S in the standard but we put it in C)
   S,//ST Institution​Address
   C,//PN Referring​Physician​Name
   C,//SQ Referring​Physician​Identification​Sequence
   C,//PN Consulting​Physician​Name
   C,//SQ Consulting​Physician​Identification​Sequence
   S,//SH Station​Name
   C,//LO Study​Description
   C,//SQ Procedure​Code​Sequence
   S,//LO Series​Description
   S,//SQ Series​Description​Code​Sequence
   S,//LO Institutional​Department​Name
   S,//SQ Institutional​Department​Type​Code​Sequence
   C,//PN Physicians​Of​Record
   C,//SQ Physicians​Of​Record​Identification​Sequence
   S,//PN Performing​Physician​Name
   S,//SQ Performing​Physician​Identification​Sequence
   C,//PN Name​Of​Physicians​Reading​Study
   C,//SQ Physicians​Reading​Study​Identification​Sequence
   S,//PN Operators​Name
   S,//SQ Operator​Identification​Sequence
   C,//LO Admitting​Diagnoses​Description
   C,//SQ Admitting​Diagnoses​Code​Sequence
   S,//LO Manufacturer​Model​Name
   C,//SQ Referenced​Study​Sequence
   S,//SQ Referenced​Performed​Procedure​Step​Sequence
   P,//SQ Referenced Patient Sequence
   S,//SQ Related​Series​Sequence
   P,//PN Patient​Name
   P,//LO Patient​ID
   P,//LO Issuer​Of​Patient​ID
   P,//CS Type​Of​Patient​ID
   P,//SQ Issuer​Of​Patient​ID​Qualifiers​Sequence
   P,//SQ Source​Patient​Group​Identification​Sequence
   P,//SQ Group​Of​Patients​Identification​Sequence
   P,//DA Patient​Birth​Date
   P,//TM Patient​Birth​Time
   P,//LO Patient​Birth​Date​In​Alternative​Calendar
   P,//LO Patient​Death​Date​In​Alternative​Calendar
   P,//CS Patient​Alternative​Calendar
   P,//CS Patient​Sex
   P,//CS Quality​Control​Subject
   P,//UC Strain​Description
   P,//LO Strain​Nomenclature
   P,//SQ Strain​Stock​Sequence
   P,//UT Strain​Additional​Information
   P,//SQ Strain​Code​Sequence
   P,//SQ Genetic​Modifications​​Sequence
   P,//LO Other​Patient​IDs
   P,//PN Other​Patient​Names
   P,//SQ Other​Patient​IDs​Sequence
   C,//AS Patient​Age
   C,//DS Patient​Size
   C,//SQ Patient​Size​Code​Sequence
   C,//DS Patient​Body​Mass​Index
   C,//DS Measured​APDimension
   C,//DS Measured​Lateral​Dimension
   C,//DS Patient​Weight
   P,//SQ Referenced​Patient​Photo​Sequence
   C,//LO Medical​Alerts
   C,//LO Allergies
   P,//SH Ethnic​Group
   C,//SH Occupation
   C,//CS Smoking​Status
   C,//LT Additional​Patient​History
   C,//US Pregnancy​Status
   C,//DA Last​Menstrual​Date
   P,//LO Patient​Species​Description
   P,//SQ Patient​Species​Code​Sequence
   C,//CS Patient​Sex​Neutered
   P,//LO Patient​Breed​Description
   P,//SQ Patient​Breed​Code​Sequence
   P,//SQ Breed​Registration​Sequence
   P,//PN Responsible​Person
   P,//CS Responsible​Person​Role
   P,//LO Responsible​Organization
   P,//LT Patient​Comments
   P,//LO Clinical​Trial​Sponsor​Name
   P,//LO Clinical​Trial​Protocol​ID
   P,//LO Clinical​Trial​Protocol​Name
   P,//LO Clinical​Trial​Site​ID
   P,//LO Clinical​Trial​Site​Name
   P,//LO Clinical​Trial​Subject​ID
   P,//LO Clinical​Trial​Subject​Reading​ID
   C,//LO Clinical​Trial​Time​Point​ID
   C,//ST Clinical​Trial​Time​Point​Description
   C,//FD Longitudinal​Temporal​Offset​From​Event
   C,//CS Longitudinal​Temporal​Event​Type
   C,//SQ Clinical​Trial​Time​Point​Type​Code​Sequence
   S,//LO Clinical​Trial​Coordinating​Center​Name
   P,//CS Patient​Identity​Removed
   P,//LO Deidentification​Method
   P,//SQ Deidentification​Method​Code​Sequence
   S,//LO Clinical​Trial​Series​ID
   S,//LO Clinical​Trial​Series​Description
   P,//LO Clinical​Trial​Protocol​Ethics​Committee​Name
   P,//LO Clinical​Trial​Protocol​Ethics​Committee​Approval​Number
   C,//SQ Consent​For​Clinical​Trial​Use​Sequence
   S,//UT Camera​Owner​Name
   S,//DS Lens​Specification
   S,//UT Lens​Make
   S,//UT Lens​Model
   S,//UT Lens​Serial​Number
   S,//CS Body​Part​Examined
   S,//SQ Intervention​Drug​Information​Sequence
   S,//CS Acquisition​Termination​Condition
   S,//CS Acquisition​Start​Condition
   S,//IS Acquisition​Start​Condition​Data
   S,//IS Acquisition​Termination​Condition​Data
   S,//LO Device​Serial​Number
   S,//UI Device​UID
   S,//LO Gantry​ID
   S,//SQ UDI​Sequence
   S,//UI Manufacturer​Device​Class​UID
   S,//LO Secondary​Capture​Device​ID
   S,//LO Secondary​Capture​Device​Manufacturer
   S,//LO Secondary​Capture​Device​Manufacturer​Model​Name
   S,//LO Secondary​Capture​Device​Software​Versions
   S,//LO Software​Versions
   S,//SH Video​Image​Format​Acquired
   S,//LO Digital​Image​Format​Acquired
   S,//LO Protocol​Name
   S,//DS Spatial​Resolution
   S,//LO Trigger​Source​Or​Type
   S,//LO Cardiac​Framing​Type
   S,//CS Synchronization​Trigger
   S,//US Synchronization​Channel
   S,//CS Beat​Rejection​Flag
   S,//LO PVC​Rejection
   S,//IS Skip​Beats
   S,//IS Heart​Rate
   S,//DS Reconstruction​Diameter
   S,//DS Gantry​Detector​Tilt
   S,//DS Gantry​Detector​Slew
   S,//CS Field​Of​View​Shape
   S,//IS Field​Of​View​Dimensions
   S,//SH Filter​Type
   S,//SH Collimator​Grid​Name
   S,//CS Collimator​Type
   S,//DS Focal​Spots
   S,//DA Date​Of​Last​Calibration
   S,//TM Time​Of​Last​Calibration
   S,//DA Date​Of​Manufacture
   S,//DA Date​Of​Installation
   S,//SH Convolution​Kernel
   S,//SH Plate​Type
   S,//LO Phosphor​Type
   S,//CS Acquisition​Time​Synchronized
   S,//SH Time​Source
   S,//CS Time​Distribution​Protocol
   S,//LO NTP​Source​Address
   S,//CS Patient​Position
   S,//CS View​Position
   C,//UI Study​Instance​UID
   S,//UI Series​Instance​UID
   C,//SH Study​ID
   S,//IS Series​Number
   S,//UI Frame​Of​Reference​UID
   S,//CS Laterality
   S,//UI Synchronization​Frame​Of​Reference​UID
   S,//LO Position​Reference​Indicator
   S,//CS Ultrasound​Acquisition​Geometry
   S,//FD Apex​Position
   S,//FD Volume​To​Transducer​Mapping​Matrix
   S,//FD Volume​To​Table​Mapping​Matrix
   S,//CS Volume​To​Transducer​Relationship
   S,//CS Patient​Frame​Of​Reference​Source
   S,//UI Volume​Frame​Of​Reference​UID
   S,//UI Table​Frame​Of​Reference​UID
   S,//CS Corrected​Image
   S,//US or SS Smallest​Pixel​Value​In​Series
   S,//US or SS Largest​Pixel​Value​In​Series
   S,//US or SS Pixel​Padding​Value
   C,//LO Requesting​Service
   C,//SQ Requesting​Service​Code​Sequence
   C,//UT Reason​For​Visit
   C,//SQ Reason​For​Visit​Code​Sequence
   C,//LO Admission​ID
   C,//SQ Issuer​Of​Admission​ID​Sequence
   C,//LO Service​Episode​ID
   C,//LO Service​Episode​Description
   C,//SQ Issuer​Of​Service​Episode​ID​Sequence
   C,//LO Patient​State
   S,//DA Performed​Procedure​Step​Start​Date
   S,//TM Performed​Procedure​Step​Start​Time
   S,//DA Performed​Procedure​Step​End​Date
   S,//TM Performed​Procedure​Step​End​Time
   S,//SH Performed​Procedure​Step​ID
   S,//LO Performed​Procedure​Step​Description
   S,//SQ Performed​Protocol​Code​Sequence
   S,//CS Performed​Protocol​Type
   S,//SQ Request​Attributes​Sequence
   S,//ST Comments​On​The​Performed​Procedure​Step
   C,//SQ Reason​For​Performed​Procedure​Code​Sequence
   
   X,//ST HL7InstanceIdentifier 0040E001  root^extension
   X,//ST DocumentTitel
   X,//OB Encapsulated​Document
   X,//LO Mime type
   
   X,//SQ Energy​Window​Range​Sequence
   X,//DS Energy​Window​Lower​Limit
   X,//DS Energy​Window​Upper​Limit
   X,//SQ Radio​pharmaceutical​Information​Sequence
   X,//US Number​Of​RR​Intervals
   X,//US Number​Of​Time​Slots
   X,//US Number​Of​Slices
   X,//US Number​Of​Time​Slices
   X,//CS Type​Of​Detector​Motion
   X,//SQ Patient​Orientation​Code​Sequence
   X,//SQ Patient​Gantry​Relationship​Code​Sequence
   X,//CS Scan​Progression​Direction
   X,//CS Series​Type
   X,//CS Units
   X,//CS Counts​Source
   X,//CS Reprojection​Method
   X,//CS SUV​Type
   X,//CS Randoms​Correction​Method
   X,//LO Attenuation​Correction​Method
   X,//CS Decay​Correction
   X,//LO Reconstruction​Method
   X,//LO Detector​Lines​Of​Response​Used
   X,//LO Scatter​Correction​Method
   X,//DS Axial​Acceptance
   X,//IS Axial​Mash
   X,//IS Transverse​Mash
   X,//DS Detector​Element​Size
   X,//DS Coincidence​Window​Width
   X,//CS Secondary​Counts​Type
   X,//SQ Referenced​Surface​Data​Sequence
   X //UI Treatment​Session​UID
};

extern FILE * inFile;

extern char *DICM;
extern u64   DICMidx;
extern u64   DICMsize;

extern u8   *CKEY;

extern u8    CKEYidx;
static char *UTF8;
u32 utf8size=0;
FILE *KVserializedFILE;
const u32 CSutf8size=10;

static u16 PCSidx;




#pragma mark ---------------------------- SOP instance

void input( int argc, char *argv[])
{
   inFile = freopen(argv[1],"rb",stdin);
   if (inFile==NULL)
   {
      if (ferror(stdin)) {
         fprintf(stderr,"inFile rb %s : %s (%d)\n", argv[1], strerror(errno), errno);
         exit(errno);
      }
      exit(exitErrorFropenDICM);
   }

   DICM=malloc(DICMsize);
   if (DICMsize!=fread(DICM,1,DICMsize,stdin)) {
      if (ferror(stdin)) {
         fprintf(stderr,"uCreate [%lu] %s (%d)\n", DICMidx, strerror(errno), errno);
         exit(errno);
      }
      fprintf(stderr,"uCreate [%lu] read %lu bytes truncated (%d)\n", DICMidx, DICMsize, exitReadTruncated);
      exit(exitReadTruncated);
   };
   fclose(inFile);

   KVserializedFILE = fopen("serialized.bin", "w");

   setlocale(LC_ALL, "");//output in UTF-8
   UTF8=malloc(0x4000);
   //LT max 10240,UT max 2^32 !!!
   //16K covers any UTF8 size increase for LT, but eventually requires larger buffer  for LT
}

void trail(int count, char *vector[]) {
   fclose(KVserializedFILE);
}


#pragma mark ---------------------------- attributes


void key(struct Ercle* attr)
{
   //reads at CKEYidx+8 the new 8 first bytes of the attribute
   //reorders group and unit at CKEYidx
   // copies the VR and empty bytes at CKEYidx+4
   // eventually reads the four additional bytes of long length at CKEYidx+8
   memcpy(attr,DICM+DICMidx,8);
   DICMidx+=8;
   attr->E=((attr->E & 0xFF00FF00) >> 8)|((attr->E & 0xFF00FF) << 8); //transform to pure big endian
   attr->e=u32swap(attr->E);//tag in little endian

   //attr->r is ascii (left to right)
   attr->l=attr->c;//short length is transferred to long length (bytes 8-11)
   //attr->c short length will be modified before parsing the value to register repositoire index
   switch (attr->r) {
      case OB://other byte
      case OW://other word
      case OD://other double
      case OF://other float
      case OL://other long
      case SV://signed 64-bit very long
      case OV://other 64-bit very long
      case UV://unsigned 64-bit very long
      case UC://unlimited characters
      case UT://unlimited text
      case UR://universal resource url identifier/locator
      case SQ://sequence
      {
         memcpy(&attr->l,DICM+DICMidx,4);
         DICMidx+=4;
      }break;
      default:break;
   }
}

void val(enum kvVRcategory vrcat,struct Ercle* attr) {

   if ((attr->e & 0xFFFF)==0) { //remove group length
      DICMidx+=attr->l;
      return;
   }

   u32  *rootTag = (u32*)(CKEY+1);
   if (*rootTag & 0x000003){ //group even private or group 2
      pAttribute(vrcat,attr);
      return;
   }

   switch (vrcat) {
      case kvUN: { pAttribute(vrcat,attr); } break;

#pragma mark - ignore useless sequence and item tags
      case kvIA:
      case kvSA:
      case kvSZ:
      case kvIZ:
      case kvIa:
      case kvSa:
      case kvSz:
      case kvIz:
         break;

      default: {
         //PCSidx: index of next little endian tag in PCStag table (patient, clinical study, series)
         //if current tag is lower than PCStag[PCSidx], current tag is instance or frame tag

         if (memcmp(rootTag, &PCStag[PCSidx], 4) < 0) //rootTag smaller than index
         {
            iAttribute(vrcat,attr);
         }
         else
         {
            while ((memcmp(rootTag, &PCStag[PCSidx], 4) > 0) && (PCSidx < 234)) (PCSidx)++;
            if (memcmp(rootTag, &PCStag[PCSidx], 4)==0)
            {
               if (PCStype[PCSidx]==0) eAttribute(vrcat,attr);
               else                    sAttribute(vrcat,attr);
               PCSidx++;
            }
            else iAttribute(vrcat,attr);
         }
      }
   }
}


