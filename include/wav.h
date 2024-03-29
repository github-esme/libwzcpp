#ifndef LIBWZ_WAV_H
#define LIBWZ_WAV_H

#include <cstdint>
#include <cstdlib>

namespace wav {

enum class WavFormatEncoding : uint16_t {
    kUnknown = 0,
    kPcm = 1,
    kAdpcm = 2,
    kIeeeFloat = 3,
    kVselp = 4,
    kIbmCvsd = 5,
    kALaw = 6,
    kMuLaw = 7,
    kDts = 8,
    kDrm = 9,
    kWmaVoice9 = 10,
    kOkiAdpcm = 16,
    kDviAdpcm = 17,
    kImaAdpcm = 17,
    kMediaspaceAdpcm = 18,
    kSierraAdpcm = 19,
    kG723Adpcm = 20,
    kDigiStd = 21,
    kDigiFix = 22,
    kDialogicOkiAdpcm = 23,
    kMediaVisionAdpcm = 24,
    kCUCodec = 25,
    kYamahaAdpcm = 32,
    kSonarC = 33,
    kDspGroupTrueSpeech = 34,
    kEchoSpeechCorporation1 = 35,
    kAudioFileAf36 = 36,
    kAptx = 37,
    kAudioFileAf10 = 38,
    kProsody1612 = 39,
    kLrc = 40,
    kDolbyAc2 = 48,
    kGsm610 = 49,
    kMsnAudio = 50,
    kAntexAdpcme = 51,
    kControlResVqlpc = 52,
    kDigiReal = 53,
    kDigiAdpcm = 54,
    kControlResCr10 = 55,
    kWaveFormatNMS_VBXADPCM = 56,
    kWaveFormatCS_IMAADPCM = 57,
    kWaveFormatECHOSC3 = 58,
    kWaveFormatROCKWELL_ADPCM = 59,
    kWaveFormatROCKWELL_DIGITALK = 60,
    kWaveFormatXEBEC = 61,
    kWaveFormatG721_ADPCM = 64,
    kWaveFormatG728_CELP = 65,
    kWaveFormatMSG723 = 66,
    kMpeg = 80,
    kWaveFormatRT24 = 82,
    kWaveFormatPAC = 83,
    kMpegLayer3 = 85,
    kWaveFormatLUCENT_G723 = 89,
    kWaveFormatCIRRUS = 96,
    kWaveFormatESPCM = 97,
    kWaveFormatVOXWARE = 98,
    kWaveFormatCANOPUS_ATRAC = 99,
    kWaveFormatG726_ADPCM = 100,
    kWaveFormatG722_ADPCM = 101,
    kWaveFormatDSAT_DISPLAY = 103,
    kWaveFormatVoxWareBYTE_ALIGNED = 105,
    kWaveFormatVoxWareAC8 = 112,
    kWaveFormatVoxWareAC10 = 113,
    kWaveFormatVoxWareAC16 = 114,
    kWaveFormatVoxWareAC20 = 115,
    kWaveFormatVoxWareRT24 = 116,
    kWaveFormatVoxWareRT29 = 117,
    kWaveFormatVoxWareRT29HW = 118,
    kWaveFormatVoxWareVR12 = 119,
    kWaveFormatVoxWareVR18 = 120,
    kWaveFormatVoxWareTQ40 = 121,
    kWaveFormatSOFTSOUND = 128,
    kWaveFormatVoxWareTQ60 = 129,
    kWaveFormatMSRT24 = 130,
    kWaveFormatG729A = 131,
    kWaveFormatMVI_MVI2 = 132,
    kWaveFormatDF_G726 = 133,
    kWaveFormatDF_GSM610 = 134,
    kWaveFormatISIAUDIO = 136,
    kWaveFormatONLIVE = 137,
    kWaveFormatSBC24 = 145,
    kWaveFormatDOLBY_AC3_SPDIF = 146,
    kWaveFormatMEDIASONIC_G723 = 147,
    kWaveFormatPROSODY_8KBPS = 148,
    kWaveFormatZYXEL_ADPCM = 151,
    kWaveFormatPHILIPS_LPCBB = 152,
    kWaveFormatPACKED = 153,
    kWaveFormatMALDEN_PHONYTALK = 160,
    kGsm = 161,
    kG729 = 162,
    kG723 = 163,
    kAcelp = 164,
    kRawAac = 255,
    kWaveFormatRHETOREX_ADPCM = 256,
    kWaveFormatIRAT = 257,
    kWaveFormatVIVOG723 = 273,
    kWaveFormatVIVOSiren = 274,
    kWaveFormatDIGITALG723 = 291,
    kWaveFormatSanyoLDADPCM = 293,
    kWaveFormatSiproLab_ACEPLNET = 304,
    kWaveFormatSiproLab_ACELP4800 = 305,
    kWaveFormatSiproLab_ACELP8V3 = 306,
    kWaveFormatSiproLab_G729 = 307,
    kWaveFormatSiproLab_G729A = 308,
    kWaveFormatSiproLab_KELVIN = 309,
    kWaveFormatG726ADPCM = 320,
    kWaveFormatQualcommPureVoice = 336,
    kWaveFormatQualcommHalfRate = 337,
    kWaveFormatTubGSM = 341,
    kWaveFormatMSAudio1 = 352,
    kWindowsMediaAudio = 353,
    kWindowsMediaAudioProfessional = 354,
    kWindowsMediaAudioLosseless = 355,
    kWindowsMediaAudioSpdif = 356,
    kWaveFormatUnisysNaADPCM = 368,
    kWaveFormatUnisysNapULAW = 369,
    kWaveFormatUnisysNapALAW = 370,
    kWaveFormatUnisysNap16K = 371,
    kWaveFormatCreativeADPCM = 512,
    kWaveFormatCreativeFastSpeech8 = 514,
    kWaveFormatCreativeFastSpeech10 = 515,
    kWaveFormatUHER_ADPCM = 528,
    kWaveFormatQUARTERDECK = 544,
    kWaveFormatILINK_VC = 560,
    kWaveFormatRAW_SPORT = 576,
    kWaveFormatESST_AC3 = 577,
    kWaveFormatIPI_HSX = 592,
    kWaveFormatIPI_RPELP = 593,
    kWaveFormatCS2 = 608,
    kWaveFormatSONY_SCX = 624,
    kWaveFormatFM_TOWNS_SND = 768,
    kWaveFormatBTV_DIGITAL = 1024,
    kWaveFormatQDESIGN_MUSIC = 1104,
    kWaveFormatVME_VMPCM = 1664,
    kWaveFormatTPC = 1665,
    kWaveFormatOLIGSM = 4096,
    kWaveFormatOLIADPCM = 4097,
    kWaveFormatOLICELP = 4098,
    kWaveFormatOLISBC = 4099,
    kWaveFormatOLIOPR = 4100,
    kWaveFormatLH_CODEC = 4352,
    kWaveFormatNORRIS = 5120,
    kWaveFormatSOUNDSPACE_MUSICOMPRESS = 5376,
    kMPEG_ADTS_AAC = 5632,
    kMPEG_RAW_AAC = 5633,
    kMPEG_LOAS = 5634,
    kVODAFONE_MPEG_ADTS_AAC = 5642,
    kODAFONE_MPEG_ADTS_AAC = 5642,
    kVODAFONE_MPEG_RAW_AAC = 5643,
    kMPEG_HEAAC = 5648,
    kWaveFormatDVM = 8192,
    kVorbis1 = 26447,
    kVorbis2 = 26448,
    kVorbis3 = 26449,
    kVorbis1P = 26479,
    kVorbis2P = 26480,
    kVorbis3P = 26481,
    kExtensible = 65534
};

#pragma pack(push, 1)
struct WavFormat {
    WavFormatEncoding format = WavFormatEncoding::kUnknown;
    uint16_t channels = 0;
    uint32_t sample_rate = 0;
    uint32_t average_bytes_per_second = 0;
    uint16_t block_ailgn = 0;
    uint16_t bits_per_sample = 0;
    uint16_t extra_size = 0;
};
#pragma pack(pop)

}  // namespace wav

#endif