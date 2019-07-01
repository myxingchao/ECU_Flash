#include "StdAfx.h"
#include "Printer.h"

mEnum_DeviceNameList Enum_DeviceNameList = NULL;
mComm_OpenPort Comm_OpenPort = NULL;
mComm_ClosePort Comm_ClosePort = NULL;
mComm_WritePort Comm_WritePort = NULL;
mComm_ReadPort Comm_ReadPort = NULL;
mComm_SetPortTimeout Comm_SetPortTimeout = NULL;
mComm_SendFile Comm_SendFile = NULL;
mComm_DrvDocOperation Comm_DrvDocOperation = NULL;
mCtrl_DevControl Ctrl_DevControl = NULL;
mCtrl_DownLoadImage Ctrl_DownLoadImage = NULL;
mCtrl_EraseModuleAndFile Ctrl_EraseModuleAndFile = NULL;
mCtrl_FeedAndBack Ctrl_FeedAndBack = NULL;
mGet_VersionInfo Get_VersionInfo = NULL;
mGet_StatusData Get_StatusData = NULL;
mSet_BasePara Set_BasePara = NULL;
mSet_OutPosition Set_OutPosition = NULL;
mSet_PaperLength Set_PaperLength = NULL;
mSet_WaterMarkMode Set_WaterMarkMode = NULL;
mSet_HorizontalCopy Set_HorizontalCopy = NULL;
mPrt_LabelToMemory Prt_LabelToMemory = NULL;
mPrt_MemoryLabel Prt_MemoryLabel = NULL;
mPrt_EnterLabelMode Prt_EnterLabelMode = NULL;
mPrt_EndLabelAndPrint Prt_EndLabelAndPrint = NULL;
mPrt_LastFieldSetting Prt_LastFieldSetting = NULL;
mPrt_LabelPrintSetting Prt_LabelPrintSetting = NULL;

HINSTANCE h_Dll = NULL;
int LoadPrintDll(CHAR* path)
{
	FreePrintDll();
	h_Dll = ::LoadLibrary(path);
	DWORD err = GetLastError();
	if(h_Dll == NULL)
		return 0;
	Enum_DeviceNameList = (mEnum_DeviceNameList)GetProcAddress(h_Dll,"Enum_DeviceNameList");
	if(Enum_DeviceNameList == NULL)
		goto FAILED;
	Comm_OpenPort = (mComm_OpenPort)GetProcAddress(h_Dll,"Comm_OpenPort");
	if(Comm_OpenPort == NULL)
		goto FAILED;
	Comm_ClosePort = (mComm_ClosePort)GetProcAddress(h_Dll,"Comm_ClosePort");
	if(Comm_ClosePort == NULL)
		goto FAILED;
	Comm_WritePort = (mComm_WritePort)GetProcAddress(h_Dll,"Comm_WritePort");
	if(Comm_WritePort == NULL)
		goto FAILED;
	Comm_ReadPort = (mComm_ReadPort)GetProcAddress(h_Dll,"Comm_ReadPort");
	if(Comm_ReadPort == NULL)
		goto FAILED;
	Comm_SetPortTimeout = (mComm_SetPortTimeout)GetProcAddress(h_Dll,"Comm_SetPortTimeout");
	if(Comm_SetPortTimeout == NULL)
		goto FAILED;
	Comm_SendFile = (mComm_SendFile)GetProcAddress(h_Dll,"Comm_SendFile");
	if(Comm_SendFile == NULL)
		goto FAILED;
	Comm_DrvDocOperation = (mComm_DrvDocOperation)GetProcAddress(h_Dll,"Comm_DrvDocOperation");
	if(Comm_DrvDocOperation == NULL)
		goto FAILED;
	Ctrl_DevControl = (mCtrl_DevControl)GetProcAddress(h_Dll,"Ctrl_DevControl");
	if(Ctrl_DevControl == NULL)
		goto FAILED;
	Ctrl_DownLoadImage = (mCtrl_DownLoadImage)GetProcAddress(h_Dll,"Ctrl_DownLoadImage");
	if(Ctrl_DownLoadImage == NULL)
		goto FAILED;
	Ctrl_EraseModuleAndFile = (mCtrl_EraseModuleAndFile)GetProcAddress(h_Dll,"Ctrl_EraseModuleAndFile");
	if(Ctrl_EraseModuleAndFile == NULL)
		goto FAILED;
	Ctrl_FeedAndBack = (mCtrl_FeedAndBack)GetProcAddress(h_Dll,"Ctrl_FeedAndBack");
	if(Ctrl_FeedAndBack == NULL)
		goto FAILED;
	Get_VersionInfo = (mGet_VersionInfo)GetProcAddress(h_Dll,"Get_VersionInfo");
	if(Get_VersionInfo == NULL)
		goto FAILED;
	Get_StatusData = (mGet_StatusData)GetProcAddress(h_Dll,"Get_StatusData");
	if(Get_StatusData == NULL)
		goto FAILED;
	Set_BasePara = (mSet_BasePara)GetProcAddress(h_Dll,"Set_BasePara");
	if(Set_BasePara == NULL)
		goto FAILED;
	Set_OutPosition = (mSet_OutPosition)GetProcAddress(h_Dll,"Set_OutPosition");
	if(Set_OutPosition == NULL)
		goto FAILED;
	Set_PaperLength = (mSet_PaperLength)GetProcAddress(h_Dll,"Set_PaperLength");
	if(Set_PaperLength == NULL)
		goto FAILED;
	Set_WaterMarkMode = (mSet_WaterMarkMode)GetProcAddress(h_Dll,"Set_WaterMarkMode");
	if(Set_WaterMarkMode == NULL)
		goto FAILED;
	Set_HorizontalCopy = (mSet_HorizontalCopy)GetProcAddress(h_Dll,"Set_HorizontalCopy");
	if(Set_HorizontalCopy == NULL)
		goto FAILED;
	Prt_LabelToMemory = (mPrt_LabelToMemory)GetProcAddress(h_Dll,"Prt_LabelToMemory");
	if(Prt_LabelToMemory == NULL)
		goto FAILED;
	Prt_MemoryLabel = (mPrt_MemoryLabel)GetProcAddress(h_Dll,"Prt_MemoryLabel");
	if(Prt_MemoryLabel == NULL)
		goto FAILED;
	Prt_EnterLabelMode = (mPrt_EnterLabelMode)GetProcAddress(h_Dll,"Prt_EnterLabelMode");
	if(Prt_EnterLabelMode == NULL)
		goto FAILED;
	Prt_EndLabelAndPrint = (mPrt_EndLabelAndPrint)GetProcAddress(h_Dll,"Prt_EndLabelAndPrint");
	if(Prt_EndLabelAndPrint == NULL)
		goto FAILED;
	Prt_LastFieldSetting = (mPrt_LastFieldSetting)GetProcAddress(h_Dll,"Prt_LastFieldSetting");
	if(Prt_LastFieldSetting == NULL)
		goto FAILED;
	Prt_LabelPrintSetting = (mPrt_LabelPrintSetting)GetProcAddress(h_Dll,"Prt_LabelPrintSetting");
	if(Prt_LabelPrintSetting == NULL)
		goto FAILED;
	return 1;	
FAILED:
	FreeLibrary(h_Dll);			
	h_Dll = NULL;
	return 0;
}
int FreePrintDll()
{
	if(h_Dll != NULL)
	{
		FreeLibrary(h_Dll);
		h_Dll = NULL;
	}
	return 1;
}