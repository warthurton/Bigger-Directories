#include<stdlib.h> //malloc
#include <fcntl.h>
#include <io.h> //setmode
#include<stdio.h> //sprintf
#include <windows.h>
#include <Strsafe.h> //safe string copy e.e. StringCchPrintf
#include <tlhelp32.h> //Find process stuff
#include "CreateLargeDir64.h" //my file
#include <Winternl.h> //NtCreateFile


#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


//#include <ntstatus.h>
//#include <ntstrsafe.h>

#pragma once 
//http://stackoverflow.com/questions/5896030/how-to-use-windows-tooltip-control-without-bounding-to-a-tool
//http://www.cprogramming.com/tutorial/printf-format-strings.html
//https://msdn.microsoft.com/en-us/library/windows/desktop/aa378137(v=vs.85).aspxy

//findHandle = FindFirstFile(@"\\?\UNC\" + folder_path, out findData

bool foundResolution = false;
bool pCmdLineActive = false;
wchar_t hrtext[256]; //An array name is essentially a pointer to the first element in an array.
WIN32_FIND_DATAW dw; // directory data this will use stack memory as opposed to LPWIN32_FIND_DATA
WIN32_FIND_DATAA da;
int const pathLength = 32759, maxPathFolder = MAX_PATH - 3, treeLevelLimit = 2000, branchLimit = 1000;
const wchar_t BOM = L'\xFEFF'; //65279
wchar_t const *invalidPathName = L":\"/\\|?*<>";
wchar_t const eolFTA = L'\n';
wchar_t const separatorFTA = L'\\';
wchar_t const *lpref = L"\\\\?\\";
wchar_t const *driveIDBaseW = L"\\\\?\\C:\\";
wchar_t const *driveIDBaseWNT = L"\\??\\C:\\"; //NtCreateFile wants the wildcard
char const *driveIDBase = "C:\\";
wchar_t rootDir [pathLength]; //maxPathFolder unless delete fails
wchar_t *pathToDeleteW, *currPathW, *findPathW, *tempDest, *thisexePath, *createlargedirVAR; // directory pointers. cannot be initialised as a pointer
char *currPath;
//http://stackoverflow.com/questions/2516096/fastest-way-to-zero-out-a-2d-array-in-c
char dacfolders[127][MAX_PATH-3]; //[32768 / 257] [ MAX_PATH- 3] double array char is triple array
wchar_t dacfoldersW[255][MAX_PATH-3], dacfoldersWtmp[127][maxPathFolder], folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder] = { NULL }, pathsToSave [branchLimit][pathLength];
wchar_t reorgTmpWFS[treeLevelLimit][maxPathFolder];	wchar_t reorgTmpW[pathLength];


int folderdirCS, folderdirCW, branchLevel, branchTotal, branchLevelCum, branchLevelClickOld, branchLevelClick, branchTotalSaveFile, branchLevelInc, branchLevelIncCum, branchSaveI, branchTotalCum, branchTotalCumOld;
int i,j,k, errCode;
int idata, index, listTotal = 0, sendMessageErr = 0;
int treeLevel, trackFTA[branchLimit][2];
BOOL createFail = FALSE;
BOOL weareatBoot = FALSE;
BOOL setforDeletion = FALSE;
BOOL removeButtonEnabled = true;
BOOL am64Bit, exe64Bit; 
LPCDLGTEMPLATE lpTemplate; //resolution
PVOID OldValue = nullptr; //Redirection
WNDPROC g_pOldProc;
HANDLE hMutex, hdlNtCreateFile, hdlNTOut, exeHandle, ds;     // directory handle


//struct fileSystem
//{
//    char FT[treeLevelLimit][branchLimit][maxPathFolder];
	//char FB[1000];

//};


typedef BOOL (__stdcall *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);



//NTcreatefile stuff
//typedef int (*NTDLLptr) (int); //Function pointer example, but following is required
typedef NTSTATUS (__stdcall *NTDLLptr)(
    PHANDLE FileHandle, 
    ACCESS_MASK DesiredAccess, 
    POBJECT_ATTRIBUTES ObjectAttributes, 
    PIO_STATUS_BLOCK IoStatusBlock, 
    PLARGE_INTEGER AllocationSize,
    ULONG FileAttributes, 
    ULONG ShareAccess, 
    ULONG CreateDisposition, 
    ULONG CreateOptions, 
    PVOID EaBuffer, 
    ULONG EaLength );


//for NTcreatefile fileObject,  NTAPI is __stdcall
typedef VOID (__stdcall *PFN_RtlInitUnicodeString) (
    IN OUT PUNICODE_STRING  DestinationString,
    IN PCWSTR  SourceString );
typedef ULONG (__stdcall *PFN_RtlNtStatusToDosError) (
    IN NTSTATUS Status );
//static my_RtlInitUnicodeString rtlInitUnicodeString; //Makes no difference
//PFN_RtlNtStatusToDosError RtlNtStatusToDosError;

//PHANDLE hdlNTOut;
NTDLLptr foundNTDLL = nullptr; //returns variable here
UNICODE_STRING fn;
OBJECT_ATTRIBUTES fileObject;
IO_STATUS_BLOCK ioStatus;
NTSTATUS status;
const char createFnString[13] = "NtCreateFile"; //one extra for null termination
const char initUnicodeFnString[21] = "RtlInitUnicodeString";
const char NtStatusToDosErrorString[22] = "RtlNtStatusToDosError";
const wchar_t CLASS_NAME[]  = L"Sample Window Class";
//A pathname MUST be no more than 32, 760 characters in length. (ULONG) Each pathname component MUST be no more than 255 characters in length (USHORT)
//wchar_t longPathName=(char)0;  //same as '\0'
//The long directory name with 255 char "\" separator



//const size_t cchDest = MAX_PATH;
//#define arraysize cchDest;


//------------------------------------------------------------------------------------------------------------------
// Protos...
//------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ValidateProc(HWND, UINT, WPARAM, LPARAM); //subclass
LPCDLGTEMPLATE DoSystemParametersInfoStuff(HWND hwnd);
int GetCreateLargeDirPath (HWND hwnd, wchar_t *exePath);
bool Kleenup (HWND hwnd, bool weareatBoot);
int ExistRegValue ();
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName);
NTDLLptr DynamicLoader (bool progInit, wchar_t *fileObjVar);
bool ProcessfileSystem(HWND hwnd, bool falseReadtrueWrite, bool appendMode);
void FSDeleteInit (HWND hwnd, HWND hList);
bool FSDelete (HWND hwnd);
bool fsDelsub (int i, int j, HWND hwnd);
int RecurseRemovePath(int trackFTA[branchLimit][2], wchar_t folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder]);


int DisplayError (HWND hwnd, LPCWSTR messageText, int errorcode, int yesNo)
{		//The way this is set up is errorcode is not modifiable here. However if errCode is passed here is always byval and will always revert to zero.
		//*hrtext  (pointee) is value pointed to by hrtext. Can be replaced by hrtext[0]
		//hrtext[0] = (wchar_t)LocalAlloc(LPTR, 256*sizeof(wchar_t)); This dynamic allocation NOT required- see below
		//if (hrtext[0] == NULL) ErrorExit("LocalAlloc");
		//hrtext[0] = NULL;  or	//*hrtext = NULL; //simple enough but not req'd

		if (errorcode ==0){
		swprintf_s(hrtext, _countof(hrtext), L"%s.", messageText);
		}
		else //LT 0 my defined error, GT error should be GET_LAST_ERROR
		{
		swprintf_s(hrtext, _countof(hrtext), L"%s. Error Code:  %d", messageText, errorcode);
		}
		//change countof sizeof otherwise possible buffer overflow: here index and folderdirCS gets set to -16843010!
		if (yesNo)
		{
		int msgboxID = MessageBoxW(hwnd, hrtext, L"Warning", MB_YESNO);
			if (msgboxID == IDYES) 
			{
			return 1;
			}
			else
			{
			return 0;
			}//IDNO
		}
		else
		{
		MessageBoxW(hwnd, hrtext, L"Warning", MB_OK);
		}

		return 0;
		//if ((HANDLE)*hrtext) LocalFree((HANDLE)*hrtext); // It is not safe to free memory allocated with GlobalAlloc. -MSDN	
		//wchar_t hrtext[256] allocates memory to the stack. It is not a dynamic allocation http://stackoverflow.com/questions/419022/char-x256-vs-char-malloc256sizeofchar
}

void ErrorExit (LPCWSTR lpszFunction, DWORD NTStatusMessage)
{
	//courtesy https://msdn.microsoft.com/en-us/library/windows/desktop/ms680582(v=vs.85).aspx
	// also see http://stackoverflow.com/questions/35177972/wide-char-version-of-get-last-error/35193301#35193301
	DWORD dww = 0;
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	
	if (NTStatusMessage)
	{
		dww = NTStatusMessage;
		FormatMessageW( 
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM | 
		FORMAT_MESSAGE_FROM_HMODULE,
		hdlNtCreateFile,
		dww,  
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR) &lpMsgBuf,  
		0,  
		nullptr );
	}
	else
	{
	
		dww = GetLastError();

	FormatMessageW(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		dww,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	(LPWSTR)&lpMsgBuf,0, nullptr);
	}
	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	
	
	StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("%s failed with error %lu: %s"), lpszFunction, dww, lpMsgBuf);
	wprintf(L"\a");  //audible bell
	MessageBoxW(nullptr, (LPCTSTR)lpDisplayBuf, L"Error", MB_OK);

	LocalFree(lpDisplayBuf);
	LocalFree(lpMsgBuf);
	
	//ExitProcess(dw);
}


void PopulateList(HWND hwnd)
{
	TCHAR* pszTxt = TEXT("My large nested directory");
	TCHAR pszDest[arraysize];
	BOOL findhandle;
	int gotolooper = 0;
	errCode = 0;


	size_t cbDest = arraysize * sizeof(TCHAR); //the use of size_t implies C++ compile.
	LPCTSTR pszFormat = TEXT("%s");
	HRESULT hr = StringCbPrintf(pszDest, cbDest, pszFormat, pszTxt); //swprintf_s
	
	
	//if (foundNTDLL) we can use the better function

	if (!DynamicLoader (true, tempDest)) DisplayError (hwnd, L"The long path function has been removed. Using 'short' path functions...", errCode, 0);

	
	#if defined(ENV64BIT) //#if is a directive: see header file
	{
    if (sizeof(void*) != 8)
    {
        	DisplayError (hwnd, L"ENV64BIT: Error: pointer should be 8 bytes. Exiting.", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			exit (1); //EndDialog will process the rest of the code in the fn.
    }
    am64Bit = true;
	exe64Bit = true;
	}
	#elif defined (ENV32BIT)
	{
		if (sizeof(void*) != 4)
		{
			DisplayError (hwnd, L"ENV32BIT: Error: pointer should be 4 bytes. Exiting.", errCode, 0);
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			ReleaseMutex (hMutex);
			exit (1);
		}
    
		if (FindProcessId (hwnd, L"CreateLargeDir64.exe", exeHandle) != NULL)
		{

			am64Bit = false;
			exe64Bit = false;
			LPFN_ISWOW64PROCESS fnIsWow64Process;

			fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(GetModuleHandleW((L"kernel32")),"IsWow64Process");
			if(nullptr != fnIsWow64Process)
				{
				exe64Bit = fnIsWow64Process(GetCurrentProcess(),&exe64Bit) && exe64Bit;
				}

		}

		else

		{
			DisplayError (hwnd, L"Our own process isn't active!? Must terminate!", 1, 0);
			ReleaseMutex (hMutex);
			exit (1); //EndDialog will process the rest of the code in the fn.
		}
}	

	#else
    //#error "user" gen error won't compile with current settings: "Must define either ENV32BIT or ENV64BIT". 128 bit?
	#endif



if (FindProcessId (hwnd, L"explorer.exe", exeHandle) == NULL)
	{
	weareatBoot=TRUE;
	EnableWindow(GetDlgItem(hwnd, IDC_LOGON), false);
	EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), false);
	}
else
	{
	if (!FindProcessId (hwnd, L"userinit.exe", exeHandle) == NULL)
	{
	DisplayError (hwnd, L"Userinit should have ended. Try rebooting before running this (or any other) program!", errCode, 0);
	}
	//Now to check if I am 64 bit


	createlargedirVAR= (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", createlargedirVAR, maxPathFolder)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.",0);
	wcscat_s(createlargedirVAR, maxPathFolder, L"\\Temp\\CreateLargeDir64.exe");

		if (GetFileAttributesW(createlargedirVAR)!=INVALID_FILE_ATTRIBUTES)
		{
			EnableWindow(GetDlgItem(hwnd, IDC_LOGON), false);
		}
	free(createlargedirVAR);
	}
	
	if (ExistRegValue() == 1)
	{
		setforDeletion = TRUE;
	}


	//NULL is a macro that's guaranteed to expand to a null pointer constant.
	//C strings are NUL-terminated, not NULL-terminated.  (char)(0) is the NUL character, (void * )(0) is	NULL, type void * , is called a null pointer constant
	//If (NULL == 0) isn't true you're not using C.  '\0' is the same as '0' see https://msdn.microsoft.com/en-us/library/h21280bw.aspx but '0' does not work!
	//http://stackoverflow.com/questions/15610506/can-the-null-character-be-used-to-represent-the-zero-character  NO
	createFail = false;
	branchLevelClickOld = 0;
	branchLevelClick = 0;
	branchLevelCum = 0;
	branchTotalSaveFile = -1;
	branchTotal = -1;
	branchTotalCum = 0;
	branchTotalCumOld = 0;
	memset(dacfolders, '\0', sizeof(dacfolders));  //'\0' is NULL L'\0' is for C++ but we are compiling in Unicode anyway
	memset(dacfoldersW, L'\0', sizeof(dacfoldersW));
	memset(folderTreeArray, L'\0', sizeof(folderTreeArray)); //required for remove function
	memset(pathsToSave, L'\0', sizeof(pathsToSave)); //required for create
	EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
	EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
	EnableWindow(GetDlgItem(hwnd, IDC_CREATE), false);
	removeButtonEnabled = true;
		
	for (j = 0; j < branchLimit; j++)
		{
		trackFTA [j][0] = 0; //Initial conditons before search on path
		trackFTA [j][1] = 0;
		}
	
	//Bad:
	//malloc(sizeof(char *) * 5) // Will allocate 20 or 40 bytes depending on 32 63 bit system
	//Good:
	// malloc(sizeof(char) * 5) // Will allocate 5 bytes

	currPath = (char *)calloc(maxPathFolder, sizeof(char));
	currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	if ((currPath == nullptr) || (currPathW == nullptr))
	{
		/* We were not so display a message */
		errCode = -1;
		DisplayError (hwnd, L"Could not allocate required memory", errCode, 0);
		return;
	}


	if (hr == S_OK)
	{
		SetDlgItemText(hwnd, IDC_TEXT, pszDest);

	}
	else
	{

		DisplayError (hwnd, L"StringCbPrintf didn't work, quitting: code %#08X", (int)hr, 0);
		return;

	}
	SetDlgItemInt(hwnd, IDC_NUMBER, 3, FALSE); //set repeat number
	

gotoloop: //Wide Char loop
		  //Populate List
	findhandle = TRUE;
		
	if (gotolooper == 0)
	{
		memset(&da, 0, sizeof(WIN32_FIND_DATA));
		folderdirCS = 0;
		strcpy_s(currPath, maxPathFolder, driveIDBase);

		strcat_s(currPath, maxPathFolder, "*");
		ds = FindFirstFileA(currPath, &da);

	}
	else
	{
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		folderdirCW = 0;
		//http://stackoverflow.com/questions/32540779/wcscpy-does-not-accept-tchar-in-destination-variable
		wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);

		wcscat_s(currPathW, maxPathFolder, L"*");
		ds = FindFirstFileW(currPathW, &dw); //dw points to found folders

	}


	//The plain versions without the underscore affect the character set the Windows header files treat as default. So if you define UNICODE, then GetWindowText will map to GetWindowTextW instead of GetWindowTextA, for example. Similarly, the TEXT macro will map to L"..." instead of "...". 
	//The versions with the underscore affect the character set the C runtime header files treat as default. So if you define _UNICODE, then _tcslen will map to wcslen instead of strlen, for example. Similarly, the _TEXT macro will map to L"..." instead of "...". 



	if (ds == INVALID_HANDLE_VALUE && (gotolooper == 0))
	{
		//StringCchPrintf (pszDest, cbDest, TEXT("No directories found."));
		errCode = -3;
		DisplayError (hwnd, L"No directories found", errCode, 0);
		goto CLEANUP;
	}

	//Main loop
	while (ds != INVALID_HANDLE_VALUE && findhandle)
	{


			if (gotolooper == 0)
			{
				if ((da.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(da.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
					|| da.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || da.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || da.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {

					//define FILE_ATTRIBUTE_READONLY              0x00000001
					//#define FILE_ATTRIBUTE_HIDDEN               0x00000002
					//#define FILE_ATTRIBUTE_SYSTEM               0x00000004
					//#define FILE_ATTRIBUTE_DIRECTORY            0x00000010
					//#define FILE_ATTRIBUTE_ARCHIVE              0x00000020
					//#define FILE_ATTRIBUTE_DEVICE               0x00000040
					//#define FILE_ATTRIBUTE_NORMAL               0x00000080
					//#define FILE_ATTRIBUTE_TEMPORARY            0x00000100
					//#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200
					//#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400
					//#define FILE_ATTRIBUTE_COMPRESSED           0x00000800
					//#define FILE_ATTRIBUTE_OFFLINE              0x00001000
					//#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000
					//#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000
					//#define FILE_ATTRIBUTE_VIRTUAL              0x00010000

				// (wchar_t *)currPathW not necessary here
					strcpy_s(currPath, maxPathFolder, (char *)driveIDBase);
					strcat_s(currPath, maxPathFolder, da.cFileName);


					strcat_s(dacfolders[folderdirCS], maxPathFolder, currPath);
					folderdirCS += 1;

					SendDlgItemMessageA(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(folderdirCS + folderdirCW), (LPARAM)currPath); // wparam cannot exceed 32,767 
					
				}
				findhandle = FindNextFileA(ds, &da);
			}
			else
			{
				if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM
					|| dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT || dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {


				wcscpy_s(currPathW, maxPathFolder, (wchar_t *)driveIDBaseW);
				wcscat_s(currPathW, maxPathFolder, dw.cFileName);
			
				
				//compare with dacfolders[folderdirC] to check for dups
				wcscat_s(dacfoldersW[folderdirCW], maxPathFolder, currPathW);
				folderdirCW += 1;
				sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, (WPARAM)(folderdirCS + folderdirCW), (LPARAM)currPathW); // wparam cannot exceed 32,767 
				
				}
				findhandle = FindNextFileW(ds, &dw);

		}
			sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)(folderdirCS + folderdirCW), (LPARAM)(folderdirCS + folderdirCW)); //(lparam)folderdirC required for getitemdata
																																		 //The Notification Code is passed as the HIWORD of wParam, the other half of the parameter that gave us the index of the control identifier in the first place. 
																																		 //HIWORD is the Upper 16 bits of UINT and LOWORD is the Lower 16 bits of UINT
																																		 //DWORD is just a typedef for 32-bit integer, whereas WORD is a typedef for a 16-bit integer

	}




	//if (folderdirC == 0) WHAAAAT you have no folders?

	if (gotolooper == 0)
	{
		FindClose(ds);
		gotolooper = 1;
		goto gotoloop;
	}



CLEANUP:
	//http://stackoverflow.com/questions/1912325/checking-for-null-before-calling-free
	if (currPath) free (currPath); //We may need these later though
	if (currPathW) free (currPathW); //Free from the heap We may need these later though
	if (findPathW) free (findPathW);
	//There's an internal index that is reset to 0 each time you call FindFirstFile() and it's incremented each time you call FindNextFile() so unless you do it in a loop, you'll only get the first filename ( a dot ) each time. 	
	FindClose(ds);
	//get leftmost string of findPathW

}


LRESULT CALLBACK ValidateProc (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
      wchar_t chChar;
      switch (message)
      {
		case WM_CHAR:
			chChar = (wchar_t) wParam;
			if(wcschr(invalidPathName, chChar)) return 0;
		break;
      }
      return CallWindowProc (g_pOldProc, hwnd, message, wParam, lParam);
}


INT_PTR CALLBACK DlgProc(HWND hwnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	errCode = 0;
	switch(Msg)
	{
		case WM_INITDIALOG:
			
            {	
				
		
			hMutex = CreateMutex( nullptr, TRUE, L"CreateLargeDir64.exe" );
			if (hMutex)
			{
			DWORD wait_success = WaitForSingleObject (hMutex, 30 );
			if (wait_success == WAIT_OBJECT_0 || wait_success == WAIT_ABANDONED)
				{
				// Our thread got ownership of the mutex or the other thread closed without releasing its mutex.
						if (pCmdLineActive) 
							{
								//MessageBoxW (NULL, L"if (pCmdLineActive)", L"\0", MB_OK); //for debugging
							if (!foundResolution) lpTemplate = DoSystemParametersInfoStuff(hwnd);

								PopulateList (hwnd);
								SendDlgItemMessage(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
								//MessageBoxW(NULL, NULL, L"Warning", NULL);
								FSDeleteInit (hwnd, nullptr);
								if (rootDir[0] != L'\0') rootDir[0] = L'\0';
								
							}

						PopulateList (hwnd);
						HWND TextValidate = GetDlgItem(hwnd, IDC_TEXT);
						// Subclass the Edit control with ValidateProc
						g_pOldProc = (WNDPROC)SetWindowLong(TextValidate, GWL_WNDPROC, (LONG)ValidateProc);
						switch (errCode)
						{
						case 1:
						{
							/* And exit */
							_CrtDumpMemoryLeaks();
							EndDialog(hwnd, 1);
							//exit(EXIT_FAILURE); //Not recommended: http://stackoverflow.com/questions/7562335/what-is-the-correct-way-to-programmatically-quit-an-mfc-application
						}
						break;
						case 2:
						{
							EndDialog(hwnd, 1);
						}
						break;
						case 3:
						{
							EndDialog(hwnd, 1);
						}
						break;
						default:
						{}
						break;
						}

					if (!ReleaseMutex (hMutex)) ErrorExit (L"ReleaseMutex: Handle error. ", 1);
	
				} 
				else
				{
				if  (WAIT_TIMEOUT && !pCmdLineActive)
					{
						DisplayError (hwnd, L"One instance is already running!", errCode, 0);
						CloseHandle (hMutex);
						_CrtDumpMemoryLeaks();
						ExitProcess(1);
					}
				}
			}
			else
			{
				DisplayError (hwnd, L"Could not create hMutex!", errCode, 0);
				CloseHandle (hMutex);
				_CrtDumpMemoryLeaks();
				ExitProcess(1);
			}
				
				
				
            }
		break;


		case WM_COMMAND: //RH command keys
			switch(LOWORD(wParam))
			{
				case  IDC_TEXT:
					{
					//validation done elsewhere //check out WM_GETDLGCODE
					}
				break;

				case IDC_NUMBER:
					{
					//no greater than 32760
					}
				break;

				case IDC_ADD: //adds directories nested ntimes
				{
					//http://www.experts-exchange.com/Programming/Languages/.NET/Visual_CPP/Q_27207428.html
					//On the first call of IDC_ADD change text & button enables. 
					//Add button disabled until a successful create or Clear
					//FIX LATER


					int len;
					wchar_t* buf;



					currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					if (currPathW == nullptr)
					{
						/* We were not so display a message */
					errCode = -1;
					DisplayError (hwnd, L"Could not allocate required memory", errCode, 0);
					goto NoAddSuccess;
					}

					SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"This entry is repeated");

					SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"times.");





					// When somebody clicks the Add button, first we get the number of
					// they entered

					BOOL bSuccess;
					int nTimes = GetDlgItemInt(hwnd, IDC_NUMBER, &bSuccess, FALSE);
					if(bSuccess) 
					{
						// Then we get the string they entered
						// First we need to find out how long it is so that we can
						// allocate some memory (2* +1 for two words > long)

						len = (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT))); //wchar
						if(len > 0)
						{
							// Now we allocate, and get the string into our buffer


							
							wchar_t *buf1 = (wchar_t *)calloc(2 * len + 1, sizeof(wchar_t));
							GetDlgItemTextW(hwnd, IDC_TEXT, buf1, 2 * len + 1);



						//validation for terminating space & period
			
							for(i = len-1; i >= 0; i--)
							{

								if (!(wcsncmp(&buf1[i], L" ", 1 )) || !(wcsncmp(&buf1[i], L".", 1)) )
								{
									wcscpy_s(&buf1[i], i, L"\0");
									SetDlgItemTextW(hwnd, IDC_TEXT, (wchar_t*)(buf1));
								}

								else

								{
								break; //all good
								}
							}
						free(buf1);
						}

						int len = 2 * (GetWindowTextLength(GetDlgItem(hwnd, IDC_TEXT)) + 1); //wchar again
						if(len > 0)
						{
						buf = (wchar_t*)GlobalAlloc(GPTR, len );
						GetDlgItemTextW(hwnd, IDC_TEXT, buf, len);


							// Now we add the string to the list box however many times user asked us to.

							for(i = 0 ; i < nTimes; i++)
							{
								sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_ADDSTRING, 0, (LPARAM)buf);

								// Here we are associating the value nTimes with the item 
								// just for the heck of it, we'll use it to display later.
								// Normally you would put some more useful data here, such
								// as a pointer.
								sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETITEMDATA, (WPARAM)sendMessageErr, (LPARAM)nTimes);
							}

							// free the memory!
							GlobalFree((HANDLE)buf);
							sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
							SetDlgItemInt(hwnd, IDC_SHOWCOUNT, nTimes, FALSE);

						}
						else 
						{
							errCode = 0;
							DisplayError (hwnd, L"You didn't enter anything!", errCode, 0);
							goto NoAddSuccess;
						}
					}
					else 
					{
						errCode = 0;
						DisplayError (hwnd, L"Couldn't translate that number :(", errCode, 0);
						goto NoAddSuccess;
					}


					if (foundNTDLL)
					{
					//update branchTotal: always 0 for the first branch

					//populate after the save file contents
						if (branchTotal < branchLimit - 1)
						{
						branchTotal +=1;
						}
						else
						{
						DisplayError (hwnd, L"Limit of number of directories reached. Cannot create anymore!", errCode, 0);
						goto NoAddSuccess;
						}

						(branchLevelClick) ? EnableWindow(GetDlgItem(hwnd, IDC_DOWN), true) : EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
						//next add is always at base
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
						HWND hList = GetDlgItem(hwnd, IDC_LIST);
						listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
						currPathW[0] = L'\0';
						branchLevel = 0;

						//check on bounds

						for (i = folderdirCS + folderdirCW + branchLevelCum; i < listTotal; i++)

						{

							if (branchLevelClick + branchLevel < treeLevelLimit)
							{
							
							sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
							wcscpy_s(folderTreeArray[branchTotal][branchLevelClick + branchLevel], maxPathFolder, (wchar_t *) currPathW); //branchLevelClickOld can be neg?
							branchLevel += 1;
							}
							else
							{
							DisplayError (hwnd, L"Limit of number of nested directories reached. Cannot create anymore!", errCode, 0);
							goto NoAddSuccess;
							}
						}

						//clear redundant branches
						for (j = branchLevelClick + branchLevel; j < treeLevelLimit; j++)
						{
						folderTreeArray[branchTotal][j][0] = L'\0';
						}

						//save branchLevelClickOld & branchLevel
						trackFTA [branchTotal][0] = branchLevelClick;
						trackFTA [branchTotal][1] = branchLevel; //the sum of these is total no of backslashes for validation
						

						
						branchLevelClickOld = branchLevelClick;


						for (j = 0; j <= branchLevelClick + branchLevel; j++)
						{
						//branchTotal's next iteration only
						wcscpy_s(folderTreeArray[branchTotal + 1][j], maxPathFolder, folderTreeArray[branchTotal][j]); //populate the entire string
						}
						

						branchLevelCum += branchLevel; //number of items added to list
					}
					else
					{
					DisplayError (hwnd, L"NTDLL not found: Only one nested path is made with CREATE.", errCode, 0);
					EnableWindow(GetDlgItem(hwnd, IDC_UP), false);
					EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
					}
				
				removeButtonEnabled = false;
				EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);

				NoAddSuccess:
				free (currPathW);
				EnableWindow(GetDlgItem(hwnd, IDC_CREATE), true);


				}
				break;


				case IDC_UP: //adds directories nested ntimes
					//rule is cannot go back up a tree once we have branched.
				{
					//check validity with branchLevelClickOld + branchLevel and grey out
					SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
					branchLevelClick +=1;
					branchLevelIncCum = 0;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					EnableWindow(GetDlgItem(hwnd, IDC_DOWN), true);
					if (branchLevelClick == branchLevel + branchLevelClickOld ) EnableWindow(GetDlgItem(hwnd, IDC_UP), false);


					for (i = branchTotal; i >= ((createFail)? branchTotalCumOld + 1: 0); i--)
					{
					branchLevelIncCum = branchLevelIncCum + trackFTA [i][1];
						if (branchLevelClick == trackFTA [i][0] + 1) //always satisfied on new branch
						{
							branchSaveI = i;
							branchLevelInc = 0;
							branchLevelIncCum = branchLevelCum - branchLevelIncCum; //start before last nesting here
							break;
						}
						else
						{
							if ((branchLevelClick > trackFTA [i][0] + 1) && (branchSaveI == i))
							{
							branchLevelInc += 1; //always 1 less than item
							branchLevelIncCum = branchLevelCum - (branchLevelIncCum - branchLevelInc);
							break;
							}
						}

					}



					if (branchLevelClick > 0)
					{
						(branchSaveI == branchTotal) ? EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true): EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((folderdirCS + folderdirCW + branchLevelIncCum)), 0);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(folderdirCS + folderdirCW + branchLevelIncCum));
						//SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(folderdirCS + folderdirCW + branchLevelCum - (branchLevel - branchLevelClick))); //DOES NOT WORK!
					}


				}
				break;

				case IDC_DOWN: //adds directories nested ntimes: branchLevelClick never < 0
				{
					SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
					branchLevelClick -=1;
					branchLevelIncCum = 0;
					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					if (branchLevelClick == ((createFail)? trackFTA [branchTotalCumOld + 1][0]: 0) )
					{
						branchLevelInc -= 1;
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
					}
					else 
						{


							for (i = branchTotal; i >= ((createFail)? branchTotalCumOld + 1: 0); i--)
							{
							branchLevelIncCum = branchLevelIncCum + trackFTA [i][1];
								if (branchLevelClick == trackFTA [i][0]) //always satisfied on new branch
								{
									branchSaveI = i - 1; //go to next branch down
									branchLevelInc = trackFTA [i][0] - trackFTA [i - 1][0] - 1;
									branchLevelIncCum = branchLevelCum - (branchLevelIncCum + trackFTA [i - 1][1] - branchLevelInc); //start before last nesting here
									break;
								}
								else
								{
									if ((branchLevelClick > trackFTA [i][0]) && (branchSaveI == i))
									{
									branchLevelInc -= 1; //always 1 less than item
									branchLevelIncCum = branchLevelCum - (branchLevelIncCum - branchLevelInc);
									break;
									}
								}


							}

						(branchSaveI == branchTotal) ? EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true): EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
						EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
						sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((folderdirCS + folderdirCW + branchLevelIncCum)), 0);
						sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)(folderdirCS + folderdirCW + branchLevelIncCum));
						}

				}
				break;




			case IDC_CREATE:
				{
				NTSTATUS ntStatus;
				int jMax = 0;
				HWND hList = GetDlgItem(hwnd, IDC_LIST);
				//get total for loop
				listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

				//wcscpy_s(cumPath, pathLength, L"\\\\\?\\C:\\"); //


				if (!SetCurrentDirectoryW(driveIDBaseW))
					{
					errCode = 1;
					ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
					goto EndCreate;
					}



					if (foundNTDLL)
					{
						currPathW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
						if (currPathW == nullptr)
							{
							/* We were not so display a message */
							errCode = -1;
							DisplayError (hwnd, L"Could not allocate required memory to initialize String.", errCode, 0);
							goto EndCreate;
							}
						
						(!branchTotal && createFail)? branchTotalCum = -1: branchTotalCum = 0;
						//Load FS into branchTotalSaveFile + 1 (appendMode true so FS loaded after)
						if (!ProcessfileSystem (hwnd, false, true))
						{
							if (!DisplayError (hwnd, L"Problem with FS file! Try alternate Create?", 0, 1))
							{
								goto AltCreate;
							}
							else
							{
								errCode = 1;
								goto EndCreate;
							}
						}

						//convert to single folder items: -previously created folders done
						for (i = (createFail)? branchTotalCumOld + 1: 0; i <= branchTotal; i++)
						{

							for (j = 0; (j < trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != '\0'); j++)
							{
								if (j != 0) wcscat_s(pathsToSave[i], pathLength, &separatorFTA);
								wcscat_s(pathsToSave[i], pathLength, folderTreeArray[i][j]);
							}

						}

						//Validation check for dups
						//compare all folderTreeArray items that have the same trackFTA
						for (i = 0; i <= branchTotalSaveFile; i++)
						{
							jMax = trackFTA [i][0] + trackFTA [i][1];
							for (j = 0; j <= jMax; j++)	
							{
								if (jMax == j)
									{
										for (k = 0; k < i; k++)
										{
											if (0 == wcscmp(pathsToSave[i], pathsToSave[k]))
											{
												pathsToSave[k][0] = L'\0';
												trackFTA [k][0] = 0;
												trackFTA [k][1] = 0;
												for (int l = 0; l < treeLevelLimit; l++) folderTreeArray[k][l][0] = L'\0';
											} 
										}

									}
							}

						}
						




						
						for (i = ((createFail)? branchTotalCumOld + 1: 0); i <= branchTotal; i++)
						{


						wcscpy_s(currPathW, pathLength, driveIDBaseWNT); //maxPathFolder too small for destination

						for (j = 0; (j <  trackFTA [i][0] + trackFTA [i][1]) && (folderTreeArray[i][j][0] != L'\0'); j++)
						//cannot create the entire nested path at once or get a "status_wait_3" see KeWaitForMultipleObjects routine
						{
						
						
						wcscat_s(currPathW, pathLength, folderTreeArray[i][j]);
						wcscat_s(currPathW, pathLength, &separatorFTA);
						if (j >= trackFTA [i][0]) //create dir if new path
						{


						if (DynamicLoader (false, currPathW))
						{

						
						//Do not specify FILE_READ_DATA, FILE_WRITE_DATA, FILE_APPEND_DATA, or FILE_EXECUTE 
						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | SYNCHRONIZE, &fileObject, &ioStatus, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_CREATE, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT, nullptr, 0);

						PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
						if ( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) )
						{
							ErrorExit (L"RtlNtStatusToDosError: Problem!", 0);
							errCode = 1;
							goto EndCreate;
						}
						DWORD Status = RtlNtStatusToDosError (ntStatus);

						switch ((DWORD)(ntStatus) >> 30)
						{
						case 0: //NT_SUCCESS
							{

							}
						break;
						case 1: //NT_INFORMATION
							{
								DisplayError (hwnd, L"Informational: No error ", Status, 0);
							}
						break;
						case 2: //NT_WARNING 
							{
								ErrorExit (L"NtCreateFile: ", Status);
							}
						break;
						case 3://NT_ERROR
							{
								
								if ((Status == 87) && (createFail == true))
								{
									DisplayError (hwnd, L"There was another error prior to this on directory create. The create function is not available. Try again after deleting a line or clearing the list.", Status, 0);
								}
								else
								{
									ErrorExit (L"NtCreateFile: ", Status);
								}
								createFail = true;
								errCode = 1;
								goto EndCreate;
							}
						}
						//SetCurrentDirectory Often fails here at root node with error 32 "used by another process"

						}


						else
						{
							errCode = 1;
							createFail = true;
							ErrorExit (L"DynamicLoader failed: Cannot create!", 1);
							goto EndCreate;
						}
					} //trackFTA condition
					}
						if (folderTreeArray[i][0][0] != L'\0') branchTotalCum +=1; // for rollback
					}

					//sort all and write to file

					if (!ProcessfileSystem(hwnd, true, true))
						{
							DisplayError (hwnd, L"There was an error writing data to file. This program may not be able to delete directories just created. If their deletion is required in the future, run 7-zip and shift-del.", errCode, 0);
							goto EndCreate;
						}
				} //foundNtdll
				goto EndCreate;

				AltCreate:

				currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));

				if (currPathW == nullptr)
					{
					/* We were not so display a message */
					errCode = -1;
					DisplayError (hwnd, L"Could not allocate required memory to initialize String.", errCode, 0);
					goto EndCreate;
					}


				//Another loop & variables for recursive create here
				for (i = folderdirCS + folderdirCW; i < listTotal; i++)
				{
				sendMessageErr = SendMessageW(hList, LB_GETTEXT, i, (LPARAM)currPathW);
				//check for double click https://msdn.microsoft.com/en-us/library/windows/desktop/bb775153(v=vs.85).aspx 


				// cannot use cumPath: http://stackoverflow.com/questions/33018732/could-not-find-path-specified-createdirectoryw/33050214#33050214
				//wcscat_s(cumPath, pathLength, currPathW);
						
						wcscat_s(currPathW, maxPathFolder, &separatorFTA);
						if (exe64Bit)
						{
						if (CreateDirectoryW(currPathW, nullptr)) 
							{
								errCode = 0;
								if (!SetCurrentDirectoryW(currPathW))
								{
								errCode = 1;
								createFail = true;
								ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
								goto EndCreate;
								}
							}
						else
							{
								errCode = 1;
								createFail = true;
								ErrorExit (L"CreateDirectoryW: ", 0);
								goto EndCreate;
							}
						}
						else
						{
							if (Wow64DisableWow64FsRedirection(&OldValue))
							{
								if (CreateDirectoryW(currPathW, nullptr)) 
								{
								errCode = 0;
									if (!SetCurrentDirectoryW(currPathW))
									{
									errCode = 1;
									createFail = true;
									ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
									goto EndCreate;
									}
								}
							else
								{
								errCode = 1;
								createFail = true;
								ErrorExit (L"CreateDirectoryW: ", 0);
								goto EndCreate;
								}
							}
							if (!Wow64RevertWow64FsRedirection(&OldValue))
							{
							DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
							goto EndCreate;
							}

						}


					}


				//There is a default string size limit for paths of 248 characters
				//errCode = CreateDirectoryW(cumPath, NULL);

				//wcscpy_s(currPathW, maxPathFolder, driveIDBaseW);
				//\a  audible bell

				//LB_GETTEXTLEN  https://msdn.microsoft.com/en-us/library/windows/desktop/bb761315(v=vs.85).aspx
					

				//longPathName
				//Clear all the added items
				EndCreate:
				if (foundNTDLL)
				{
					if (hdlNTOut) CloseHandle (hdlNTOut);
					memset(&ioStatus, 0, sizeof(ioStatus));
					memset(&fileObject, 0, sizeof(fileObject));
					FreeLibrary ((HMODULE)hdlNtCreateFile);
					ntStatus = NULL;
				}

				if (createFail)
				{
					//Write the first successful block, but if second error don't write same stuff again
					if (branchTotalCum > 0)
					{

						if (!ProcessfileSystem(hwnd, true, true)) DisplayError (hwnd, L"There was another error, this time writing data to file. This program may not be able to delete the created directories. To do so run 7-zip and shift-del.", errCode, 0);
					
						k = 0;
						int l = folderdirCS + folderdirCW;


						for (i = branchTotalCumOld; i < branchTotalCum; i++)
						{


							listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);

							//set delete LB sel items up to cum
							
							for (j = l; (j < l + trackFTA [i][1]); j++)
							{
								sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)TRUE, (LPARAM)j);
							}


							if ((sendMessageErr != LB_ERR) && (trackFTA [i][1] > 0))
							{
								l += trackFTA [i][1];

								currPathW[0] = L'\0';
								wcscpy_s(currPathW, pathLength, driveIDBaseW);
							
								if (branchTotalCumOld + 1 != branchTotalCum)
								{
									if (!wcsncmp(folderTreeArray[i][0], folderTreeArray[i + 1][0], 1 ) && (folderTreeArray[i][0][0] != L'\0')) //0 match
									{
									wcscat_s(currPathW, pathLength, folderTreeArray[i + 1][0]);
									sendMessageErr = SendMessageW(hList, LB_INSERTSTRING, (WPARAM)(folderdirCS + folderdirCW + k), (LPARAM)currPathW);
									k += 1;
									}

								}
								else
								{
									wcscat_s(currPathW, pathLength, folderTreeArray[branchTotalCum][0]);
									sendMessageErr = SendMessageW(hList, LB_INSERTSTRING, (WPARAM)(folderdirCS + folderdirCW), (LPARAM)currPathW);
									k = 1;
								}
							}
						}

						//now the deletion
						int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
						int *buf = (int*)GlobalAlloc(GPTR, sizeof(int) * count);

						sendMessageErr = SendMessageW(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);
						for (i = count - 1; i >= 0; i--)
						{
							//Some corruption in the memory (wouldn't be compiler?) causing errCode to reset to zero when the following function is invoked. No idea what it is.
							sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
							branchLevelCum -=1;
						}
						GlobalFree(buf);

						errCode = 1; //reset errCode: what's going on here?


						//A bugfree Rollback for the current setup is_a_major_drama. A little wonky for the time being.
						folderdirCW +=k;
						branchTotal = branchTotalCum;

						branchTotalCumOld += (branchTotalCum - 1); //for next possible iteration of Create/fail
						listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);


					}
					//No folders created last pass so do nothing
					else
					{
						if (!branchTotalCumOld) branchTotalCumOld = -1;
					}
				}
				else
				{
					branchTotalCum = 0;
				}

				free(currPathW);
				//free(cumPath);
				if (errCode == 0) //succeeded
				{
				sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
				PopulateList(hwnd);
				removeButtonEnabled = true;
				EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);
				}
				else
				{
				errCode = 0;
				}
				}
				break;


				case IDC_REMOVE:
				{
					//When the user clicks the Remove button, we first get the number of selected items

					HWND hList = GetDlgItem(hwnd, IDC_LIST);
					int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
					listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
					if (count != LB_ERR)
					{
						
						sendMessageErr = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh

						//index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
						if (count == 1 && (index < (folderdirCS + folderdirCW)))
						{
							
						FSDeleteInit (hwnd, hList);

						}
						else

						{
							if (count != 0)
							{

								// And then allocate room to store the list of selected items.


								int *buf = (int*)GlobalAlloc(GPTR, sizeof(int) * count);
								sendMessageErr = SendMessageW(hList, LB_GETSELITEMS, (WPARAM)count, (LPARAM)buf);
								//index = SendMessageW(hList, LB_GETCURSEL, 0, 0L) + 1; //+ 1 ???
								// Now we loop through the list and remove each item that was selected, looping backwards, because if we removed items from top to bottom, it would change the indexes of the other items!!!

								for (i = count - 1; i >= 0; i--)
								{
									sendMessageErr = SendMessageW(hList, LB_DELETESTRING, (WPARAM)buf[i], 0);
								}

								GlobalFree(buf);


								
								branchLevelCum -= 1;
								branchLevel -= 1;
								if (!branchLevelIncCum) branchLevelIncCum = branchLevel;
								if (branchLevelInc) branchLevelInc -= 1;
								
								index =  index + 1 - (folderdirCS + folderdirCW + branchLevelCum - branchLevel);

								
																
								trackFTA [branchTotal][1] -= 1;
								if (index <= branchLevelClick - branchLevelClickOld)
									{
									if (branchLevelClick) branchLevelClick -= 1;
									trackFTA [branchTotal][0] -= 1;
									}
								if (branchLevelClick)
									{
									EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
									}
								else 
									{
										removeButtonEnabled = true; //Last branch removed
										EnableWindow(GetDlgItem(hwnd, IDC_DOWN), false);
										EnableWindow(GetDlgItem(hwnd, IDC_UP), true);
									}
								sendMessageErr = SendMessageW(hList, LB_SETSEL, (WPARAM)FALSE, (LPARAM)(-1));
								sendMessageErr = SendMessageW(hList, LB_SETTOPINDEX, (WPARAM)((folderdirCS + folderdirCW + branchLevelIncCum)), 0);
								listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
								


							}
							else
							{
								errCode = 0;
								DisplayError (hwnd, L"No items selected...", errCode, 0);
							}
						}
					}
					else
					{
						errCode = 0;
						DisplayError (hwnd, L"Error counting items...", errCode, 0);
					}
			}
			break;
			case IDC_CLEAR:
				{
				sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
				PopulateList(hwnd);
				}
			break;

				case IDC_LOGON:
				{

					thisexePath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
					ExpandEnvironmentStringsW(L"%systemroot%", tempDest, pathLength);
					wcscat_s(tempDest, pathLength, L"\\Temp\\CreateLargeDir64.exe");
					if (GetCreateLargeDirPath (hwnd, thisexePath) == 1)
					{
							ErrorExit (L"GetCreateLargeDirPath: Problem with program copy.", 0);
							break;
					}


					if (CopyFileW(thisexePath, tempDest, FALSE) == 0)
					{
						ErrorExit (L"CopyFile: Copy to Temp failed... aborting.", 0);
						EnableWindow(GetDlgItem(hwnd, IDC_LOGON), false);
						EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), false);
						free (thisexePath);
						free (tempDest);
						break;
					}


				system ("CD\\ & PUSHD %SystemRoot%\\Temp & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & (IF DEFINED OSB (FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% /reg:64 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\") ELSE ((FOR /F \"USEBACKQ SKIP=2 TOKENS=1-4 DELIMS= \" %G IN (`REG QUERY %KEY_NAME% /v %VALUE_NAME% 2^>Userinitregerror.txt`) DO @SET \"CURREGVALUE=%I%J\"))) & >NUL FINDSTR \"^\" \"Userinitregerror.txt\" && SET \"ERRTXT=\" || SET \"ERRTXT=1\" & (IF DEFINED ERRTXT (>Userinitreg.txt CALL ECHO %CURREGVALUE% & (IF '%errorlevel%' NEQ '0' (CALL ECHO Copy reg record failed! & PAUSE >NUL))) ELSE (ECHO No reg key! & PAUSE NUL)) & (IF DEFINED OSB (CALL CALL SET \"NEWREGVALUE=%SystemRoot%\\Temp\\CreateLargeDir64.exe\") ELSE (CALL CALL SET \"NEWREGVALUE=%CURREGVALUE:%SystemRoot%\\system32\\userinit.exe=%SystemRoot%\\Temp\\CreateLargeDir64.exe,%SystemRoot%\\system32\\userinit.exe%\")) & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %NEWREGVALUE% /f /reg:64 & POPD");
					
				free (thisexePath);
				free (tempDest);
				EnableWindow(GetDlgItem(hwnd, IDC_LOGON), false);

				//NOTE WOW6432node is 64bit view of 32bit setting. reg:64 bypasses VS 32bit redirection
				//Debug & CALL ECHO %KEY_NAME% %VALUE_NAME% %CURREGVALUE% %NEWREGVALUE%
				}
			break;
		
				case IDC_NOLOGON:
			{
					//hFind = FindFirstFile("%systemroot%\Temp\Userinitreg.txt", &FindFileData);
					//https://msdn.microsoft.com/en-us/library/windows/desktop/aa365743(v=vs.85).aspx
					//If you are writing a 32-bit application to list all the files in a directory and the application may be run
					//on a 64-bit computer, you should call the Wow64DisableWow64FsRedirectionfunction before calling FindFirstFile
					//and call Wow64RevertWow64FsRedirection after the last call to FindNextFile.
			
			if (setforDeletion==TRUE)
			{
			if (!DisplayError (hwnd, L"The PendingFileRenameOperations key already has data. Please reply no and check they key's value if unsure whether another program besides this one has marked another file for deletion at reboot.", errCode, 1)) break;
			
			//delete the key ExistRegValue
			system ("REG DELETE \"HKLM\\System\\CurrentControlSet\\Control\\Session Manager\" /v PendingFileRenameOperations /f");

			}
			
			if (Kleenup (hwnd, weareatBoot))
			{
				EnableWindow(GetDlgItem(hwnd, IDC_NOLOGON), false);
				EnableWindow(GetDlgItem(hwnd, IDC_LOGON), true);
				setforDeletion = TRUE;
			}
			else
			{
				setforDeletion = FALSE;
			}

			//

			

			}
			break;
			case IDC_HALP: //HALP used because tlhelp32 produces a c4005 macro redefinition warning for HELP
			{
			}
			break;
			case IDC_LIST:

				switch(HIWORD(wParam))
				{
					case LBN_SELCHANGE:
					{
						// Get the number of items selected.

						HWND hList = GetDlgItem(hwnd, IDC_LIST);
						int count = SendMessageW(hList, LB_GETSELCOUNT, 0, 0);
						bool removeTrig = false;
						if(count != LB_ERR)
						{
							// We only want to continue if one and only one item is
							// selected.

							if(count == 1)
							{

								//place string into text box
								index = SendMessageW(hList, LB_GETCURSEL, 0, 0L);
								currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
								if ((currPathW == nullptr) || (index == LB_ERR))
								{
									DisplayError (hwnd, L"Something has gone wrong with memory!", errCode, 0);
									return 0;
								}
								sendMessageErr = SendMessageW(hList, LB_GETTEXT, index, (LPARAM)currPathW);
								SetDlgItemTextW(hwnd, IDC_TEXT, currPathW);
								free (currPathW);

								// Since we know ahead of time we're only getting one index, there's no need to allocate an array.
									
								//sendMessageErr = SendMessageW(hList, LB_GETCURSEL, (WPARAM)1, (LPARAM)&index); //GETSELITEMS substituted with LB_GETCURSEL for a laugh
									

								if (index >= folderdirCS + folderdirCW)
								{
									SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
									if (index >= listTotal - branchLevel)
									{
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
									}
									else
									{
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
									}
								}
								else
								{

									SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");

									if (wcsstr(dacfoldersW[index], lpref) == nullptr)
										//Check for wide string folder here
									{
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), removeButtonEnabled);
									}
									else
									{
											//Cannot remove short folders
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
									}
								}


								if (sendMessageErr != LB_ERR)
								{
									// Get the data we associated with the item above (the number of times it was added)
									idata = SendMessageW(hList, LB_GETITEMDATA, (WPARAM)index, 0); //lparam not used, but return value IS value of lparam in setitemdata
									//SO idata becomes ntimes when items are added, but not reliable! http://stackoverflow.com/questions/25337801/why-is-lb-getitemdata-returning-0

									if (idata)
									{
										SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"This entry is repeated");
										SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"times.");
										SetDlgItemInt(hwnd, IDC_SHOWCOUNT, idata, FALSE);
									}
									
									else
									{
										SetDlgItemInt(hwnd, IDC_SHOWCOUNT, index, FALSE);
										SetDlgItemTextW(hwnd,IDC_STATIC_ONE, L"This entry is ranked");
										SetDlgItemTextW(hwnd,IDC_STATIC_TWO, L"on the list.");
									}
									//This function performs like:
									//TCHAR buf[16];
									//wnsprintf(buf, 16, bSigned ? TEXT("%i") : TEXT("%u"), uValue);
									//SetDlgItemText(hDlg, nIDDlgItem, buf);



								}
								else 
								{
									errCode = 0;
									DisplayError (hwnd, L"Error getting selected item :(", errCode, 0);
								}
							}
							else 
							{
								// No items selected, or more than one.Either way, we aren't going to process this.
									
								sendMessageErr = SendMessageW(hList, LB_GETANCHORINDEX, 0, 0L);
								int selItems[32767];
								sendMessageErr = SendMessage(hwnd, LB_GETSELITEMS, count, (LPARAM)selItems);

								if (listTotal > folderdirCS + folderdirCW)
								{
									for (i = 0; i < count; i++)
									{
										if (selItems[i] < listTotal - branchLevel)
										{
											EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
											SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del\0");
											removeTrig = true;
										}

									}
									if (!removeTrig)
									{
									SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Line\0");
									EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
									}
								}
								else
								{
									for (i = 0; i < count; i++)
									{
										if (selItems[i] < folderdirCS)
										{
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), false);
										removeTrig = true;
										}

									}
										if (!removeTrig)
										{
										SetWindowTextW(GetDlgItem(hwnd, IDC_REMOVE), L"Del Dir\0");
										EnableWindow(GetDlgItem(hwnd, IDC_REMOVE), true);
										}
										
								}




								SetDlgItemTextW(hwnd, IDC_SHOWCOUNT, L"-");
							}
						}
						else
						{
							errCode = 0;
							DisplayError (hwnd, L"Error counting items :(", errCode, 0);
						}
					}
					break; //This break for consistency
				}
			break;
			} //end WM_COMMAND
		break;
		case WM_CLOSE:
			{
			//Cleanup
			if (weareatBoot) Kleenup (hwnd, weareatBoot);
			 
			if (exeHandle != INVALID_HANDLE_VALUE) CloseHandle(exeHandle);
			EndDialog(hwnd, 0);
			_CrtDumpMemoryLeaks();
			}
		break;

		case WM_DESTROY: PostQuitMessage(0); return TRUE;
		break;
		default: return FALSE;
		break;	
	}
	return TRUE;
}


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	
	
	if (pCmdLine[0] != L'\0')
	{
	//also https://msdn.microsoft.com/en-us/library/windows/desktop/bb776391%28v=vs.85%29.aspx?f=255&MSPPError=-2147217396
	memset(rootDir, L'\0', sizeof(rootDir));
	wcscpy_s (rootDir, pathLength, (wchar_t *) pCmdLine);
	pCmdLineActive = true;
	}
	//else
	


 WNDCLASS wc = { };

 wc.lpfnWndProc   = WindowProc;
 wc.hInstance     = hInstance;
 wc.lpszClassName = CLASS_NAME;

 RegisterClass(&wc);

    // Create the window.

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles.
        CLASS_NAME,                     // Window class
        L"Learn to Program Windows",    // Window text
        WS_OVERLAPPEDWINDOW,            // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

        NULL,       // Parent window    
        NULL,       // Menu
        hInstance,  // Instance handle
        NULL        // Additional application data
        );

    if (hwnd == NULL)
    {
        return 0;
    }


	
	
	
	
	lpTemplate = (LPCDLGTEMPLATE)IDD_768P;
	return DialogBoxW(hInstance, MAKEINTRESOURCEW(lpTemplate), nullptr, DlgProc);



}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
	PostQuitMessage(0);
	return 0;
	
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


LPCDLGTEMPLATE DoSystemParametersInfoStuff(HWND hwnd)
{
	HMONITOR hMon = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
	return lpTemplate;
}


int GetCreateLargeDirPath (HWND hwnd, wchar_t *exePath)
{
DWORD result;



    //  Disable redirection immediately prior to the native API
    //  function call.
    
if (exe64Bit)
{
		result  = GetModuleFileNameW(nullptr, exePath, MAX_PATH-4);
	    errCode = (int)GetLastError();

}
else
{
	if( Wow64DisableWow64FsRedirection(&OldValue) ) 
    {
	result  = GetModuleFileNameW(nullptr, exePath, MAX_PATH-4);
    errCode = (int)GetLastError();
	}
	else
	{
		DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
		return 1;
	}
    if (!Wow64RevertWow64FsRedirection(&OldValue) ) 
	{
		DisplayError (hwnd, L"Problems with redirection...", errCode, 0);
		return 1;
	}
}

    if (0 == result)
    {
        exePath = 0;
        return 1;
    }
    else if (result == (int)MAX_PATH-4)
    {
        /* May need to also check for ERROR_SUCCESS here if XP/2K */
        if (ERROR_INSUFFICIENT_BUFFER != errCode)
        {
            return 1;
        }
    }


if (!exePath)
{
    switch (errCode)
	{
		case 2:
			DisplayError (hwnd, L"Problems: FILE_NOT_FOUND", errCode, 0);
		break;
		case 50:
			DisplayError (hwnd, L"Problems: NOT_SUPPORTED", errCode, 0);
		break;
		case 110:
			DisplayError (hwnd, L"Problems: OPEN_FAILED", errCode, 0);
		break;
		case 113:
			DisplayError (hwnd, L"Problems: NO_MORE_SEARCH_HANDLES", errCode, 0);
		break;
		case 114:
			DisplayError (hwnd, L"Problems: INVALID_TARGET_HANDLE", errCode, 0);
		break;
		case 123:
			DisplayError (hwnd, L"Problems: INVALID_NAME", errCode, 0);
		break;
		case 126:
			DisplayError (hwnd, L"Problems: MODULE_NOT_FOUND", errCode, 0);
		break;
		case 259:
			DisplayError (hwnd, L"Problems: NO_MORE_ITEMS", errCode, 0);
		break;
		case 303:
			DisplayError (hwnd, L"Problems: DELETE_PENDING", errCode, 0);
		break;
		case 310:
			DisplayError (hwnd, L"Problems: INVALID_EXCEPTION_HANDLER", errCode, 0);
		break;
		case 335:
			DisplayError (hwnd, L"Problems: Cannot run this out of an archive: COMPRESSED_FILE_NOT_SUPPORTED", errCode, 0);
		break;
		default:
			{
			DisplayError (hwnd, L"Unknown error has occurred.", errCode, 0);
			}
	}
	return 1;
}
else
{
	return 0;
}
}
bool Kleenup (HWND hwnd, bool weareatBoot)
{
	STARTUPINFOW lpStartupInfo;
	PROCESS_INFORMATION lpProcessInfo;

	ZeroMemory(&lpStartupInfo, sizeof(lpStartupInfo));
	ZeroMemory (&lpStartupInfo, sizeof(lpStartupInfo));
	thisexePath = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));

	if (pCmdLineActive) //on program restart on remove root directory bug
	{
		if (!GetModuleFileNameW(nullptr, thisexePath, pathLength) || (wcslen(thisexePath) > pathLength))
		{
			DisplayError (hwnd, L"Oops, process path too long!? or non-existent?! Quitting...", 0, 0);
			free (tempDest);
			free (thisexePath);
			return false;
		}
		else
		{
		wcscpy_s (tempDest, pathLength, L"\"");
		wcscat_s (tempDest, pathLength, thisexePath);
		wcscat_s (tempDest, pathLength, L"\" ");
		wcscat_s (tempDest, pathLength, pathToDeleteW);
		if (!CreateProcessW (thisexePath, tempDest, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) ErrorExit (L"Oops: Something went wrong. Please restart the program...", 0);
		free (tempDest);
		free (thisexePath);
		CloseHandle(lpProcessInfo.hProcess);
		CloseHandle(lpProcessInfo.hThread);
		}
	}
	else
	{

			system ("CD\\ & PUSHD %SystemRoot%\\Temp & REG QUERY \"HKLM\\Hardware\\Description\\System\\CentralProcessor\\0\" | FIND /i \"x86\" >NUL && CALL SET \"OSB=\" || CALL SET \"OSB=64BIT\" & SET KEY_NAME=\"HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" & SET \"VALUE_NAME=Userinit\" & SET \"Userinitreg=\" & (IF EXIST Userinitreg.txt CALL SET \"Userinitreg=TRUE\") & (IF DEFINED Userinitreg (FOR /F \"usebackq tokens=1,2* delims=,\" %G IN (\"Userinitreg.txt\") DO SET \"REGVALUE=%G\" & (IF DEFINED OSB (CALL SET \"REGVALUE=%REGVALUE%,\" & CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f /reg:64) ELSE (CALL REG ADD %KEY_NAME% /v %VALUE_NAME% /d %REGVALUE% /f))) ELSE (CALL ECHO Backup reg record does not exist & PAUSE >NUL)) & (IF EXIST Userinitregerror.txt (DEL \"Userinitregerror.txt\")) & DEL \"Userinitreg.txt\" & POPD");
			
			if (!ExpandEnvironmentStringsW(L"%systemroot%", tempDest,  pathLength)) ErrorExit (L"ExpandEnvironmentStringsW failed.", 0);
			wcscpy_s(thisexePath, pathLength, tempDest); //Small hole in logic here
			wcscat_s(tempDest, pathLength, L"\\Temp\\CreateLargeDir64.exe");

			if (weareatBoot)
				{
				//reset to vanilla
				wcscat_s(thisexePath, pathLength, L"\\system32\\userinit.exe");
				//Create process userinit.exe


				SetLastError(ERROR_INVALID_PARAMETER); //https://msdn.microsoft.com/en-us/library/ms682425(VS.85).aspx
				if (!CreateProcessW(thisexePath, nullptr, nullptr, nullptr, FALSE, NULL, nullptr, nullptr, &lpStartupInfo, &lpProcessInfo)) ErrorExit (L"userinit could not be started through this program. Please reboot after closing this program.", 0);
				//The reg value is restored to userinit before theis point
				}
			if(!MoveFileExW(tempDest,nullptr,MOVEFILE_DELAY_UNTIL_REBOOT))
			{
			DisplayError (hwnd, L"Problems with file deletion. Solved with next Disk Cleanup...", 0, 0);
			free (tempDest);
			free (thisexePath);
			return false;
			}
			else
			{
			free (tempDest);
			free (thisexePath);
			}
	}
	return true;
}
int ExistRegValue ()
{
	wchar_t *keyName;
	wchar_t *valueName;
	keyName = (wchar_t *)calloc(260, sizeof(wchar_t));
	valueName = (wchar_t *)calloc(260, sizeof(wchar_t));
	wcscpy_s(keyName, 260, L"HKLM\\System\\CurrentControlSet\\Control\\Session Manager");
	wcscpy_s(valueName, 260, L"PendingFileRenameOperations");

	if (RegQueryValueExW((HKEY)keyName, valueName, nullptr, nullptr, nullptr, nullptr) == ERROR_FILE_NOT_FOUND)
	{
		free (keyName);
		free (valueName);
		return 0;
	}
	else
	{
		free (keyName);
		free (valueName);
		return 1;
	}

	// \??\C:\Windows\Temp\CreateLargeDir64.exe: nasty multo_sz

}
DWORD FindProcessId(HWND hwnd, const wchar_t *processName, HANDLE hProcessName)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;
    DWORD result = NULL;

    // Take a snapshot of all processes in the system.
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hProcessSnap) return(FALSE);

    pe32.dwSize = sizeof(PROCESSENTRY32); // <----- IMPORTANT

    // Retrieve information about the first process,
    // and exit if unsuccessful
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);          // clean the snapshot object
        DisplayError (hwnd, L"!!! Failed to gather information on system processes! \n", 1, 0);
        return(NULL);
    }

    do
    {
		

		
        if (0 == wcscmp(processName, pe32.szExeFile))
        {
            result = pe32.th32ProcessID;
			hProcessName = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
			if(hProcessName == nullptr )
				{
					DisplayError (hwnd, L"Cannot open this process!", 1, 0);
					CloseHandle(hProcessSnap);
					return(NULL);
				}
            break;
        }

    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);

    return result;
}

NTDLLptr DynamicLoader (bool progInit, wchar_t * fileObjVar)
{
	hdlNtCreateFile = LoadLibraryW(L"NtDll.dll");
	foundNTDLL = (NTDLLptr) GetProcAddress ((HMODULE) hdlNtCreateFile, createFnString);
	if (foundNTDLL)
		{
			if (progInit)
			{
			memset(&ioStatus, 0, sizeof(ioStatus));
			memset(&fileObject, 0, sizeof(fileObject));
			fileObject.Length = sizeof(fileObject);
			fileObject.Attributes = OBJ_CASE_INSENSITIVE;
			}
			else
			{
			//status error codes //also try GetModuleHandle("ntdll.dll"),
			//RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress((HMODULE)hdlNtCreateFile, NtStatusToDosErrorString);
			PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
			if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) return nullptr;
			//init Unicode string
			PFN_RtlInitUnicodeString RtlInitUnicodeString;
			if( !(RtlInitUnicodeString = (PFN_RtlInitUnicodeString) GetProcAddress( (HMODULE)hdlNtCreateFile, initUnicodeFnString )) ) return nullptr;
			RtlInitUnicodeString(&fn, fileObjVar);
			fileObject.ObjectName = &fn; //Ntdll.dll
			}
			return foundNTDLL;
		}
	else
		{
		FreeLibrary ((HMODULE) hdlNtCreateFile);
		return foundNTDLL;
		}

}

bool ProcessfileSystem(HWND hwnd, bool falseReadtrueWrite, bool appendMode)
{
	NTSTATUS ntStatus;
	DWORD Status;
	bool newFile = false;
	int  result;
	int  jLim;
	wint_t ch = 0, chOld = 0;
	FILE *stream = nullptr;


	wchar_t *fsName= (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	if (!ExpandEnvironmentStringsW (L"%SystemRoot%", fsName, pathLength)) ErrorExit (L"ExpandEnvironmentStringsW failed for some reason.",0);
	wcscat_s(fsName, pathLength, L"\\Temp\\CreateLargeFileSystem.txt");
	stream = _wfopen(fsName, L"r+b");
	//If the file already exists and is opened for reading or appending, the Byte Order Mark (BOM), if it present in the file, determines the encoding.
	if (!stream) //returns NULL Pointer
	{
		if (falseReadtrueWrite) 
		{
			if (DisplayError (hwnd, L"_wfopen returns NULL: possible first time run? Click yes to create new file, no to abort...", 0, 1))
			{
				newFile = true;
				stream = _wfopen(fsName, L"w+b");
				if (stream == nullptr) 
				{
					ErrorExit (L"Problems with opening input File.", 0);
					free (fsName);
					return false;
				}
			}
			else
			{
			free (fsName);
			return false; // can't read from empty file
			}
		}
		else
		{
		DisplayError (hwnd, L"Cannot Find Input file so cannot read!", 0, 0);
		free (fsName);
		return false;
		}
	}
	else //file exists
	{
	
	if (appendMode)
	{
	stream = _wfopen(fsName, L"a+b");
	}
	else //load FS when deleting
	{
	newFile = true; //BOM must be rewritten as it is wiped
	(falseReadtrueWrite)? stream = _wfopen(fsName, L"w+b"): stream = _wfopen(fsName, L"rb");

	}
	
	//When you switch from writing to reading, you must use an intervening call to either fflush or a file positioning function.
	if (!stream) //returns NULL Pointer
	{
	ErrorExit (L"Problems with input File: Cannot append.", 0);
	free (fsName);
	return false;
	}

	}


	if (falseReadtrueWrite) //write or append to file
	{

		_setmode(_fileno(stdout), _O_U16TEXT);
		//write BOM for byte-order endianness (storage of most/least significant bytes) and denote Unicode steream
	if (newFile)
	{
		if(fputwc(BOM, stream) == EOF)
		//if (fwrite("\xFEFF", 2, 2, stream) < 0)
		
		{
			ErrorExit (L"fwprintf: Problems with writing to input File.", 0);
			free (fsName);
			fclose (stream);
			return false;
		}
	}	
		
		//copy to whole string first: no sorting for write: create: append, Remove: write


		for (i = (createFail)? branchTotalCumOld: 0; (i <= ((createFail)? branchTotalCum - 1: branchTotal)); i++) //For deletion write the strings NOT deleted, creation the strings that succeeded
		{
			
			(appendMode)? jLim = trackFTA[i][0] + trackFTA[i][1] - 1: jLim = trackFTA[i][0] - 1;
			
			
			for (j = 0; (folderTreeArray[i][0][0] != L'\0') && (j <= jLim) ; j++)
			{
				k = 0;

				if (folderTreeArray[i][j][0] != L'\0')
					{
						do
						{	
    						ch = folderTreeArray[i][j][k];
							if (ch == L'\0')
							{	
								fputwc (separatorFTA, stream);
							}

							fputwc (ch, stream);
							
							k += 1;
						}
						while (ch != L'\0');

						if (j == jLim) fputwc (eolFTA, stream);
					}


 				}

		}

	}




	else //read from file
  	{
	
	ch = 1;
	(appendMode)? i = branchTotal + 1: i = 0;
	result = fseek(stream, 0L, SEEK_SET);  /* moves the pointer to the beginning of the file */
	//rewind(stream); //does the same?
	if (result)
	{
	ErrorExit (L"fseek: Could not rewind!", 0);
	free (fsName);
	fclose (stream);
	return false;
	}

	//Read BOM
	ch = fgetwc(stream);
		
	if(ch != BOM)
		
	{
		DisplayError(hwnd, L"fgetwc: input file does not have BOM!", 0, 0);
		free (fsName);
		fclose (stream);
		return false;
	}


		do
		{
		//we are reading so last null condition mandatory
		
			if (ch == eolFTA) ch = 1;	//ugly
			for (j = 0; (j < treeLevelLimit) && (ch != eolFTA); j++)
			{
			if (chOld == separatorFTA) chOld = 1;
			//populate folderTreeArray- using getline method (or "template function") might be more efficent
			for (k = 0; ((k  < (maxPathFolder - 1)) && (chOld != separatorFTA) && (ch != WEOF) ); k++) //screwy logic?

			{


			if ((ch = fgetwc(stream)) == separatorFTA) 
			{
				chOld = ch;
				ch = fgetwc(stream);
				
			}
			else
			{
				chOld = ch;
				if (ch == WEOF) goto WEOFFOUND;
			}
						
			if (ch == eolFTA)
			{
				break;
			}
			else
			{
				folderTreeArray[i][j][k] = (wchar_t)ch;
			}
			}

			trackFTA [i][0] = j; //track the nesting level for validation
			if (ch == eolFTA) break;
			if (j != 0) wcscat_s(pathsToSave[i], pathLength, &separatorFTA); //tacking them back on: what a waste doing it this way
			wcscat_s(pathsToSave[i], pathLength, folderTreeArray[i][j]);

 			}

			
			if (foundNTDLL && !appendMode) //only verify entire FS before delete
				{
					wcscpy_s(tempDest, pathLength, driveIDBaseWNT);
					wcscat_s(tempDest, pathLength, pathsToSave[i]);
					if (DynamicLoader (false, tempDest))
					{

						ntStatus = foundNTDLL (&hdlNTOut, FILE_LIST_DIRECTORY | SYNCHRONIZE, &fileObject, &ioStatus, nullptr, FILE_ATTRIBUTE_NORMAL, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_OPEN, FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_REPARSE_POINT, nullptr, 0);

						PFN_RtlNtStatusToDosError RtlNtStatusToDosError;
						if( !(RtlNtStatusToDosError = (PFN_RtlNtStatusToDosError) GetProcAddress( (HMODULE)hdlNtCreateFile, NtStatusToDosErrorString )) ) 
						{
							ErrorExit (L"RtlNtStatusToDosError: Problem!", 0);
							free (fsName);
							fclose (stream);
							return false;
						}
						Status = RtlNtStatusToDosError (ntStatus);

						switch ((DWORD)(ntStatus) >> 30)
							{
							case 0: //NT_SUCCESS
								{
								}
							break;
							case 1: //NT_INFORMATION
								{
								}
							break;
							case 2: //NT_WARNING 
								{
								}
							break;
							case 3://NT_ERROR
								{
									for (j = 0; (j  <= trackFTA [i][j]); j++)
									{
										folderTreeArray[i][j][0] = L'\0';
									}
									pathsToSave[i][0] = L'\0';
									ErrorExit (L"Cannot verify file in FS: Probably doesn't exist!!! ", 1);
									if (appendMode)
									{
										if (i > branchTotal + 1) i -=1;
									}
									else
									{
										if (i > 0) i -=1;
									}
									
								}
							break;
							}
						}
						else
						{
							errCode = 1;
							createFail = true;
							ErrorExit (L"DynamicLoader failed: Cannot create. ", 1);

						}

				}

		branchTotalSaveFile = i;
		i += 1;
		} while ((i < branchLimit) && (ch != WEOF));

	} 


WEOFFOUND:
if (foundNTDLL && !appendMode && !falseReadtrueWrite) //cleanup
{
	if (hdlNTOut) CloseHandle (hdlNTOut);
	memset(&ioStatus, 0, sizeof(ioStatus));
	memset(&fileObject, 0, sizeof(fileObject));
	FreeLibrary ((HMODULE)hdlNtCreateFile);
	ntStatus = NULL;
}
	// Close stream if it is not NULL 

	if (fclose (stream))
	{
	ErrorExit (L"Stream was not closed properly: exit & restart?", 0);
	free (fsName);
	return false;
	}
	else
	{
	free (fsName);
	return true;
	}




	}

void FSDeleteInit (HWND hwnd, HWND hList)
{
	bool cmdlineParmtooLong = false;
	tempDest = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	findPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t)); // only required for the old RecurseRemovePath
	currPathW = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
	pathToDeleteW = (wchar_t *)calloc(pathLength, sizeof(wchar_t));
	if (errCode != -100) errCode = -4;
	if (findPathW == nullptr || currPathW == nullptr || pathToDeleteW == nullptr)
	{
	/* We were not so display a message */
	DisplayError (hwnd, L"Bad: Could not allocate required memory. ", errCode, 0);
	goto RemoveKleenup;
	}
if (pCmdLineActive)
{
	wchar_t * rootDirPtr = wcschr (rootDir, '\\');
	if (rootDirPtr)
	{
		
		for (i = 0; i < (int)(rootDirPtr - rootDir); i++) rootDirPtr[i] = rootDir[i];
		rootDirPtr[i] = L'\0';
		wcscpy_s(rootDir, pathLength, rootDirPtr);
		pCmdLineActive = false;
	}
	else
	{
	DisplayError (hwnd, L"Oops! CmdLine not there!", errCode, 0);
	free(pathToDeleteW);
	goto RemoveKleenup;
	}

	currPathW[0] = L'\0';
	wcscpy_s(pathToDeleteW, pathLength, rootDir);
	wcscpy_s(currPathW, maxPathFolder, rootDir);
	if (sizeof(pathToDeleteW) > maxPathFolder) cmdlineParmtooLong = true;
	findPathW[0] = L'\0';
}

else
{
	memset(dacfoldersWtmp, L'\0', sizeof(dacfoldersWtmp)); //sizeof(dacfoldersW) ok
	//SHFileOperationW(_Inout_ LPSHFILEOPSTRUCT lpFileOp);

		for (i = 0; i < folderdirCS; i++)
		{
			mbstowcs(dacfoldersWtmp[i], dacfolders[i], maxPathFolder);
			//cumPath = dacfoldersW[i] + 4; // ////?// prefix away: 4 cuts out the //?/
			if (wcscmp(dacfoldersW[index-folderdirCW] + 4, dacfoldersWtmp[i]) == 0)
			{
			if (!DisplayError (hwnd, L"This Directory has an \"ANSII\" equivalent. Remove won't work if it contains files. Continue?", errCode, 1))
				{
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			}
		}

	currPathW[0] = L'\0';
	wcscat_s(currPathW, maxPathFolder, dacfoldersW[index-folderdirCW]);
	wchar_t * currPathWtmp;
	currPathWtmp = currPathW + 7;
	wcscpy_s(rootDir, pathLength, currPathWtmp);
	findPathW[0] = L'\0';
}


	if (foundNTDLL)
	{
		if (errCode > -100) 
		{
			if (!ProcessfileSystem(hwnd, false, false)) //Reads and verifies entire FS
			{
			if (!DisplayError (hwnd, L"No FS file! Cannot tell whether directory was created by this program. Try alternate delete?", 0, 1))
				{
					free (tempDest);
					free(pathToDeleteW);
					goto RemoveKleenup;
				}
			else
				{
					free (tempDest);
					free(pathToDeleteW);
					goto OldDelete;
				}
			}
		}
		else
		{
		errCode = -4;
		}
		//zero all trackFTA for anything that isn't rootDir
		//reorg DB so rootdir is first.
		free (tempDest);
		j = branchTotalSaveFile;
		branchTotal = j;
		int tmp;
		memset(reorgTmpWFS, L'\0', sizeof(reorgTmpWFS));
		memset(reorgTmpW, L'\0', sizeof(reorgTmpW));


		for (i = branchTotal; (i >= 0); i--) //place paths to delete at end of FS
		{

			if (!wcsncmp (rootDir, pathsToSave[i], wcslen (rootDir))) //0 if perfect match
			{
				if (i < j)
				{
				wcscpy_s(reorgTmpW, pathLength, pathsToSave[j]);
				wcscpy_s(pathsToSave[j], pathLength, pathsToSave[i]);
				wcscpy_s(pathsToSave[i], pathLength, reorgTmpW);

				for (k = 0; (k < trackFTA [j][0]); k++)
				{
				wcscpy_s(reorgTmpWFS[k], maxPathFolder, folderTreeArray[j][k]);
				}
				for (k = 0; (k < trackFTA [i][0]); k++)
				{
				wcscpy_s(folderTreeArray[j][k], maxPathFolder, folderTreeArray[i][k]);
				}
				folderTreeArray[j][trackFTA [i][0]][0] = L'\0';

				for (k = 0; (k < trackFTA [j][0]); k++)
				{
				wcscpy_s(folderTreeArray[i][k], maxPathFolder, reorgTmpWFS[k]);
				}
				folderTreeArray[i][trackFTA [j][0]][0] = L'\0';

				tmp = trackFTA [j][0];
				trackFTA [j][0] = trackFTA [i][0];
				trackFTA [i][0] = tmp;
				}
				j -= 1;
			}

		}

		if (branchTotal)
		{
			if (branchTotal == j)
			{
				if (DisplayError (hwnd, L"The selected folder is not found in the FS. Try alternate delete?", 0, 1))
				{
					free(pathToDeleteW);
					goto OldDelete;
				}
				else
				{
					free(pathToDeleteW);
					goto RemoveKleenup;

				}
			}
			branchTotalCum = j + 1;
		}
		else
		{
			branchTotalCum = 0;
		}


			do
			{
			} while (FSDelete (hwnd));
			//Write remaining FS
			free(pathToDeleteW);
			if (!errCode) //errCode is still -4 if no match!
			{
			if (!pCmdLineActive) ((ProcessfileSystem(hwnd, true, false))? errCode = 1: errCode = 0);
			//this can bug out if the user edits or deletes the FS in the intervening milliseconds when called from PopulateList.				
			goto RemoveKleenup;
			}

	}
	else
	{
		if (!DisplayError (hwnd, L"NTDLL not found on this machine. Continue with alternate delete?", 0, 1))
			{
				free(pathToDeleteW);
				goto RemoveKleenup;
			}
	}


OldDelete:

	
if (cmdlineParmtooLong)
	{
		errCode = 0;
		DisplayError (hwnd, L"Oops, command line too long! Delete won't work. Quit and try again should work.", 0, 0);
		goto RemoveKleenup;
	}
	wcscat_s(currPathW, maxPathFolder, &separatorFTA);
	wcscpy_s(folderTreeArray[0][0], maxPathFolder, currPathW);

	treeLevel = 0;
	trackFTA [0][0] = 0; //Initial conditions before search on path
	trackFTA [0][1] = 1;

	for (i = 1; i < branchLimit; i++)
	{
	trackFTA [i][0] = 0; //Initial conditons before search on path
	trackFTA [i][1] = 0;
	}
						
	if (!SetCurrentDirectoryW (driveIDBaseW))
	{
	ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
	goto RemoveKleenup;
	}
	if (RecurseRemovePath(trackFTA, folderTreeArray))
		{
			errCode = 0;
			DisplayError (hwnd, L"Remove failed.", 0, 0);
		}
	else
		{
			errCode = 1;
		}


					
	RemoveKleenup:
	if (findPathW) free (findPathW);
	if (currPathW) free (currPathW);
	rootDir[0] = L'\0';
	_CrtDumpMemoryLeaks();
		if (pCmdLineActive)
		{
			//ReleaseMutex (hMutex); //problematic??
			EndDialog(hwnd, 1);
		}

		else 
		{
			if (errCode != 0) //"succeeded"
			{
			errCode = 0; //flag okay now
			listTotal = SendMessageW(hList, LB_GETCOUNT, 0, 0);
			sendMessageErr = SendDlgItemMessageW(hwnd, IDC_LIST, LB_RESETCONTENT, 0, 0);
			PopulateList(hwnd);
			}
			else 
			{
			errCode = -100;
			}
		}
return;
}

bool FSDelete (HWND hwnd)
{	

if (branchTotal == branchTotalCum - 1) //branchTotal is decremented here, not branchTotalCum
	{
		branchTotal = branchTotalCum; // required for FS Write
		return false;
	}




	for (i = branchTotalCum; (i <= branchTotal); i++)
	{
		trackFTA [i][1] = trackFTA [i][0];
	}


	for (i = branchTotalCum + 1; (i <= branchTotal); i++)
		{ 
		int tmp = trackFTA [i][1]; //insertion sort
			for (j = i; ((j > branchTotalCum) && (tmp < trackFTA [j-1][1])); j--)
			{
				trackFTA [j][1] = trackFTA [j-1][1];
			
			}
		trackFTA [j][1] = tmp;
	}



	for (i = branchTotal; (i >= branchTotalCum); i--)
	{
		//delete the bottom folder of pathsToSave[i] whose first strings correspond to rootDir in the order of trackFTA [i][1] 
			{
				for (j = branchTotal; (j >= branchTotalCum); j--)
				{
					if (trackFTA [i][1] == trackFTA [j][0]) return (fsDelsub (i, j, hwnd));
				}
			}
	}

return true; //shouldn't get here though
}
bool fsDelsub (int i, int j, HWND hwnd)
{

	for (k = (trackFTA [i][1]); (k >= ((i > branchTotalCum)? trackFTA [i - 1][1]: 0)); k--)
	{
	//do not iterate below trackFTA [i + 1][1]

	pathToDeleteW[0] = L'\0'; // driveIDBaseWNT L"C:\\"

	wcscpy_s(pathToDeleteW, pathLength, driveIDBaseWNT);
	wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);

	if (RemoveDirectoryW (pathToDeleteW))
			{
				//clear everything
				pathsToSave[j][0] = L'\0';
				//we have only removed the last dir from pathsToSave so remove last dir from folderTreeArray
							

				folderTreeArray[j][k-1][0] = L'\0';

				trackFTA [i][1] -=1;
				trackFTA [j][0] -=1;


				if (trackFTA [i][1] != 0)
				{
				//rebuild pathsToSave
					for (int l = 0; (l < k - 1); l++) //extra loop adds the terminator
					{
						if (l != 0) wcscat_s(pathsToSave[j], pathLength, &separatorFTA);
						wcscat_s(pathsToSave[j], pathLength, folderTreeArray[j][l]);
					}

				}
				else
				{
				goto FSReorg;
				}
			
			}
		else
			{
				if (((int)GetLastError() == 32) ) //"used by another process" error
				{
					memset(rootDir, L'\0', sizeof(rootDir));
					pCmdLineActive = true;
					wcscpy_s(pathToDeleteW, pathLength, L" "); //http://forums.codeguru.com/showthread.php?213443-How-to-pass-command-line-arguments-when-using-CreateProcess
					wcscat_s(pathToDeleteW, pathLength, pathsToSave[j]);

					((ProcessfileSystem(hwnd, true, false))? errCode = 0: errCode = 1);
					Kleenup (hwnd, weareatBoot);
					return false;

				}
				else
				{
					if (((int)GetLastError() == 2) || ((int)GetLastError() == 3)) //cannot find file or path specified
						{
						//The entry in pathsToSave must have a duplicate elsewhere: nuke the current one:
						pathsToSave[j][0] = L'\0';
								
						folderTreeArray[j][0][0] = L'\0';

						trackFTA [i][1] = 0;
						trackFTA [j][0] = 0;
						goto FSReorg;
						}
					else
						{
						ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
						errCode = 0;
						return false;
					}

				}

			}

	}
	//k loop

return true;

FSReorg:
errCode = 0;

if (j != branchTotal)
	{
//Move everything down to fill slot
	for (k = j + 1; (k <= branchTotal); k++)
	{
		for (int l = 0; (l < trackFTA [k][0]); l++)
			{
			wcscpy_s(folderTreeArray[k-1][l], maxPathFolder, folderTreeArray[k][l]); 
			}	

		trackFTA [k-1][0] = trackFTA [k][0];
		wcscpy_s(pathsToSave[k-1], pathLength, pathsToSave[k]); 
	}
	}	
pathsToSave [branchTotal][0] = L'\0';
branchTotal -=1;
return true;
}
int RecurseRemovePath(int trackFTA[branchLimit][2], wchar_t folderTreeArray[branchLimit + 1][treeLevelLimit + 1][maxPathFolder])

	 //first element of trackFTA is LAST_VISIT, second is number of folders found
{
	

	if (trackFTA [treeLevel][1] > 0) //Have we done a search on this level yet? if yes then here
	{

			//move along the branch

			//wcscpy_s(currPathW, maxPathFolder, folderTreeArray[treeLevel][trackFTA[treeLevel][0]-1]);

			//FINISH coding this
			if (trackFTA [treeLevel][0] == trackFTA [treeLevel][1]) //end of the branch?
			{
			//Must go down but we have already found the files of directory below
			trackFTA [treeLevel][1] = 0;  //important
			treeLevel -=1;
			wcscpy_s(currPathW, maxPathFolder, folderTreeArray[trackFTA [treeLevel][0]-1][treeLevel]);


					if (treeLevel == 0) //Last folder to do!! 
					{
						if (!SetCurrentDirectoryW (driveIDBaseW)) //objects to L".."
						{
						ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						return 1;
						}
						wchar_t * currPathWtmp;
						currPathWtmp = currPathW + 4;

						if (RemoveDirectoryW (currPathWtmp))
						{
							return 0;
						}
						else
						{
							ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

							return 1; //Need more than this
						}

					}

					else
					{
						if (!SetCurrentDirectoryW (L".."))
						{
							ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
							return 1;
						}

						if (RemoveDirectoryW (currPathW))
						{

						if (RecurseRemovePath(trackFTA, folderTreeArray))
							{
							return 1;
							}
							else
							{
							return 0;
							}
						}
						else
							{
								ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
								return 1; //Need more than this
							}


					}




			}

			else
			{
				//folderTreeArray[treeLevel][j+1]
				if (trackFTA[treeLevel][0] <= 999)
				{
				trackFTA[treeLevel][0] +=1;

				// set inits for this branch
				findPathW[0] = L'\0';
				wcscat_s(findPathW, maxPathFolder, folderTreeArray[trackFTA[treeLevel][0]-1][treeLevel]);
				//
				if (!SetCurrentDirectoryW (findPathW))
				{
					ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
					return 1;
				}

				treeLevel +=1; // up next tree
					if (RecurseRemovePath(trackFTA, folderTreeArray))
						{
						return 1;
						}
						else
						{
						return 0;
						}

				}
				else
				{
				trackFTA[treeLevel][0] = 0;
				trackFTA[treeLevel][1] = 0;
				treeLevel -=1;
				ErrorExit (L"Too many folders in the tree: If folder was created by this program, a warning should have been issued on folder creation.", 0);
				return 1; 
				}
			}

	}

	else //search yet to be done on branch
	{



	//Do find folders in new branch, findPathW already set
			
				
		memset(&dw, 0, sizeof(WIN32_FIND_DATAW));
		//Find first file
		//Get fulqualpath
		if (!GetCurrentDirectoryW (maxPathFolder, findPathW))
			{
				ErrorExit (L"GetCurrentDirectoryW: Zero", 0);
				return 1;
			}
		wcscat_s(findPathW, maxPathFolder, L"\\*");
		ds = FindFirstFileW(findPathW, &dw);
			if (ds == INVALID_HANDLE_VALUE) //redundant as first 2 pickups are "." and ".."
			{
				// No Folders so this must be top level
				FindClose(ds);
				ErrorExit (L"FindFirstFileW: Should never get here. No can do!", 0);
				return 1; //Need more than this
							
			}


						
			BOOL findhandle = TRUE;
			j = 0;
							
			while (ds != INVALID_HANDLE_VALUE && findhandle)
			{

			if ((dw.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && !(dw.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM || dw.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ||
				dw.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE || dw.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN || !wcscmp(dw.cFileName, L".") || !wcscmp(dw.cFileName, L"..")))
										
				//"." is just an alias for "this" directory
				//".." is just an alias for the parent directory.

			{

				currPathW[0] = L'\0';
				wcscat_s(currPathW, maxPathFolder, dw.cFileName);
				wcscat_s(currPathW, maxPathFolder, &separatorFTA);


				wcscpy_s(folderTreeArray[j][treeLevel], maxPathFolder, (wchar_t *)currPathW);
				j +=1;

			}
				findhandle = FindNextFileW(ds, &dw);
			}

			if (!FindClose(ds)) ErrorExit (L"FindClose: Non zero", 0);
			//wcscpy_s(currPathW, maxPathFolder, folderTreeArray[treeLevel][j-1]);
			trackFTA [treeLevel][0] = 0; //check reset counter if necessary here

			if (j == 0)
				{
				// No Folders so this must be top level

					if (treeLevel == 1) //Last folder to do!! 
					{
						if (!SetCurrentDirectoryW (driveIDBaseW)) //objects to L".."
						{
						ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						return 1;
						}
					wchar_t * currPathWtmp = (wchar_t *)calloc(maxPathFolder, sizeof(wchar_t));
					currPathWtmp = currPathW + 4;
					//GetCurrentDirectoryW(maxPathFolder, findPathW);
						if (RemoveDirectoryW (currPathWtmp))
						{
							return 0;
						}
						else
						{
							ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);

							return 1; //Need more than this
						}

					}

					else
					{

						if (!SetCurrentDirectoryW (L".."))
						{
						ErrorExit (L"SetCurrentDirectoryW: Non zero", 0);
						return 1;
						}
					}

					//GetCurrentDirectoryW(maxPathFolder, findPathW);
					if (RemoveDirectoryW (folderTreeArray[trackFTA[treeLevel-1][0]-1][treeLevel-1]))
					{
						trackFTA [treeLevel][1] = 0;  //important
						treeLevel -=1;
						if (RecurseRemovePath(trackFTA, folderTreeArray))
							{
							return 1;
							}
							else
							{
							return 0;
							}
					}
					else
					{
						ErrorExit (L"RemoveDirectoryW: Cannot remove Folder. It may contain files.", 0);
						return 1; //Need more than this
					}

				}

		else //Do an iteration on this new branch
				{
					//if (!GetCurrentDirectoryW(maxPathFolder, findPathW)) ErrorExit("SetCurrentDirectoryW: Non zero", 0);
					trackFTA [treeLevel][1] = j;

					if (RecurseRemovePath(trackFTA, folderTreeArray))
					{
					return 1;
					}
					else
					{
					return 0;
					}
				}
	} //trackFTA[treeLevel][0] = 0
 
}
