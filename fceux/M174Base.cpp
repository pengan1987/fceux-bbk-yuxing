#include "M174Base.h"

bool ScanVCD() {
    std::vector<std::string> vectorPath;
    pTools->ScanFolder("X:\\TEMP\\科王KW-SC2000随机软盘\\科王1-5", vectorPath);
    for (int i = vectorPath.size() - 1; i >= 0; i--) {
        auto strFile = vectorPath[i];
        auto f = fopen(strFile.c_str(), "rb");
        if (!f) {
            continue;
        }
        fseek(f, 0, SEEK_END);
        auto s = ftell(f);
        fseek(f, 0, SEEK_SET);
        const char* pszFcGames = "FC GAMES";
        BYTE buf[0x50];
        fread(buf, sizeof(buf), 1, f);
        if (memcmp(buf, pszFcGames, 8) != 0) {
            fclose(f);
            continue;
        }
        fclose(f);
        auto mod = buf[0xe] >> 5;
        char szTemp[1024], szNum[32];
        auto p = strrchr(strFile.c_str(), '\\');
        itoa(mod, szNum, 2);
        strcpy(&szNum[12], "00");
        strcpy(&szNum[15 - strlen(szNum)], szNum);
        if (buf[0xd] > 0) {
            strcpy(szNum, "W");
        }
        else {
            strcpy(szNum, "");
        }
        if (buf[0xf] > 0) {
            strcat(szNum, "R");
        }
        sprintf(szTemp, "X:\\TEMP\\科王KW-SC2000随机软盘\\游戏\\%s-%s%s", &szNum[12], szNum, ++p);
        CopyFile(strFile.c_str(), szTemp, FALSE);
    }
    return true;
}
//static bool bScanVCD = ScanVCD();

static BMAPPINGLocal* createMapper(int num)
{
	if (MapperBase::pMapper)
		delete MapperBase::pMapper;
	MapperBase::pMapper = new KW3000Create();
	static BMAPPINGLocal ss;
	ss.init = MapperBase::s_Mapper_Init;
	ss.number = 174;
	ss.name = "YuXing";
	return &ss;
}
static MAP_Mapper& d = AddMapper(174, createMapper);


CHAR CdHsgId[] = "CDROM";
CHAR CdIsoId[] = "CD001";
CHAR CdXaId[] = "CD-XA001";

#define LEN_ROOT_DE                 (34)

typedef struct _RAW_ISO_VD {

    UCHAR       DescType;           // volume type: 1 = standard, 2 = coded
    UCHAR       StandardId[5];      // volume structure standard id = CD001
    UCHAR       Version;            // volume structure version number = 1
    UCHAR       VolumeFlags;        // volume flags
    UCHAR       SystemId[32];       // system identifier
    UCHAR       VolumeId[32];       // volume identifier
    UCHAR       Reserved[8];        // reserved 8 = 0
    ULONG       VolSpaceI;          // size of the volume in LBN's Intel
    ULONG       VolSpaceM;          // size of the volume in LBN's Motorola
    UCHAR       CharSet[32];        // character set bytes 0 = ASCII
    USHORT      VolSetSizeI;        // volume set size Intel
    USHORT      VolSetSizeM;        // volume set size Motorola
    USHORT      VolSeqNumI;         // volume set sequence number Intel
    USHORT      VolSeqNumM;         // volume set sequence number Motorola
    USHORT      LogicalBlkSzI;      // logical block size Intel
    USHORT      LogicalBlkSzM;      // logical block size Motorola
    ULONG       PathTableSzI;       // path table size in bytes Intel
    ULONG       PathTableSzM;       // path table size in bytes Motorola
    ULONG       PathTabLocI[2];     // LBN of 2 path tables Intel
    ULONG       PathTabLocM[2];     // LBN of 2 path tables Motorola
    UCHAR       RootDe[LEN_ROOT_DE];// dir entry of the root directory
    UCHAR       VolSetId[128];      // volume set identifier
    UCHAR       PublId[128];        // publisher identifier
    UCHAR       PreparerId[128];    // data preparer identifier
    UCHAR       AppId[128];         // application identifier
    UCHAR       Copyright[37];      // file name of copyright notice
    UCHAR       Abstract[37];       // file name of abstract
    UCHAR       Bibliograph[37];    // file name of bibliography
    UCHAR       CreateDate[17];     // volume creation date and time
    UCHAR       ModDate[17];        // volume modification date and time
    UCHAR       ExpireDate[17];     // volume expiration date and time
    UCHAR       EffectDate[17];     // volume effective date and time
    UCHAR       FileStructVer;      // file structure version number = 1
    UCHAR       Reserved3;          // reserved
    UCHAR       ResApp[512];        // reserved for application
    UCHAR       Reserved4[653];     // remainder of 2048 bytes reserved

} RAW_ISO_VD;
typedef RAW_ISO_VD* PRAW_ISO_VD;


typedef struct _RAW_HSG_VD {

    ULONG       BlkNumI;            // logical block number Intel
    ULONG       BlkNumM;            // logical block number Motorola
    UCHAR       DescType;           // volume type: 1 = standard, 2 = coded
    UCHAR       StandardId[5];      // volume structure standard id = CDROM
    UCHAR       Version;            // volume structure version number = 1
    UCHAR       VolumeFlags;        // volume flags
    UCHAR       SystemId[32];       // system identifier
    UCHAR       VolumeId[32];       // volume identifier
    UCHAR       Reserved[8];        // reserved 8 = 0
    ULONG       VolSpaceI;          // size of the volume in LBN's Intel
    ULONG       VolSpaceM;          // size of the volume in LBN's Motorola
    UCHAR       CharSet[32];        // character set bytes 0 = ASCII
    USHORT      VolSetSizeI;        // volume set size Intel
    USHORT      VolSetSizeM;        // volume set size Motorola
    USHORT      VolSeqNumI;         // volume set sequence number Intel
    USHORT      VolSeqNumM;         // volume set sequence number Motorola
    USHORT      LogicalBlkSzI;      // logical block size Intel
    USHORT      LogicalBlkSzM;      // logical block size Motorola
    ULONG       PathTableSzI;       // path table size in bytes Intel
    ULONG       PathTableSzM;       // path table size in bytes Motorola
    ULONG       PathTabLocI[4];     // LBN of 4 path tables Intel
    ULONG       PathTabLocM[4];     // LBN of 4 path tables Motorola
    UCHAR       RootDe[LEN_ROOT_DE];// dir entry of the root directory
    UCHAR       VolSetId[128];      // volume set identifier
    UCHAR       PublId[128];        // publisher identifier
    UCHAR       PreparerId[128];    // data preparer identifier
    UCHAR       AppId[128];         // application identifier
    UCHAR       Copyright[32];      // file name of copyright notice
    UCHAR       Abstract[32];       // file name of abstract
    UCHAR       CreateDate[16];     // volume creation date and time
    UCHAR       ModDate[16];        // volume modification date and time
    UCHAR       ExpireDate[16];     // volume expiration date and time
    UCHAR       EffectDate[16];     // volume effective date and time
    UCHAR       FileStructVer;      // file structure version number
    UCHAR       Reserved3;          // reserved
    UCHAR       ResApp[512];        // reserved for application
    UCHAR       Reserved4[680];     // remainder of 2048 bytes reserved

} RAW_HSG_VD;
typedef RAW_HSG_VD* PRAW_HSG_VD;


typedef struct _RAW_JOLIET_VD {

    UCHAR       DescType;           // volume type: 2 = coded
    UCHAR       StandardId[5];      // volume structure standard id = CD001
    UCHAR       Version;            // volume structure version number = 1
    UCHAR       VolumeFlags;        // volume flags
    UCHAR       SystemId[32];       // system identifier
    UCHAR       VolumeId[32];       // volume identifier
    UCHAR       Reserved[8];        // reserved 8 = 0
    ULONG       VolSpaceI;          // size of the volume in LBN's Intel
    ULONG       VolSpaceM;          // size of the volume in LBN's Motorola
    UCHAR       CharSet[32];        // character set bytes 0 = ASCII, Joliett Seq here
    USHORT      VolSetSizeI;        // volume set size Intel
    USHORT      VolSetSizeM;        // volume set size Motorola
    USHORT      VolSeqNumI;         // volume set sequence number Intel
    USHORT      VolSeqNumM;         // volume set sequence number Motorola
    USHORT      LogicalBlkSzI;      // logical block size Intel
    USHORT      LogicalBlkSzM;      // logical block size Motorola
    ULONG       PathTableSzI;       // path table size in bytes Intel
    ULONG       PathTableSzM;       // path table size in bytes Motorola
    ULONG       PathTabLocI[2];     // LBN of 2 path tables Intel
    ULONG       PathTabLocM[2];     // LBN of 2 path tables Motorola
    UCHAR       RootDe[LEN_ROOT_DE];// dir entry of the root directory
    UCHAR       VolSetId[128];      // volume set identifier
    UCHAR       PublId[128];        // publisher identifier
    UCHAR       PreparerId[128];    // data preparer identifier
    UCHAR       AppId[128];         // application identifier
    UCHAR       Copyright[37];      // file name of copyright notice
    UCHAR       Abstract[37];       // file name of abstract
    UCHAR       Bibliograph[37];    // file name of bibliography
    UCHAR       CreateDate[17];     // volume creation date and time
    UCHAR       ModDate[17];        // volume modification date and time
    UCHAR       ExpireDate[17];     // volume expiration date and time
    UCHAR       EffectDate[17];     // volume effective date and time
    UCHAR       FileStructVer;      // file structure version number = 1
    UCHAR       Reserved3;          // reserved
    UCHAR       ResApp[512];        // reserved for application
    UCHAR       Reserved4[653];     // remainder of 2048 bytes reserved

} RAW_JOLIET_VD;
typedef RAW_JOLIET_VD* PRAW_JOLIET_VD;
const char CDUdfId[] = "BEA01";
enum { CD_SECTOR_SIZE = 2048 };
#define FIRST_VD_SECTOR             16

bool KW3000() {
    auto p = L"X:\\TEMP\\科王KW-SC2000随机软盘\\科王2000游戏光盘3\\BUNGCVCD3.cue";
    //p = L"O:\\360极速浏览器下载\\OK\\Game\\科王\\科王电脑VCD操作说明及演示.bin";
	auto d = CDImageOpen(p);
	char buf[40960];
	char VolumeDescriptor[CD_SECTOR_SIZE];
	if (ReadSectors(FIRST_VD_SECTOR, VolumeDescriptor, 1) != CD_SECTOR_SIZE)
		return false;

	if (!memcmp(((PRAW_ISO_VD)VolumeDescriptor)->StandardId, CdIsoId, sizeof(CdIsoId) - 1))
	{
        auto d = (PRAW_ISO_VD)VolumeDescriptor;
        return true;
    }
    else if (!memcmp(((PRAW_HSG_VD)VolumeDescriptor)->StandardId, CdHsgId, sizeof(CdHsgId) - 1))
    {
        auto d = (PRAW_HSG_VD)VolumeDescriptor;
        return true;
    }
    else if (!memcmp(&VolumeDescriptor[1], CDUdfId, sizeof(CDUdfId) - 1))
    {
        return true;
    }
    else
        return false;
}
//static bool bkw = KW3000();
