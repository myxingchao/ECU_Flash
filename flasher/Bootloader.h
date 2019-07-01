#pragma once

static CHAR* _START_CHIP_ID				= "\
:20F60000E6030300E6020300F2F024F076F02000F6F024F00A891414E001F6F15CB6E7F031\
:20F620005A00BB239AB6FE70F2F01EFF8AB7037028012D050DFB7EB7E011F6F15CB6F2F0BD\
:1CF640007CF0BB25E6F040FA9AB7FE70A400B2FE7EB786F05FFA3DF8EA0040FAFFFFE2\
:20F65E009AB7FE70F3F0B2FE7EB7CB009AB6FE707EB6F7F0B0FEF2F15CB62D01BBF1CB0019\
:1CF67E00ECF1BBEEF120BBECF130F001FCF1CB00ECF0BBECF101BBEAFCF0CB0097\
:00000001FF";

static CHAR* _MONITOR_004_B				= "\
:10E00000E6020300F68ED0A3E7F05A00BB549AB69E\
:10E01000FE70E6F0FFFF8AB7037028012D050DFBA7\
:10E020007EB7E011F6F1D0A3F2F07CF0BB5CE6F035\
:10E03000D0E3BB59E6F00000BB56F2F0D2A3BB53CD\
:10E04000CA00A4E5E60303004F891A8900E0E60947\
:10E0500000FCE60B00FCE60A00FABB2547F0B7001F\
:10E060003D1EE6F07B00BB27E005BB35F01046F116\
:10E0700040009D15E6F2FAA348102D05BB14B90225\
:10E08000082128110DF9F005BB2EBB2546F0FECA6C\
:10E090003D06BB2F48003D03E6F05A000D02E6F0B6\
:10E0A000A500BB090DDA9AB7FE70F3F0B2FE01A02D\
:10E0B00019B07EB7CB00ECF19AB6FE707EB6F7F0E1\
:10E0C000B0FE01A019B0F2F1D0A32D03ECF5BBEB2B\
:10E0D000FCF5FCF1CB00ECF1BBE6F120BBE4F13048\
:10E0E000F001FCF1CB00ECF0BBE6F101BBE4FCF08D\
:10E0F000CB00E6F0FAA39810F2F208FE46F1000118\
:10E100009D04DC02E6F23AE40D05DC02E6F26CE482\
:10E1100026F100015C110021A812E005AB01CB0043\
:10E12000ECF5D4500800BB0ABBDEF2F0DAA3BBDB8F\
:10E13000F2F0DCA3BBD8E000FCF5CB00ECF1ECF294\
:10E14000ECF3ECF4ECF6ECF7ECF89820981098303F\
:10E150009840986098709880F065F2F51CFFF6F58D\
:10E16000DAA3F6F5DCA39AF60500DC01A90201A00A\
:10E1700019B50D4B9AF60410BB96DC01B9020D439C\
:10E180009AF60420BBA8DC01B8020D3F9AF60430D1\
:10E19000DC01A902BB900D379AF60440DC01A8020D\
:10E1A000BBA20D339AF60950DC01A80240072D04EA\
:10E1B000BB3AF2F51EFF0D2E0D289AF60760DC0122\
:10E1C000A902DC08B907087118800D1D9AF60770BE\
:10E1D000DC01A802DC08B807087218800D169AF650\
:10E1E0000880CA00D6E0DC01A87240702D01BB1B7C\
:10E1F0000D0C9AF60890CA00A6E0DC01A9E241E005\
:10E200002D01BB110D0008210D0108221810283125\
:10E2100038403DA9FCF8FCF7FCF6FCF4FCF3FCF2FA\
:10E22000FCF1F005CB00ECF08150F2F0DCA34800EB\
:10E230003D06F6F2DAA3F6F1DCA3F78FDDA3FCF0DE\
:10E24000CB00ECF1ECF2ECF3ECF4C6F50000ECF6EC\
:10E250009820E6F0A500CA00B6E09AB6FE70F0621B\
:10E2600066F6FF1FF6F6B4FE9AF202D0DFD80D0173\
:10E27000DED8E6F20001E003E000CA00A6E041605B\
:10E280003D0A56F0FFFFCA00B6E09AB6FE700831AC\
:10E2900028213DF30D0EE6F0A500CA00B6E09AB6BF\
:10E2A000FE70E6F2FEA39802DED8F6F0B4FEE010AF\
:10E2B0000D01E000FCF6FCF5FCF4FCF3FCF2FCF1D3\
:10E2C000CB00ECF1ECF2ECF3ECF4C6F50000ECF66C\
:10E2D0009820E6F0A500CA00B6E09AB6FE70F0629B\
:10E2E00066F6FF1FF6F6B4FE9AF202D0DFD80D01F3\
:10E2F000DED8E044E002E6F3FF00E000CA00A6E05A\
:10E3000041603D1128213DFA28413DF4E044E002FE\
:10E31000E6F0FF00CA00B6E09AB6FE7028213DFA8A\
:10E3200028413DF50D0EE6F0A500CA00B6E09AB60C\
:10E33000FE70E6F2FEA39802DED8F6F0B4FEE0101E\
:10E340000D01E000FCF6FCF5FCF4FCF3FCF2FCF142\
:10E35000CB00C6F10100E7F0A500CA00B6E09AB60E\
:10E36000FE70E62400008AE2FEB06FA49AE2FEB0DE\
:10E370006EA4CA00A6E0F2F048FECA00E6E0CA00B9\
:10E38000D6E0F6F0D4A3E001E000FCF1CB00EC0015\
:10E39000EC01EC02EC03A8103D02AE890D01AF893F\
:10E3A000FA00A4E3CC00CC00CC00CC00FC03FC02BF\
:10E3B000FC01FC00E000CB00E6F040FA9AB7FE70EA\
:10E3C000A400B2FE7EB786F05FFA3DF8EA0040FA9C\
:02E3D2000B003E\
:10E43A00B8E320E152E38EE342E2C2E2B4E3B4E39A\
:10E44A00B4E3B4E3B4E3B4E3ECF4F04066F4FFFFFE\
:10E45A00E6F10000E6F08AE45C3400041810FCF4EB\
:10E46A00CB00B4E30AE51EE510E536E50AE542E528\
:10E47A0028E53AE7000001000080000010000200D1\
:10E48A000000010000200000002001000020000020\
:10E49A000040010000200000006001000020000090\
:10E4AA00008001000080000000000200000001005E\
:10E4BA000000030000000100000004000000010049\
:10E4CA000000050000000100000006000000010035\
:10E4DA000000070000000100000008000000010021\
:10E4EA00000009000000010000000A00000001000D\
:10E4FA0000000B000000010000000C0000000200F8\
:10E50A00BB5DE000CB00E6F0FFFFBB65CA00B6E0EA\
:10E51A00E000CB00A800BB5FCA00B6E0CB00A800B1\
:10E52A00CA002EE7CA00E6E0E000CB00F2F07CF079\
:10E53A00CA00E6E0E000CB00ECF1ECF2ECF3ECF41C\
:10E54A00ECF5ECF6ECF7ECF8981098209830984037\
:10E55A00E00566F3FCFFCA00D6E0F060F080CA006E\
:10E56A00D6E0F070CA00D0E640703D0940683D0729\
:10E57A0008141820283438403DEEE0060D01E01654\
:10E58A00F005CA00E6E0F006FCF8FCF7FCF6FCF53C\
:10E59A00FCF4FCF3FCF2FCF1CB00E6013800CF8975\
:10E5AA00E6F0FCA384001EFFCA008EE3E040D70019\
:10E5BA000E00F6F000E0BB02E010CB00ECF0D70052\
:10E5CA000E00F2F0140066F030FED7000E00F6F0EE\
:10E5DA001400FCF0CB00ECF8F08066F8FF032D0D78\
:10E5EA00D7100E00F6F80400F2F8020076F8800858\
:10E5FA00D7000E00F6F80200BB45F0807CA86883BD\
:10E60A002D0DD7100E00F6F80600F2F8020076F883\
:10E61A008008D7000E00F6F80200BB34F0807CC8F0\
:10E62A0068872D0FD7100E00F6F80400F2F80200E2\
:10E63A0066F87FFF76F80008D7000E00F6F80200A9\
:10E64A00BB21F08066F800802D10E038D7100E004C\
:10E65A00F6F80600F2F8020066F87FFF76F800087E\
:10E66A00D7000E00F6F80200BB0DD7000E00F2F03C\
:10E67A00140066F0CF012D00D7000E00F68E1400AC\
:10E68A00FCF8CB00ECF8D7000E00F2F8020076F89E\
:10E69A000080D7000E00F6F80200D7000E00F2F84C\
:10E6AA0000008AF8FA40D7000E00F2F8000066F877\
:10E6BA0066003DF9D7000E00F2F8020066F800B8CD\
:10E6CA003DF2FCF8CB00ECF8D7000E00F2F802009D\
:10E6DA0076F8002046F209009D0376F880000D02C4\
:10E6EA0066F87FFFD7200E00F6F80200F6F1100058\
:10E6FA00F6F21200D7100E00F6F60800F6F00A003D\
:10E70A00BBC1D7000E00F2F8140066F84D012D03C4\
:10E71A00F086F060F008E008D7000E00F6F8140062\
:10E72A00FCF8CB00ECF1ECF2E000FCF2FCF1CB00DF\
:10E73A00ECF1ECF2E040CA00B6E0D7000E00F2F2CB\
:10E74A00B0DF6827D7000E00F2F1B2DF68135C4130\
:10E75A007021D7000E00F2F1B4DF66F1FF035C61AD\
:10E76A007021D7000E00F2F1B6DF68135CE1702168\
:10E77A00F002CA00E6E0D7000E00F2F1BCDFD700D3\
:10E78A000E00F2F2BEDF91207012F005CA00E6E038\
:0EE79A00F001CA00E6E0E000FCF2FCF1CB006A\
:00000001FF";