#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <dbt.h>
#include <ntddkbd.h>

const GUID DECLSPEC_SELECTANY GUID_DEVINTERFACE_KEYBOARD = { 0x884b96c3, 0x56ef, 0x11d1, { 0xbc, 0x8c, 0x00, 0xa0, 0xc9, 0x14, 0x05, 0xdd } }; 
const GUID DECLSPEC_SELECTANY GUID_DEVINTERFACE_MOUSE    = { 0x378de44c, 0x56ef, 0x11d1, { 0xbc, 0x8c, 0x00, 0xa0, 0xc9, 0x14, 0x05, 0xdd } }; 

void hid_arrived()
{
	LockWorkStation();
}

void keyboard_device_arrived()
{
	hid_arrived();
}

void mouse_device_arrived()
{
	hid_arrived();
}

int same_guid(GUID a, GUID b)
{
	return ((a.Data1 == b.Data1) && (a.Data2 == b.Data2) && (a.Data3 == b.Data3) && ((memcmp(a.Data4, b.Data4, 8)) == 0));
}

HDEVNOTIFY *MyRegisterDeviceInterfaceToHwnd(GUID InterfaceClassGuid, HWND hWnd)
{
    DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

    ZeroMemory( &NotificationFilter, sizeof(NotificationFilter) );
    NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
    NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
    NotificationFilter.dbcc_classguid = InterfaceClassGuid;

    return RegisterDeviceNotification( hWnd,  &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE );
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam) {
	static HDEVNOTIFY hDeviceNotify;
	switch(Message) {

        case WM_CREATE:
            hDeviceNotify = MyRegisterDeviceInterfaceToHwnd(GUID_DEVINTERFACE_KEYBOARD, hwnd);
			if (hDeviceNotify == NULL) {
				MessageBox(hwnd, "Error registering device notifier", "Error", MB_ICONERROR | MB_OK);
                ExitProcess(1);
            }
            break;
            
        case WM_CLOSE:
            UnregisterDeviceNotification(hDeviceNotify);
            DestroyWindow(hwnd);
            break;

		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
		
        case WM_DEVICECHANGE:
        {
         	PDEV_BROADCAST_HDR hdr = (PDEV_BROADCAST_HDR) lParam;

            switch (wParam)
           {
                case DBT_DEVICEARRIVAL:
                	if (hdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
                        PDEV_BROADCAST_DEVICEINTERFACE b = (PDEV_BROADCAST_DEVICEINTERFACE) lParam;
                	    if (same_guid(b->dbcc_classguid, GUID_DEVINTERFACE_KEYBOARD)) {
                	    	keyboard_device_arrived();
						}
                	    if (same_guid(b->dbcc_classguid, GUID_DEVINTERFACE_MOUSE)) {
                	    	mouse_device_arrived();
						}
					}
                    break;
           }
        }
	}

	return DefWindowProc(hwnd, Message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG msg;

	memset(&wc,0,sizeof(wc));
	wc.cbSize		 = sizeof(WNDCLASSEX);
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInstance;
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszClassName = "Keyboard-Auto-Lock";
	wc.hIcon		 = LoadIcon(NULL, IDI_APPLICATION);
	wc.hIconSm		 = LoadIcon(NULL, IDI_APPLICATION);

	if(!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	hwnd = CreateWindowEx(WS_EX_CLIENTEDGE,"Keyboard-Auto-Lock","Keyboard-Auto-Lock",/*WS_VISIBLE|*/WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1, 1, NULL,NULL,hInstance,NULL);
	if(hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);
		return 0;
	}

	ShowWindow(hwnd, SW_HIDE);

	while(GetMessage(&msg, NULL, 0, 0) > 0) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}

