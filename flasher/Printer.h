
//port type
#define PORT_SERIAL    1
#define PORT_PARALLEL  2
#define PORT_USBDEVICE 3
#define PORT_ETHERNET  4
#define PORT_DRIVER    5

//code of return value
#define SDK_SUCCESS		            0              
#define ERR_HANDLE			        -1
#define ERR_PARAMETER			    -2
#define ERR_APIFUNCTION			    -3
#define ERR_TIMEOUT			        -4
#define ERR_OPENFILE			    -5
#define ERR_READFILE			    -6
#define ERR_WRITEFILE			    -7
#define ERR_OUTBOUND			    -8
#define ERR_LOADDLLFILE			    -9
#define ERR_LOADDLLFUNCTION	        -10
#define ERR_DOWNLOADFILECLASH		-11
#define ERR_DOWNLOADFILE			-12
#define ERR_NOFLASH			        -13
#define ERR_NORAM			        -14
#define ERR_IMAGETYPE			    -15
#define ERR_BARCODE_PERCENTAGE		-16
#define ERR_BARCODE_CHARACTEROUT	-17
#define ERR_NOTSUPPORT				-18
#define ERR_NOFILENAME              -19
#define ERR_BUFFERERROR             -20

//label printing commands
#define PRINT_LINE                  1
#define PRINT_BOX                   2
#define PRINT_CIRCLE                3
#define PRINT_INNERFONT             4
#define PRINT_DOWNLOADINGFONT       5
#define PRINT_MIXFONT               6
#define PRINT_CODE_39               7
#define PRINT_CODE_UPCA             8
#define PRINT_CODE_UPCE             9
#define PRINT_CODE_NOVERIFY25       10
#define PRINT_CODE_128              11
#define PRINT_CODE_ENA13            12
#define PRINT_CODE_ENA8             13
#define PRINT_CODE_HBIC             14
#define PRINT_CODE_CODABAR          15
#define PRINT_CODE_VERIFY25         16
#define PRINT_CODE_INDUSTRY25       17
#define PRINT_CODE_TRANSPORT        18
#define PRINT_CODE_UPC2             19
#define PRINT_CODE_UPC5             20
#define PRINT_CODE_93               21
#define PRINT_CODE_POSTNET          22
#define PRINT_CODE_UCCENA           23
#define PRINT_CODE_EUROPE25         24
#define PRINT_CODE_JAPAN25          25
#define PRINT_CODE_CHINA25          26
#define PRINT_PDF                   27
#define PRINT_QR                    28
#define PRINT_MAXI                  29
#define PRINT_IMAGE                 30
#define PRINT_DOWNLOADINGIMAGE      31
#define PRINT_GRAYIMAGE             32
#define PRINT_TREUTYPE              33
#define PRINT_GM                    34

typedef struct COMSetting	//serial port struct
{
	char cPortName[32];		//serial port name,							spec: COM1, COM2 ... COM10, COM11, and so on.
	int iBaudrate;			//bits per second,							spec: 9600, 19200, 38400, 57600, and 115200.
	int iDataBits;			//data bits,								spec: 7 or 8.
	int iStopBits;			//stop bits,								spec: 1 or 2.
	int iParity;			//Parity,									spec: 0: None, 1: Odd, 2: Even.
	int iFlowControl;		//Flow control,								spec: 0: DTR_DSR, 1: RTS/CTS, 2: Xon/Xoff, 3: None. 
	int iCheckEnable;		//enable of checking communication,			spec: 0: not check communication, 1: check communication.
	int iCheckTimeout;		//set time out of checking communication,	spec: iCheckEnable = 1, iCheckTimeout is valid.
}COMPara;

typedef struct LPTSetting	//parallel port struct
{
	char cPortName[32];		//parallel port name, spec: LPT1, LPT2, and so on.
}LPTPara;

typedef struct NETSetting	//Ethernet port struct
{
	char cIPAddr[40];		//IP address
	int iNetPort;			//port 
}NetPara;

typedef struct DrvSetting	//driver struct
{
	char cDrvName[64];		//driver name
}DrvPara;

typedef struct USBSetting	//USB port struct
{
	int iUSBMode;			//USB mode setting, spec: 0---API mode USB, 1---class mode USB.
	int iDevID;				//device ID,		spec: iDevID >= 0, iDevID express device internal ID, open usb port by internasl ID; iDevID = -1, open usb port. 
	char cDevName[64];		//device name,		spec: iUSBMode = 1, cDevName express class mode USB device name.
}USBPara;

typedef struct BaseParameter//base parameter struct
{
	int iDPI;	        	//device DPI,				spec: 0 (default setting (200 DPI)), 1 (200DPI), 2 (300DPI).
	int iUnit;				//device setting unit,		spec: 0 (default setting(millimeter/10)), 1 (millimeter/10), 2 (dot), 3 (inch/100).
	int iOutMode;	    	//paper present mode,		spec: 0 (default setting of EEPROM), 1 (cutter), 2 (peel off), 3 (tear off), 4 (rewind). 
	int iPaperType;	  		//paper type,				spec: 0 (default setting of EEPROM), 1 (continuous paper),2 (mark paper), 3 (transmission label paper).
	int iPrintMode;	  		//print method,				spec: 0 (default setting of EEPROM), 1 (thermal), 2 (thermal transfer).
	int iAllRotateMode;		//set rotary print,			spec: 0 (switch off 180 degree rotary printing), 1 (Switch on the rotary printing).
	int iAllMirror;	  		//set mirror image print,	spec: 0 (not setting), 1 (set mirror image print).
}BasePara;

typedef struct LineSetting	//line printing struct
{
	int iStartX;	     	//start X-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	     	//start Y-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iEndX;	       		//end X-coordinate,   spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iEndY;				//end Y-coordinate,   spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iLineWidth;	  		//line width,         spec: 0 --- 999,  unit: dot.
}LinePrintPara;

typedef struct BoxSetting	//box printing struct
{
	int iStartX;	     	//start X-coordinate,					spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	     	//start Y-coordinate,					spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iWidth;	      		//horizontal width of box,				spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iHeight;	     	//vertical height of box,				spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iHorizontal;	 	//thickness of right and left box edge, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iVertical;	   		//thickness of bottom and top box edge, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
}BoxPrintPara;

typedef struct CircleSetting//circle printing struct
{
	int iCenterX;	    	//the center X-coordinate of a circle,	spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iCenterY;	    	//the center Y-coordinate of a circle,	spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRadius;	     	//radius,								spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iLineWidth;	  		//edge thickness,						spec: 0 --- 999, unit: dot.
}CirclePrintPara; 

typedef struct DownLoadingImageSetting//downloading image printing struct
{
	char cImageName[8]; 		//bitmap name of memory module,  spec: Max 8 characters.
	int iStartX;	      		//start X-coordinate,            spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	      		//start Y-coordinate,            spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iPointWidth;	  		//Dot width multiplying factor,  spec: 1 --- 8.	
	int iPointHeight;	 		//dot height multiplying factor, spec: 1 --- 8.
}DownLoadingImagePrintPara;

typedef struct ImageSetting		//image printing struct
{
	char cImageName[256];		//bitmap file path and name. 
	int iStartX;	      		//start X-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	      		//start Y-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
}ImagePrintPara;

typedef struct InnerFontSetting	//inner font printing struct
{
	char *cTextData;	  		//printing text data.
	int iStartX;	      		//start X-coordinate,			spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	      		//start Y-coordinate,			spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	      		//rotation in clockwise,		spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iFontType;	    		//font type ID,					spec: 0 --- 22.
	int iWidthMultiple; 		//dot width multiplying factor,	spec: 1 --- 8.
	int iHeightMultiple;		//dot height multiplying factor,spec: 1 --- 8.
	int iSpace;	       			//character space,				spec: -99 --- 99.
	int iMirrorEnable;			//mirror setting,				spec:0(mirror disabled),1(mirror enabled)
}InnerFontPrintPara;

typedef struct DownloadingFontSetting	//downloading font printing struct
{
	char *cText;	      		//printing text data
	int iStartX;				//start X-coordinate,				spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	      		//start Y-coordinate,				spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	      		//rotation in clockwise,			spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	char cFontName[20];	  		//font name, which is name of memory module font.
	int iWidthMultiple; 		//dot width multiplying factor,		spec: 1 --- 8.
	int iHeightMultiple;		//dot height multiplying factor,	spec: 1 --- 8.
	int iSpace;	       			//character space,					spec: -99 --- 99.
	int iMirrorEnable;			//mirror setting,					spec:0(mirror disabled),1(mirror enabled)
}DownloadingFontPrintPara;

typedef struct MixFontSetting	//mix font printing struct
{
	char *cText;	         	//printing text data  
	int iStartX;	         	//start X-coordinate,					spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iCN_StartY;	      		//Chinese start Y-coordinate,			spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iXY_Adjust;	      		//English font Offset against Chinese,	spec: -999 --- +999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,				spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	char cEN_FontType[20]; 		//English font name,					spec: "XYNN".
	char cCN_FontName[20]; 		//Chinese font name, which is name of memory module font.
	int iWidthMultiple;	  		//dot width multiplying factor,			spec: 1 --- 8.
	int iHeightMultiple;	 	//dot height multiplying factor,		spec: 1 --- 8.
	int iSpace;	          		//character space,						spec: -99 --- 99.
	int iMirrorEnable;	   		//mirror setting,						spec:0(mirror disabled),1(mirror enabled)
}MixFontPrintPara;

typedef struct TruetypeSetting	//vector font printing struct
{
	char *cText;	         	//printing text data  
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	char cFontName[100];	  	//system font name.
	int iFontHeight;	     	//font height.
	int iFontWidth;	      		//font width.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iBold;	           		//bold setting,				spec: 0 (normal), 1 (bold).
	int iItalic;	         	//italic setting,			spec: 0 (normal), 1 (italic).
	int iUnderline;	      		//underline setting,		spec: 0 (normal), 1 (underline).
}TruetypePrintPara;

typedef struct Code39Setting	//code 39 printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text). 
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}Code39PrintPara;

typedef struct CodeUPCASetting	//code UPCA printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).    
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeUPCAPrintPara;

typedef struct CodeUPCESetting	//code UPCE printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;					//bar code type,			spec: 0 (with no text), 1 (with text).  
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeUPCEPrintPara;

typedef struct NoVerify25CodeSetting//interleaved 2 of 5 (without check character) code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}NoVerify25CodePrintPara;

typedef struct Code128Setting	//code 128 printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).  
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}Code128PrintPara;

typedef struct CodeEna13Setting	//code ENA13 printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;					//bar code type,			spec: 0 (with no text), 1 (with text).  
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeEna13PrintPara;

typedef struct CodeEna8Setting	//code ENA8 printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).    
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeEna8PrintPara;

typedef struct CodeHBICSetting	//code HBIC printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).    
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}CodeHBICPrintPara;

typedef struct CodaBarSetting	//CODABAR printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).     
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}CodaBarPrintPara;

typedef struct CodeVerify25Setting//Interleaved 2 of 5 (with check character) code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}CodeVerify25PrintPara;

typedef struct CodeIndustry25Setting//industrial 2 of 5 code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}CodeIndustry25PrintPara;

typedef struct CodeTransportSetting	//shipping bearer code printing struct */ 
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,			spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,		spec: 1 --- 24.
}CodeTransportPrintPara;

typedef struct CodeUPC2Setting	//UPC2 code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeUPC2PrintPara;

typedef struct CodeUPC5Setting	//UPC5 code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;					//bar code type,			spec: 0 (with no text), 1 (with text).   
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeUPC5PrintPara;

typedef struct Code93Setting	//93 code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).  
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}Code93PrintPara;

typedef struct CodePOSTNETSetting//POSTNET code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodePOSTNETPrintPara;

typedef struct CodeUCCENASetting//UCC/ENA code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,			spec: 0 (with no text), 1 (with text).  
	int iHeight;	         	//bar code height,			spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumberBase;	     	//narrow bar width,			spec: 1 --- 24.
}CodeUCCENAPrintPara;

typedef struct CodeEurope25Setting//Matrix 2 of 5(Europe standard) printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;	         	//start X-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise, spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,         spec: 0 (with no text), 1 (with text).     
	int iHeight;	         	//bar code height,       spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,       spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,     spec: 1 --- 24.
}CodeEurope25PrintPara;

typedef struct CodeJapan25Setting//Matrix 2 of 5(Japan standard) printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;				//start X-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise, spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,         spec: 0 (with no text), 1 (with text).    
	int iHeight;	         	//bar code height,       spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,       spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,     spec: 1 --- 24.
}CodeJapan25PrintPara;

typedef struct CodeChina25Setting//Postnet 2 of 5(China) printing struct
{
	char* cCodeData;	     	//bar code data.
	int iStartX;				//start X-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,    spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise, spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).
	int iType;	           		//bar code type,         spec: 0 (with no text), 1 (with text).      
	int iHeight;	         	//bar code height,       spec: 0 --- 999, unit: dot, millimeter/10, inch/100.
	int iNumber;	         	//ratio numerator,       spec: 1 --- 24.
	int iNumberBase;	     	//ratio denominator,     spec: 1 --- 24.
}CodeChina25PrintPara;

typedef struct PDFSetting		//PDF417 printing struct
{
	char* cCodeData;	     	//bar code data.
	int iDataLen;	        	//length of data. 
	int iStartX;	         	//start X-coordinate,			spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,			spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,		spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).	
	int iBaseWidth;	      		//factor width,					spec: 1 --- 24.
	int iBaseHeight;	     	//factor height,				spec: 1 --- 24.
	int iScaleWidth;	     	//appearance ratio numerator,   spec: 0 --- 9. 
	int iScaleHeight;	    	//appearance ratio denominator, spec: 0 --- 9.		
	int iRow;	            	//the number of lines,          spec: 3 --- 90.
	int iColumn;	         	//the number of columns,        spec: 1 --- 30.
	int iCutMode;	        	//truncate type,                spec: 0 (normal type), 1 (truncate type).
	int iLeve;	           		//check level,                  spec: 0 --- 8.
	int iDataMode;	       		//data mode,                    spec: 0 (character string mode), 1 (specify data length mode).
}PDFPrintPara;

typedef struct QRSetting		//QR code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iDataLen;	        	//length of bar code data.
	int iStartX;	         	//start X-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iWeigth;	         	//unit factor width,  spec: 1 --- 10.
	int iSymbolType;	     	//symbol type,        spec: 1 (the original type), 2 (enhanced type). 
	int iLanguageMode;	   		//language mode,      spec: 0 (Chinese), 1 (Japan).
}QRPrintPara;

typedef struct MAXISetting		//MAXI code printing struct
{
	char* cCodeData;	     	//bar code data.
	int iDataLen;	        	//length of data. 
	int iStartX;	         	//start X-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate, spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
}MAXIPrintPara;

typedef struct GMSetting		//GMCODE printing struct
{
	char* cCodeData;	     	//bar code data.
	int iDataLen;	        	//length of data. 
	int iStartX;	         	//start X-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iStartY;	         	//start Y-coordinate,		spec: 0 --- 9999, unit: dot, millimeter/10, inch/100.
	int iRotate;	         	//rotation in clockwise,	spec: 1 (0 degree), 2 (90 degree), 3 (180 degree), 4 (270 degree).		
	int iLevel;	          		//check level,				spec: 0 --- 5.
	int iWidth;	          		//unit factor width,		spec: 1 --- 99.  
	int iHeight;	         	//bar code height,			spec: 1 --- 999, unit: dot, millimeter/10, inch/100.
}GMPrintPara;

typedef struct GrayImageSetting	//Gray image printing struct
{
	char cImageFileName[256];	//Gray image file name
	int iGrayMode;	           	//Gray data mode,			spec: 0 --- general mode, 1 --- data format mode.

}GrayImagePrintPara;

//-----------------------------------------------------------
int LoadPrintDll(CHAR* path);
int FreePrintDll();

typedef int (__stdcall *mEnum_DeviceNameList)(int iOperationType,char *cDevNameBuf,int iBufLen,int *iNumber,char *cFilterInfor);
typedef int (__stdcall *mComm_OpenPort)(int iPortType,void *vPortPara,int iPortParaStructLen,int iSaveFileEnable,char *cFileName);
typedef int (__stdcall *mComm_ClosePort)(int hDev);
typedef int (__stdcall *mComm_WritePort)(int hDev,char *cBuf,int iBufLen,int *iReturnLen);
typedef int (__stdcall *mComm_ReadPort)(int hDev,char *cBuf,int iBufLen,int *iReturnLen);
typedef int (__stdcall *mComm_SetPortTimeout)(int hDev,int iWriteTimeout,int iReadTimeout);
typedef int (__stdcall *mComm_SendFile)(int hDev,char *cFileName);
typedef int (__stdcall *mComm_DrvDocOperation)(int hDev,int iOperation);
typedef int (__stdcall *mCtrl_DevControl)(int hDev,int iOperationID);
typedef int (__stdcall *mCtrl_DownLoadImage)(int hDev,char* cImageName, int iImageType, int iModuleType, char* cFileName,int iCheckTimeout);
typedef int (__stdcall *mCtrl_EraseModuleAndFile)(int hDev,int iEraseMode,int iModuleType,int iFileType, char *cFileName,int iWaitTimeout);
typedef int (__stdcall *mCtrl_FeedAndBack)(int hDev,int iDistance, int iDelayTime);
typedef int (__stdcall *mGet_VersionInfo)(int hDev,char *cVersionInfo,int iInfoLength, int *iInfoTrueLen,int iCheckTimeout);
typedef int (__stdcall *mGet_StatusData)(int hDev,char *cStatusBuf,int iStatusBufLen,int *iStatusDataLen,int iTimeout);
typedef int (__stdcall *mSet_BasePara)(int hDev,BasePara *sBasePara,int iStructLen);
typedef int (__stdcall *mSet_OutPosition)(int hDev,int iPosition);
typedef int (__stdcall *mSet_PaperLength)(int hDev,int iContinueLength);
typedef int (__stdcall *mSet_WaterMarkMode)(int hDev,int iLayoutMode,char *cFileName,int iCheckTimeout);
typedef int (__stdcall *mSet_HorizontalCopy)(int hDev,int iPieces, int iGap);
typedef int (__stdcall *mPrt_LabelToMemory)(int hDev);
typedef int (__stdcall *mPrt_MemoryLabel)(int hDev,int iPieces);
typedef int (__stdcall *mPrt_EnterLabelMode)(int hDev,int iGrayModeEnable,int iWidth,int iColumn,int iRow,int iSpeed,int iDarkness);
typedef int (__stdcall *mPrt_EndLabelAndPrint)(int hDev,int iTotalNum,int iSameNum,int iOutUnit);
typedef int (__stdcall *mPrt_LastFieldSetting)(int hDev,char *cDataBuf,int iDataLength,char *cFileEnter,int iFieldStartPoint,int iFieldLength);
typedef int (__stdcall *mPrt_LabelPrintSetting)(int hDev,int iSettingCommand,void *vParaSetting,int iStructLen, int iBitMode);

extern mEnum_DeviceNameList Enum_DeviceNameList;
extern mComm_OpenPort Comm_OpenPort;
extern mComm_ClosePort Comm_ClosePort;
extern mComm_WritePort Comm_WritePort;
extern mComm_ReadPort Comm_ReadPort;
extern mComm_SetPortTimeout Comm_SetPortTimeout;
extern mComm_SendFile Comm_SendFile;
extern mComm_DrvDocOperation Comm_DrvDocOperation;
extern mCtrl_DevControl Ctrl_DevControl;
extern mCtrl_DownLoadImage Ctrl_DownLoadImage;
extern mCtrl_EraseModuleAndFile Ctrl_EraseModuleAndFile;
extern mCtrl_FeedAndBack Ctrl_FeedAndBack;
extern mGet_VersionInfo Get_VersionInfo;
extern mGet_StatusData Get_StatusData;
extern mSet_BasePara Set_BasePara;
extern mSet_OutPosition Set_OutPosition;
extern mSet_PaperLength Set_PaperLength;
extern mSet_WaterMarkMode Set_WaterMarkMode;
extern mSet_HorizontalCopy Set_HorizontalCopy;
extern mPrt_LabelToMemory Prt_LabelToMemory;
extern mPrt_MemoryLabel Prt_MemoryLabel;
extern mPrt_EnterLabelMode Prt_EnterLabelMode;
extern mPrt_EndLabelAndPrint Prt_EndLabelAndPrint;
extern mPrt_LastFieldSetting Prt_LastFieldSetting;
extern mPrt_LabelPrintSetting Prt_LabelPrintSetting;